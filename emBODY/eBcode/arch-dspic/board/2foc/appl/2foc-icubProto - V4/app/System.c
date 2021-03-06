//
// system related functions
// system clock, IO ports, system LEDs
//

#include <p33FJ128MC802.h>

#include <dsp.h>
#include <libpic30.h>  //__delay32
#include <timer.h>
//#define USE_AND_OR
//#include <i2c.h>
#include "i2cTsens.h"

#include "UserParms.h"
#include "System.h"
#include "UserTypes.h"
#include "I2T.h"
//#include "controller.h"
#include "qep.h"
#include "faults.h"
#include "ecan.h"
#include "encoder.h"
//#include "rotor_alignment.h"
#include "crc16.h"

//some local constants to make code cleaner..
unsigned char * eeprom_header_start = (unsigned char *) &ApplicationData;
unsigned short eeprom_header_len = sizeof(tEepromHeader);

unsigned char * eeprom_payload_start =
                         (unsigned char *) &ApplicationData + sizeof(tEepromHeader);
unsigned short eeprom_payload_len = sizeof(ApplicationData) -  sizeof(tEepromHeader);


void ActuateMuxLed()
// Actuate Multiplexed led ports
{
  static short Slot=0;

  if (0 == Slot)
  {
    // slot for green led
    if (1 == LED_status.Green )
      TurnOnLedGreen()
    else
      TurnOffLed();
    Slot=1;
  }
  else
  {
    // slot for red led
    if (1 == LED_status.Red)
      TurnOnLedRed()
    else
      TurnOffLed();
    Slot=0;
  }
}

void BlinkLed()
// blink leds according to the desired blinking rate
{
  static long RedCounter=0, GreenCounter=0;

  RedCounter++;
  GreenCounter++;

  switch(LED_status.GreenBlinkRate)
  {

    case BLINKRATE_STILL:
      LED_status.Green=1;
    break;

    case BLINKRATE_OFF:
      LED_status.Green=0;
    break;

    default:
      // green led blinker
      if(GreenCounter >= LED_status.GreenBlinkRate)
      {
        GreenCounter=0;

        // toggle green LED
        if(1==LED_status.Green)
        {
          LED_status.Green=0;
        }
        else
        {
          LED_status.Green=1;
        }
      }
    break;
  }

  switch(LED_status.RedBlinkRate)
  {
    case BLINKRATE_STILL:
      LED_status.Red=1;
    break;

    case BLINKRATE_OFF:
      LED_status.Red=0;
    break;

    default:
      // red blinker
      if(RedCounter >= LED_status.RedBlinkRate)
      {
        RedCounter = 0;

        // toggle red LED
        if(1==LED_status.Red)
        {
          LED_status.Red=0;
        }
        else
        {
          LED_status.Red=1;
        }
      }
    break;
  }
}

extern bool updateOdometry();

void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void)
// Timer 3 IRQ service routine
// used to run I2T (in order to unwind it) when 2FOC loop is stopped
// and to perform encoder turn count
{
    I2Tdata.IQMeasured = 0;
    I2Tdata.IDMeasured = 0;

    //I2T(&I2Tdata);

    updateOdometry();

    updateGulp();

    IFS0bits.T3IF = 0; // clear flag
}

void __attribute__((interrupt, no_auto_psv)) _MI2C1Interrupt(void)
{
        //IFS1bits.MI2C1IF = 0;		//Clear the DMA0 Interrupt Flag;
}

//#define WAITFOR(flag,errcode,jump) for (wdog=12800; wdog && (flag); --wdog); if (!wdog) { gTemperature=errcode; goto jump; }

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
//
// TIMER 1 IRQ Service Routine
// used for  low frequency operation (LED toggling reference....)
//
{
  extern int gTemperature;
  extern unsigned int i2cERRORS;

  // polls for recovered can error in order to reset status flags
  ECANCheckState();
  // blink leds according to the desired blinking rate
  BlinkLed();
  // light the leds
  ActuateMuxLed();

  static int cycle = 0;
  if (++cycle>=20)
  {
        cycle = 0;

        gTemperature = readI2CTsens();

        if (gTemperature < 0) ++i2cERRORS;
  }

  IFS0bits.T1IF = 0; // clear flag
}

