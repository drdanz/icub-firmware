/*
 * firmware/controller application.
 *
 */

#include "dsp56f807.h"
#include "options.h"
#include "versions.h"
#include "asc.h"
#include "ti1.h"
#include "pid.h"
#include "calibration.h"
#include "trajectory.h"
#include "can_interface.h"
#include "serial_interface.h"
#include "ad.h"
#include "can1.h" 
#include "leds_interface.h"
#include "currents_interface.h"
#include "flash_interface.h"
#include "pwm_interface.h"
#include "faults_interface.h"
#include "temp_sens_interface.h"
#include "brushed_comm.h"
#include "abs_ssi_interface.h"
#include "encoders_interface.h"
#include "pwm_decoupling.h"
#include "position_decoupling.h"
#include "check_range.h"
#include "pwm_a.h"
#include "pwm_b.h"
#include "control_enable.h"

byte	_board_ID = 15;	
char    _additional_info [32];
byte    _build_number = BUILD_4DC;
byte    _my_can_protocol_major = CAN_PROTOCOL_MAJOR;
byte    _my_can_protocol_minor = CAN_PROTOCOL_MINOR;
bool    _can_protocol_ack = false;

UInt8    mainLoopOVF=0;
int     _countBoardStatus[2] ={0,0};
Int16   _flash_version=0; 
UInt8   BUS_OFF=false;

#ifdef TEMPERATURE_SENSOR
	//temperature sensor variables
	Int16    TempSens[]= {0,0};
	UInt8    overtemp[]= {0,0};
	UInt8    errortemp[]={0,0};

#endif


//**********************
// externs
//**********************
extern bool           _pad_enabled[2];

//********************
// Local prototypes 
//********************

#if VERSION == 0x0161
Int16 _version = 0x0161;
#elif VERSION == 0x0162
Int16 _version = 0x0162;
#elif VERSION == 0x0163
Int16 _version = 0x0163;
#endif
#ifndef VERSION
#	error "No valid version specified"
#endif 


#ifdef ENCODER_SHIFT_11BITS
	#define Filter_Bit(x) ((x>>1)<<1)
#else
	#define Filter_Bit(x) (x)
#endif

/***********************************************************************
	This is the main controller loop.
	sequences of operations:
		- reads from CAN bus or serial port 1.
		- reads encoders (or ADC).
		- computes the control value (a PID in this version).
		- checks limits and other errors.
		- sets PWM
		- does extra functions (e.g. communicate with neighboring cards).
 ***********************************************************************/