void Timer1Config()
// Setup timer 1 registers for low frequency operation (LED, .... )
{
  // reset timer
  WriteTimer1(0);
  // 4,82 msec (207.4Hz)
  OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_STOP & T1_PS_1_64 &
    T1_SYNC_EXT_OFF & T1_SOURCE_INT, 3000);
}

void Timer3Enable()
{
  WriteTimer3(0);
  IFS0bits.T3IF = 0; // clear flag
  EnableIntT3;
}

void Timer3Disable()
{
  DisableIntT3;
}

/*
void Timer2Config()
// Setup timer 2 registers for velocity calculation
{
  unsigned int timertick;

  // reset timer
  WriteTimer2(0);
  // TODO: blindly stolen from timer1. Change if needed. NO, YOU CHANGE IT!
  // TODO: non ho capito! (LC)
  // 4,82 msec (207.4Hz) TODO: perch� copi e non cambi i commenti?
  // one timertick is 1.6 us
  // TODO: un commento che si capisca pare di troppo.
  timertick = ((SPEEDLOOPTIME * 1000.0*1000.0) / 1.6);
  OpenTimer2(T4_ON & T4_GATE_OFF & T4_IDLE_STOP & T4_PS_1_64 &
    T4_SOURCE_INT, timertick );
}
*/

void Timer3Config()
// Setup timer 3 for I2T unwind when the board is faulted or in shutdown..
{
  // reset timer
  WriteTimer3(0);
  // should be at the same freq of 2FOC loop
  OpenTimer3(T3_ON & T3_GATE_OFF & T3_IDLE_STOP & T3_PS_1_1 &
  /*  T3_SYNC_EXT_OFF &*/ T3_SOURCE_INT, LOOPINTCY);
}

void Timer4Config()
// Setup timer 4 registers for CAN send
{
  unsigned int timertick;

  // reset timer
  WriteTimer4(0);
  // one timertick is 1.6 us
  timertick = ((CAN_OUTPUT_DATARATE * 100.0) / (1.6));
  OpenTimer4(T4_ON & T4_GATE_OFF & T4_IDLE_STOP & T4_PS_1_64 &
    T4_SOURCE_INT, timertick );
}

void Timer5Config()
// setup timer 5 for can setpoint timeout
{
  unsigned long timertick;
  WriteTimer5(0);

  // one timer tick is 6.4 us
  timertick = ((setpoint_watchdog_timeout ) / (6.4));

  // clamp to valid-range bounds.
  if(timertick > 0xffff) timertick = 0xffff;
  if(timertick == 0) timertick = 1;

  CloseTimer5();

  IFS1bits.T5IF = 0; // clear flag

  OpenTimer5(T5_ON  & T5_GATE_OFF & T5_IDLE_STOP & T5_PS_1_256 &
    T5_SOURCE_INT, timertick);
}


void oscConfig(void)
{
  /*  Configure Oscillator to operate the device at 40MIPS
  Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
  Fosc= 8M*40/(2*2)=80Mhz for 8M input clock */

  // set up the oscillator and PLL for 40 MIPS
  //            Crystal Frequency  * (DIVISOR+2)
  // Fcy =     ---------------------------------
  //              PLLPOST * (PRESCLR+2) * 4
  // Crystal  = Defined in UserParms.h
  // Fosc    = Crystal * dPLL defined in UserParms.h
  // Fcy    = DesiredMIPs

  if(OSCCONbits.COSC == 0b011){
  // already running on PLL (set by bootloader)
    // TODO decide wheter to trust the bootloader or
    // to switch back to EC or FRC and reconfigure
  return;
  }
  // dPLL derived in UserParms.h
  PLLFBD = (int)(DPLL * 4 - 2);
  // N1=2
  CLKDIVbits.PLLPOST = 0;
  // N2=2
  CLKDIVbits.PLLPRE = 0;

  __builtin_write_OSCCONH(0x03);    // Initiate Clock Switch to Primary Oscillator (EC) with PLL (NOSC=0b011)
  __builtin_write_OSCCONL(0x01);    // Start clock switching

  while(OSCCONbits.COSC != 0b011);  // Wait for PLL to lock
  while(OSCCONbits.LOCK != 1);
}

void ExternalFaultEnable()
// Enables the external fault (pushbutton) interrupt
{
  // Enable change notification for external fault
  //CNEN1 |= 0;
  CNEN2 |= 1<<13;
  // clear irq flag
  IFS1bits.CNIF = 0;
}


void OverCurrentFaultIntEnable()
// over current fault interrupt enable
{
  // clear irq flag
  IFS3bits.FLTA1IF = 0;
  //enable external fault interrupt
  IEC3bits.FLTA1IE = 1;
}

void SetupHWParameters(void)
// Setup variables related to HW implementation
// Current conversion factor and offset
//
{
  unsigned long pwmmax,pwmoffs;
  //
  // configure DSP core
  //

  // multiplication are NOT unsigned
  CORCONbits.US  = 0;
  // enable saturation on DSP data write
  CORCONbits.SATDW = 1;
  // ENABLE saturation on accumulator A.
  CORCONbits.SATA  = 1;

  //  do super saturation
  CORCONbits.ACCSAT  = 1;
  // program space not visible in data space
  CORCONbits.PSV  = 0;
  // conventional rounding mode
  CORCONbits.RND  = 1;

  // DSP multiplication are in fractional mode.
  CORCONbits.IF  = 0;

  //
  // ADC - Current Measure
  //

  // Current scaling constants: Determined by hardware design
  MeasCurrParm.qKa = DQKA;
  MeasCurrParm.qKb = DQKB;
  MeasCurrParm.qKc = DQKC;

  //
  // ADC - VDC-Link Measure
  //

  //
  // SV Generator
  //

  // Set PWM period to Loop Time
  //SVGenParm.iPWMPeriod = LOOPINTCY;

  // pwm registers range for pwm HW periph is 0 to LOOPINTCY
  pwmmax = LOOPINTCY;
  // calculate the given clamp offset for PWM regs given the
  // PWM guard percentage.
  pwmoffs = (pwmmax /100) * PWMGUARD;

  // PWM greater value accepted is PWM max minus the guard
  //SVGenParm.pwmmax = pwmmax - pwmoffs;
  // PWM smaller value accepted is PWM min (zero) plus the guard
  //SVGenParm.pwmmin = pwmoffs;

}

void InterruptPriorityConfig( void )
// Initialize interrupt priority registers
// WARNING: Unused interrupt are currently set to priority 0 (disabled !).
// WARNING: Faults are handled at max priority, even when int are diabled.
// Pay attention to race conditions (classic scenario:
//  lower priority int preempted by fault isr reenables all things
//  after fault isr terminates).
//
// The higher the number the highest the priority
//  7: FAULT(Overload+External)
//  6: ADCDMA
//  5: SPI(DMA)
//  4: CANTX-TIMER4
//  4: CAN RX
//  3: VELOCITY calc TIMER2
//  1: TIMER1... low freq.
{
  // all IPCx irqs disabled
  IPC0 = 0;
  IPC0bits.T1IP    = 1; // Timer1 (IDLE time operations) prioriy 1 (lowest)

  // all IPCx irqs disabled
  IPC1 = 0;
  IPC1bits.DMA0IP  = 6; // DMA0 (adc triggered) priority 6
  //IPC1bits.T2IP    = 3; // TIMER2 Velocity calculation timer

  // all IPCx irqs disabled
  IPC2 = 0;
  IPC2bits.SPI1IP  = 5; // SPI int priority 5
  IPC2bits.SPI1EIP = 5; // SPI error INT prioriy 5;
  IPC2bits.T3IP    = 5; // I2T when the 2foc loop is not running (fault,shutdown)

  // TODO . handle SPI error int.

  IPC3 = 0;
  IPC3bits.DMA1IP  = 4; // Can TX (DMA1) priority 4;
  IPC3bits.AD1IP   = 6; // ADC interrupt (if not in dma mode)

  IPC4 = 0;
  IPC4bits.CNIP    = 7; // external fault interrupt priority 7( highest,
  // not masked by DISI instruction. for fault)
  // incidentally this is also the same prio of the QEP rotor alignement (when int are used
  // instead of QEP hw)

  IPC5 = 0;

  IPC6 = 0;
  IPC6bits.DMA2IP   = 4; // Can RX (DMA2) priority 4;
  IPC6bits.T4IP     = 4; // CAN tx timer

  IPC7 = 0;

  IPC8 = 0;
  IPC8bits.C1RXIP   = 0; // can RX not in DMA mode
  IPC8bits.C1IP     = 4;
  //IPC8bits.C1IP1
  //IPC8bits.C1IP2

  IPC9 = 0;
  IPC9bits.DMA3IP   = 5; //DMA3 (spi) priority 5;
  
  IPC11 = 0;

  IPC14 = 0;
  IPC14bits.QEI1IP  = 3; // Quadrature encoder interrupt

  IPC15 = 0;
  IPC15bits.FLTA1IP = 7; // Fault max priority

  IPC16 = 0;
  IPC17 = 0;
  IPC18 = 0;

  // Ensure interrupt nesting is enabled
  INTCON1bits.NSTDIS = 0;
}