void main(void)
{
	Int32 PWMoutput [JN];
	byte i=0;
	UInt16 *value=0;
	Int32 temp_swap = 0;
	Int32 PID_R= 2;
	Int32 kpp=1;
	Int16 test =0;
	byte first_step=0;
	Int32 t1val=0;	
 	Int16 _safeband[JN];	//it is a value for reducing the JOINT limit of 2*_safeband [tick encoder]
	//temperature sensor variables
	Int16  TempSens[]=	 {0,0};
	byte   TempSensCount1 = 0;
	UInt32 TempSensCount2 = 0;
	
	bool  status_lin = 0;
	bool  status_inc = 0;
	bool  status_dec = 0;
	bool  status_ocf = 0;
	bool  status_cof = 0;
	
	
	/* gets the address of flash memory from the linker */
	_flash_addr = get_flash_addr();
		
	/* enable interrupts */
	setReg(SYS_CNTL, 0);
	
	// IPL channels from 0 to 6 enabled
	// external interrupts IRQA and IRQB disabled
	setRegBits(IPR, 0xFE00); 

	// enable FAULT
	__ENIGROUP (61, 3);
	__ENIGROUP (60, 3);
		
	// enable SCI
	__ENIGROUP (52, 4);
	__ENIGROUP (53, 4);
	__ENIGROUP (50, 4);
	__ENIGROUP (51, 4);
	
    // enable data flash
	__ENIGROUP (13, 4);
	
	// enable CAN	
	__ENIGROUP (14, 6);
	__ENIGROUP (15, 6);
	__ENIGROUP (16, 6);
	__ENIGROUP (17, 6);
	
	// enable ADCA/ADCB 
	__ENIGROUP (55, 4);
	__ENIGROUP (54, 4);
	
	// enable timers
	// TIMER_A
	__ENIGROUP (45, 7); //
	__ENIGROUP (44, 7); //
	__ENIGROUP (43, 7); //
	__ENIGROUP (42, 7); // TI1 1ms delay main loop
	// TIMER_B
	__ENIGROUP (41, 7); //
	__ENIGROUP (40, 7); //
	__ENIGROUP (39, 7); //
	__ENIGROUP (38, 7);
	// TIMER_C
	__ENIGROUP (37, 1); //
	__ENIGROUP (36, 1); //
	__ENIGROUP (35, 1); //AD triggered with the PWM 
	__ENIGROUP (34, 1);
	// TIMER_D
	__ENIGROUP (33, 7); // 1ms timer for brushed PWM ramp 
	__ENIGROUP (32, 1);
	__ENIGROUP (31, 1);
	__ENIGROUP (30, 1);

//variable init	
	mainLoopOVF=0;
	BUS_OFF=false;
	_count=0;

	__EI(); 
	
	flash_interface_init  (JN);			
	readFromFlash (_flash_addr);  
	
	__DI();
   
	init_leds  			  ();			
//	serial_interface_init (JN);
	can_interface_init    (JN);
	init_strain          ();

	Init_Brushed_Comm    ();	


//
    init_faults           (true,true,true);	 
    init_position_encoder ();
    TI1_init 			  ();
	__EI();
	init_position_abs_ssi  ();

//	print_version ();
	
#warning "no calibration check" 	
#if VERSION==0x0161		
	/* initialization */
	for (i=0; i<JN; i++) _calibrated[i] = true;
#endif

#if VERSION==0x0162	|| VERSION==0x0163
	/* initialization */
	for (i=0; i<JN; i++) _calibrated[i] = false;
#endif	
	/* reset trajectory generation */
	for (i=0; i<JN; i++) abort_trajectory (i, 0);
	
#if VERSION==0x0162	|| VERSION==0x0163
   	for (i=0; i<JN; i++)	_position[i]=(Int32) Filter_Bit(get_position_abs_ssi(i));
   	for (i=0; i<JN; i++)    _max_real_position[i]=Filter_Bit(4095);
   	for (i=0; i<JN; i++)	_position_enc[i]=0;
#endif

#    	
	/* initialize speed and acceleration to zero (useful later on) */
	for (i=0; i<JN; i++) _position_old[i] = 0;
	for (i=0; i<JN; i++) _position_enc_old[i] = 0;
	for (i=0; i<JN; i++) _speed_old[i] = 0;
	for (i=0; i<JN; i++) _accel[i] = 0;
	for (i=0; i<JN; i++) _safeband[i] =-5; //5 ticks => 1 grado di AEA.
	


	/* main control loop */
	for(_counter = 0;; _counter ++) 
	{
		if (_counter >= CAN_SYNCHRO_STEPS) _counter = 0;
		led3_on;
		while (_wait) ;
		
		test=test+1;
		_count=0;
		led3_off;

    	// BUS_OFF check
		if (getCanBusOffstatus() )
		{
			for (i=0; i<JN; i++) put_motor_in_fault(i);
			led1_off
		}
		else
			led1_on
		
		can_interface();
		
		for (i=0; i<JN; i++)
		if (_pad_enabled[i]==false && _control_mode[i]!=MODE_HW_FAULT) _control_mode[i]=MODE_IDLE;
					
	
	    //Position calculation
	    // This is used to have a shift of the zero-cross out of the 
	    // joint workspace
	    //
	    // max_real_position is the limit of the joint starting from 
	    // 4095 and going to decrease this number without zero-cross
	    // untill the joint limit is reached
	    	    
	    for (i=0; i<JN; i++) 
		{		
			_position_old[i]=_position[i];
#if VERSION==0x0162	|| VERSION==0x0163	
		    _position_enc_old[i]=_position_enc[i];	
			_position[i]=Filter_Bit (get_position_abs_ssi(i));
			_motor_position[i]=_position_enc[i]=get_position_encoder(i);
			//can_printf("%f %f %f %f",_position_enc[0],_position_enc[1],_position[0],_position[1]);
			
#elif VERSION==0x0161
		_motor_position[i]=_position[i]=get_position_encoder(i);
#endif					
		}
	
		// decoupling the position	 	
		decouple_positions();

	//#define TEST_I2C	
	#ifdef TEST_I2C
		if (_filt_current[0] < MAX_I2T_CURRENT)
		can_printf("%f %f", _filt_current[0], MAX_I2T_CURRENT);
		else
		can_printf("%f %f ***", _filt_current[0], MAX_I2T_CURRENT);
	#endif	
		
		
		/* this can be useful to estimate speed later on */
		if (_counter == 0)
		{		
			/* this can be useful to estimate speed later on */
	#if VERSION==0x0161		
		
			for (i=0; i<JN; i++) _speed[i] = (Int16)(_position[i] - _position_old[i]);

	#elif VERSION==0x0162 || VERSION==0x0163
            _motor_speed[0] = (Int16)  (_position_enc[0] - _position_enc_old[0]);
            _motor_speed[1] = (Int16)  (_position_enc[1] - _position_enc_old[1]);
            _speed[0]       = (Int32)  (_motor_speed[0]  - _motor_speed[1]);
            _speed[1]       = (Int32)  (_motor_speed[0]  + _motor_speed[1]);
  
	#endif 
			/* this can be useful to estimate acceleration later on */
			for (i=0; i<JN; i++) _accel[i] = (_speed[i] - _speed_old[i]);
			
			/* memorize old position and velocity (for next cycle) */
			for (i=0; i<JN; i++) _position_old[i] = _position[i];
			for (i=0; i<JN; i++) _speed_old[i] = _speed[i];
		}
			
		/* in position? */
		for (i=0; i<JN; i++) _in_position[i] = check_in_position(i); 
				
		/* in reference configuration for calibration? */
		for (i=0; i<JN; i++) check_in_position_calib(i); 
	
		//FT sensor watchdog update 
		for (i=0; i<STRAIN_MAX; i++) 
			if (_strain_wtd[i]>0) _strain_wtd[i]--;
		
		
	    for (i=0; i<JN; i++) 
		{		
		   if (get_error_abs_ssi(i)==ERR_ABS_SSI)
		   {
				put_motor_in_fault(i);	
				PWM_outputPadDisable(0);
				PWM_outputPadDisable(1);
				#ifdef DEBUG_CAN_MSG
		    	can_printf("ABS error %d",i);	
				#endif
		   }				
		}  
						
		/* computes controls */
		for (i=0; i<JN; i++) PWMoutput[i] = compute_pwm(i);
		
	     
		/* decouple PWM */	
		decouple_dutycycle(PWMoutput); 		
		
//******************************************* SATURATES CONTROLS ***************************/                
		/* saturates controls if necessary */
		for (i=0; i<JN; i++)
		{
			if (_control_mode[i] == MODE_TORQUE ||
				_control_mode[i] == MODE_IMPEDANCE_POS ||
				_control_mode[i] == MODE_IMPEDANCE_VEL)
			{
				//add the bemf compensation term
				//PWMoutput[i]+=compensate_bemf(i, _comm_speed[i]); //use the motor speed
				PWMoutput[i]+=compensate_bemf(i, _speed[i]); //use the joint speed
				
				//add the coulomb friction compensation term
				if (_kstp_torque[i] != 0 ||
				    _kstn_torque[i] != 0)
				//PWMoutput[i]+=compensate_friction(i, _comm_speed[i]); //use the motor speed
				PWMoutput[i]+=compensate_friction(i, _speed[i]); //use the joint speed
				
				// Protection for joints out of the admissible range during force control
				check_range_torque(i, _safeband[i], PWMoutput);
				// PWM saturation
				ENFORCE_LIMITS	(i,PWMoutput[i], _pid_limit_torque[i] );
			}
			else
			{
				ENFORCE_LIMITS	(i,PWMoutput[i], _pid_limit[i] );
			}			
			if      (_pid[i] < -MAX_DUTY) _pid[i]=-MAX_DUTY;
			else if (_pid[i] > MAX_DUTY)  _pid[i]= MAX_DUTY;
		}
				
		/* generate PWM */		
		for (i=0; i<JN; i++)
		{
//#define	DEBUG_FAULT_ON_LIMIT 
#ifdef DEBUG_FAULT_ON_LIMIT
			if (_calibrated[0]==true && _calibrated[1] == true)
				if (_position[0]>_max_position[0] || _position[0]<_min_position[0] ||
			        _position[1]>_max_position[1] || _position[1]<_min_position[1])
			{
				can_printf("OUT!!");
				put_motor_in_fault(0);
				put_motor_in_fault(1);
			}
#endif
			if (!mode_is_idle(i)) 
			{
				PWM_generate(i,_pid[i]);
		//		setReg (TMRD0_CNTR, 39998);              				
			}
			
		}

		
		
/*****************************************************************
/*
/*      Check Current is done in the Ti1 Interrupt routine  
/*
/*****************************************************************/


#ifdef DEBUG_TRAJECTORY
		if (_verbose && _counter == 0)
		{
			for (i=0; i<JN; i++)
			{
				AS1_printDWordAsCharsDec (_position[i]);
				AS1_printStringEx (" ");	
			}
			AS1_printStringEx ("\r\n");
		}
#endif

		/* do extra functions, communicate, etc. */
		can_send_broadcast();
			//	Check for the MAIN LOOP duration 

//	Check for the MAIN LOOP duration
 
			
		t1val= (UInt16) TI1_getCounter(); 	
		if (	_count>0)
		{
			
		#ifdef DEBUG_CAN_MSG
    		can_printf("MainLoop OVF");
		#endif		
		mainLoopOVF=1;
		_count=0;
		}
		else
		mainLoopOVF=0;
		/* tells that the control cycle is completed */
		_wait = true;	
		
	} /* end for(;;) */
}
	