void SetupPorts_LED(void)
// config dsPIC pins for LED usage
{
  // LED Green and Red
  TRISAbits.TRISA4 = 0;
}

void SetupPorts_CAN(void)
// config dsPIC pins for CAN usage
{
  // CAN bus pin configuration:
  // Set RP4 as output CANTX (valid for both HW revisions)
  RPOR2bits.RP4R = 0b10000;

  // CANRx connesso a RP2
  RPINR26bits.C1RXR = 2;
}

void SetupPorts_EXTF(void)
// config dsPIC pins for External Fault usage
{
}

void SetupPorts_QE(void)
// config dsPIC pins for quadrature encoder usage
{
  // QEA su HEV
  // Connect QEI1 Phase A to RP5
  RPINR14bits.QEA1R = 5;
  // QEB su HEW
  // Connect QEI1 Phase B to RP6
  RPINR14bits.QEB1R = 6;
  // QEZ su HEU
  // Connect QEI1 Index to RP7
  RPINR15bits.INDX1R = 7;
  // RP5, RP6, RP7 are used for Quadrature encoders and for Hall Effect Sesnsors
  // Configure QEI or HES pins as digital inputs

  // TODO: verificare l'inizializzazione di ADPCFG
  ADPCFG |= (1<<5) | (1<<6) | (1<<7); // QEA/HESU=pin5, QEB/HESV=pin6, QEZ/HESW=pin7
}

void SetupPorts_OCF(void)
// config dsPIC pins for Over Current Fault usage
{
  // Overcurrent
  // Connect PWM overcurrent Fault to RP1
  RPINR12bits.FLTA1R = 1;
}

void SetupPorts_AI(void)
// config dsPIC pins for Analog Input usage
{
  // Analog inputs
  AD1PCFGLbits.PCFG0 = 0; // AN0 analog - IA
  AD1PCFGLbits.PCFG1 = 0; // AN1 analog - IB
  AD1PCFGLbits.PCFG2 = 0; // AN2 analog - VBUS
}

void SetupPorts_SPI(void)
// config dsPIC pins for SPI usage
{
  //  RP8 as SPI-SS (HV)
  // set port P8 as output on HV
  TRISBbits.TRISB8 = 0;
  // Slave select is driven in Bit Banging
  // P8 connected to B8 port
  RPOR4bits.RP8R = 0b0;
  // SPI1 Slave Select Output su rp1 HV molex
  // RPOR0bits.RP1R = 0b01001;

  //  B3 as SPI-CLK (HU)
  // set port RB3 as output
  TRISBbits.TRISB3 = 0;
  // SPI1 Clock Output su RP3 HU
  RPOR1bits.RP3R = 0b01000;

  // Connect SPI1 MISO on RP9. HW su molex
  RPINR20bits.SDI1R = 9;
}

void SetupPorts_DHES(void)
// config dsPIC pins for Digital Hall Effect Sensors usage
{
  // No particular init needed, pins already inputs
}

void SetupPorts_I2C(void)
{
    setupI2CTsens();
}

void SetupPorts( void )
//
//  init dsPic ports and peripheral mapping
//
{
  // set everything as input
  LATA  = 0x0000;
  TRISA = 0xFFFF;

  LATB  = 0x0000;
  TRISB = 0xFFFF;

  SetupPorts_LED();

  // define remappable peripheral allocation
  // see chapter 11 dsPic datasheet

  // issue an UNLOCK sequence to change OSCCON.IOLOCK (bit 6)
  __builtin_write_OSCCONL( OSCCON & ~0x40 );

  // default all pins to DIGITAL
  AD1PCFGL = 0xffff;

  // CAN
  SetupPorts_CAN();

  // External fault
  SetupPorts_EXTF();

  // Over Current Fault
  SetupPorts_OCF();

  // Analog inputs
  SetupPorts_AI();

#if defined(ENCODER_ABS) || defined(ENCODER_TLE)  || defined(ENCODER_AUX_ABS) || defined(ENCODER_AUX_TLE)
  // SPI encoders
  SetupPorts_SPI();
#endif
  
  // Quadrature encoders
  SetupPorts_QE();

#if defined (ENCODER_DHES) || defined(ENCODER_AUX_DHES)
  // Digital Hall Effect Sensors
  SetupPorts_DHES();
#endif

  // I2C port
  SetupPorts_I2C();

  // issue an LOCK sequence
  __builtin_write_OSCCONL( OSCCON | 0x40 );

  // reset change notification peripheral
  CNEN1 = 0;
  CNEN2 = 0;
}

void __attribute__((__interrupt__,no_auto_psv)) _CNInterrupt(void)
// pin change notification int handler
{
  // clear interrupt flag
  IFS1bits.CNIF = 0;

  // an external fault has been triggered
  if (ExternaFaultIsAsserted())
  {
    FaultExternalTriggered();
  }
}




void EepromTest()
// EMUROM checks (CRC and size)
{


  unsigned short eeprom_calculated_crc, eeprom_read_crc;

  eeprom_read_crc = ApplicationData.app_data_crc16;

  // dummy payload CRC used to calculate CRC itself.
  ApplicationData.app_data_crc16= 0;

  // compute payload CRC
  eeprom_calculated_crc = crc16(0,eeprom_payload_start, eeprom_payload_len);

  // restore payload CRC in RAM image
  ApplicationData.app_data_crc16 =  eeprom_read_crc;

#ifdef EEPROM_CRC_CALCULATE_AND_TX_ON_CAN
  ApplicationData.app_data_crc16 = eeprom_calculated_crc;
#else
  // check for payload CRC
  if(eeprom_calculated_crc != eeprom_read_crc){
    SysError.EMUROMCRCFault = 1;

  }
#endif
}

/*
void EepromSave()
// Update values in EEPROM RAM image and
// then write it onto the flash memory
{

  int runmode;
  SFRAC16 p,i,d, max;

  unsigned short eepromCRC;


  // the application data has been written over the default values
  ApplicationData.app_data_default = 0;
  // I2T parameters
  ApplicationData.I2TParam = I2Tdata.Param;
  ApplicationData.I2TThreshold = I2Tdata.IThreshold;

  // currently D and Q PID have the same parameters
  ControllerGetCurrentDPIDParm(&p,&i,&d, &max);
  //  ControllerGetCurrentQPIDParm(&p,&i,&d);
  // Current PI(D) parameters


  ApplicationData.CPIDP =  p;
  ApplicationData.CPIDI =  i;
  ApplicationData.CPIDD =  d;
  ApplicationData.CPIDM =  max;

  ControllerGetWPIDParm(&p,&i,&d, &max);

    // velocity PI(D) parameters
  ApplicationData.WPIDP = p;
  ApplicationData.WPIDI = i;
  ApplicationData.WPIDD = d;
  ApplicationData.WPIDM = max;

  // save run mode (torque, speed..)
  runmode = SysStatus.OpenLoop |
            ( SysStatus.TorqueControl << 1 ) |
            ( SysStatus.SpeedControl << 2 )  |
            ( SysStatus.PositionControl << 3 ) |
            ( SysStatus.TorqueSensorLoop << 4);

  ApplicationData.runmode = runmode;

  // save position limit buonds
  ApplicationData.position_limit_lower = position_limit_lower;
  ApplicationData.position_limit_upper = position_limit_upper;

  // save setpoints watchdog params
  ApplicationData.setpoint_watchdog_timeout = setpoint_watchdog_timeout;
  ApplicationData.setpoint_watchdog_enabled = setpoint_watchdog_enabled;


  ApplicationData.torque_q_limit_upper = torque_q_limit_upper;
  ApplicationData.torque_q_limit_lower = torque_q_limit_lower;

  ApplicationData.torque_d_limit_upper = torque_d_limit_upper;
  ApplicationData.torque_d_limit_lower = torque_d_limit_lower;

  ApplicationData.speed_limit_upper = speed_limit_upper;
  ApplicationData.speed_limit_lower = speed_limit_lower;

  ApplicationData.periodic_command1 = PeriodicMessageContents[0];
  ApplicationData.periodic_command2 = PeriodicMessageContents[1];
  ApplicationData.periodic_command3 = PeriodicMessageContents[2];
  ApplicationData.periodic_command4 = PeriodicMessageContents[3];

  //ApplicationData.Ira_qVq = IraqVq_max;
  //ApplicationData.Ira_direction = Ira_direction;
  //ApplicationData.Ira_ramp_steepness = Ira_ramp_steepness;

  // dummy CRC to calculate dummy CRC itself
  ApplicationData.app_data_crc16 = 0;

  eepromCRC = crc16(0,eeprom_payload_start, eeprom_payload_len);
  ApplicationData.app_data_crc16 = eepromCRC;
  // Reflash to the EEPROM
  REFLASH_EMU_ROM(EMURomSpace,ApplicationData);
}

void EepromLoad()
// Refresh RAM copy reading eeprom data, then
// "unpack" this data updating every single
// variables or calling every single nedded function
{
  int runmode;


  // I2T parameters
  I2Tdata.Param =  ApplicationData.I2TParam;
  I2Tdata.IThreshold = ApplicationData.I2TThreshold;

  // currently D and Q PID have the same parameters
  // Current PI(D) parameters
  ControllerSetCurrentDPIDParm(ApplicationData.CPIDP,ApplicationData.CPIDI,ApplicationData.CPIDD,ApplicationData.CPIDM);
  ControllerSetCurrentQPIDParm(ApplicationData.CPIDP,ApplicationData.CPIDI,ApplicationData.CPIDD,ApplicationData.CPIDM);


  // velocity PI(D) parameters
  ControllerSetWPIDParm(ApplicationData.WPIDP,ApplicationData.WPIDI,ApplicationData.WPIDD,ApplicationData.WPIDM);

  // set operation mode (torque, speed..)
  runmode = ApplicationData.runmode;

  SysStatus.OpenLoop = !!(runmode & 0x01);
  SysStatus.TorqueControl = !!(runmode & 0x02);
  SysStatus.SpeedControl = !!(runmode & 0x04);
  SysStatus.PositionControl = !!(runmode & 0x08);
  SysStatus.TorqueSensorLoop = !!(runmode & 0x10);

  // update position limits.
  position_limit_lower = ApplicationData.position_limit_lower;
  position_limit_upper = ApplicationData.position_limit_upper;

  setpoint_watchdog_timeout = ApplicationData.setpoint_watchdog_timeout;
  setpoint_watchdog_enabled = ApplicationData.setpoint_watchdog_enabled;

  torque_q_limit_upper = ApplicationData.torque_q_limit_upper;
  torque_q_limit_lower = ApplicationData.torque_q_limit_lower;

  torque_d_limit_upper = ApplicationData.torque_d_limit_upper;
  torque_d_limit_lower = ApplicationData.torque_d_limit_lower;

  speed_limit_upper = ApplicationData.speed_limit_upper;
  speed_limit_lower = ApplicationData.speed_limit_lower;

  PeriodicMessageContents[0] = ApplicationData.periodic_command1;
  PeriodicMessageContents[1] = ApplicationData.periodic_command2;
  PeriodicMessageContents[2] = ApplicationData.periodic_command3;
  PeriodicMessageContents[3] = ApplicationData.periodic_command4;

  //IraqVq_max = ApplicationData.Ira_qVq;
  //Ira_direction = ApplicationData.Ira_direction;
  //Ira_ramp_steepness = ApplicationData.Ira_ramp_steepness;
}
*/
