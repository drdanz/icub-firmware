
/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------


#include "stdlib.h"
#include "string.h"
#include "EoCommon.h"
#include "EOMtask.h"

#include "EOMtheEMSappl.h"
#include "EOMtheEMSapplCfg.h"


#include "EoError.h"
#include "EOtheErrorManager.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "EOMtheEMSconfigurator.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "EOMtheEMSconfigurator_hid.h" 


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// marco.accame on Nov 26 2014: the user-defined emsconfigurator_evt_userdef is triggered by the EOtheEMSapplBody to
// tell the EOMtheEMSconfigurator that has received a CAN frame.

extern void eom_emsconfigurator_hid_userdef_ProcessUserdef00Event(EOMtheEMSconfigurator* p)
{
#if 0
    eOresult_t  res;
    uint8_t     numofRXcanframe = 0;

    
    EOappCanSP  *appcanSP = eo_emsapplBody_GetCanServiceHandle(eo_emsapplBody_GetHandle());
    EOappTheDB  *db = eo_emsapplBody_GetDataBaseHandle(eo_emsapplBody_GetHandle());
   
    res = eo_appCanSP_GetNumOfRecCanframe(appcanSP, eOcanport1, &numofRXcanframe);
    if(eores_OK != res)
    {
        return;
    }
    

    eo_appCanSP_read(appcanSP, eOcanport1, numofRXcanframe, NULL);
    

    // before version 1.70, this part was skipped if the skin was connected on CAN bus 2
    numofRXcanframe = 0;
    res = eo_appCanSP_GetNumOfRecCanframe(appcanSP, eOcanport2, &numofRXcanframe);
    if(eores_OK != res)
    { // se c'e un errore sui parametri passati ovvero mai., 
        return;
    }

    eo_appCanSP_read(appcanSP, eOcanport2, numofRXcanframe, NULL);  
#endif    
}


// marco.accame on Jan 15 2015: the user-defined emsconfigurator_evt_userdef01 is triggered by a timer inside the EOtheEMSapplBody to
// tell the that we must do again of verification of presence of can boards.

extern void eom_emsconfigurator_hid_userdef_ProcessUserdef01Event(EOMtheEMSconfigurator* p)
{
#if 0
    uint32_t readyCanBoardsMask = 0;    // keeps the boards that are ready. if bit pos i-th is 1, then the board in list i-th is OK   
    uint32_t checkedmask = 0;           // keeps the boards that are checked
    static uint32_t count_times = 0;
    count_times++;
    /*
    // marco.accame on 4 feb 2014:
    // if the 2foc receives a get-fw-version request in initial period of life of the application then it remains silent forever.
    // in the meantime that this bug is fixed in the 2foc application, we introduce the following work-around.
    // we wait for 500 ticks, each of 10 ms for a total of 5 seconds before we send the can message.
    // it is a lot of time (maybe 3 seconds is enough), but in this way we are sure that the the 2foc is surely out of the bootloader
    // and beyond its first second of life (the timer starts after not before 1-2 seconds of bootstrap after the ipnet has connected
    // the sockets with ip address 10.0.1.104)
    // to remove the delay: just change onlyAFTER to value 1 (not 0!!!)
    static uint32_t times = 0;
    static const uint32_t onlyAFTER = 500; 
    
    times++;
    
    
//    if(times == 1)
//    {    
//        eo_emsapplBody_discovery_Mais_Start(eo_emsapplBody_GetHandle());
//    }
    
    if(times < onlyAFTER)
    {
        return;
    }    
    else if(times == onlyAFTER)
    {
        uint32_t dontaskmask = 0; // the first time we ask to every board
        eo_emsapplBody_checkCanBoardsAreReady(eo_emsapplBody_GetHandle(), dontaskmask); 
        
        // marco.accame on 24feb14: if we launch it only in here then it is not OK.
        // redmine issue #494
        // eo_emsapplBody_discovery_Mais_Start(eo_emsapplBody_GetHandle());
        
        //eo_emsapplBody_discovery_Mais_Start(eo_emsapplBody_GetHandle());
        //eo_emsapplBody_GetAppRunMode(eo_emsapplBody_GetHandle());
        
        return;        
    }
    */
    //  davide on 25 feb 2015:
    //  it seems that the problem of the 2foc is due to the high frequency of the messages sent via CAN.
    //  Now the messages are sent with a frequency of 4HZ (timer countdown = 250ms), and the problem should be fixed without
    //  a fixed delay of 5 seconds from the beginning of the application
    
    // The first time I only send the request...from that point on, I continue to check if the boards are ready, and if
    // not I re-send the request for the firmware version
    if(count_times == 1)
    {
        uint32_t dontaskmask = 0; // the first time we ask to every board
        eo_emsapplBody_checkCanBoardsAreReady(eo_emsapplBody_GetHandle(), dontaskmask);
        return;
    }
        
    EOappTheDB  *db = eo_emsapplBody_GetDataBaseHandle(eo_emsapplBody_GetHandle());      

    // verifico che le board mc4 ed 1foc siano ready, ovvero che abbiamo mandato la loro fw version
    if(eobool_true == eo_appTheDB_areConnectedCanBoardsReady(db, &readyCanBoardsMask, &checkedmask))
    {
        // se tutte le 1foc e le mc4 sono ready, allora setto che lo sono (stoppo il timer canBoardsReady_timer da 10 milli)
        // e poi mando la configurazione alle board can
        eo_emsapplBody_checkCanBoards_Stop(eo_emsapplBody_GetHandle());
        eo_emsapplBody_sendConfig2canboards(eo_emsapplBody_GetHandle());
        // poi abilito MAIS e BCastPolicy nel caso di MC4
        eOmn_appl_runMode_t appl_run_mode = eo_emsapplBody_GetAppRunMode(eo_emsapplBody_GetHandle());
        if((applrunMode__skinAndMc4 == appl_run_mode) || (applrunMode__mc4Only == appl_run_mode))
        {   
            eo_emsapplBody_MAISstart(eo_emsapplBody_GetHandle());
        }
    }
    else
    {  
        // i check again if the can boards are ready. however, i dont check the boards already ready
        eo_emsapplBody_checkCanBoardsAreReady(eo_emsapplBody_GetHandle(), readyCanBoardsMask);
    }
#endif
    
    //after 10 seconds, go 2 run
    /*
    static uint32_t count_times = 0;
    count_times++;
    
    if (count_times == 40)
        eom_task_SetEvent(eom_emsconfigurator_GetTask(eom_emsconfigurator_GetHandle()), emsconfigurator_evt_go2runner);
    */
}

// marco.accame on Nov 26 2014: this function is triggered if function eom_emssocket_Transmit() fails
// to transmit a udp packet.
extern void eom_emsconfigurator_hid_userdef_onemstransceivererror(EOMtheEMStransceiver* p)
{    
    eOerrmanDescriptor_t errdes = {0};
    errdes.code             = eoerror_code_get(eoerror_category_System, eoerror_value_SYS_configurator_udptxfailure);
    errdes.par16            = 0;
    errdes.par64            = 0;
    errdes.sourcedevice     = eo_errman_sourcedevice_localboard;
    errdes.sourceaddress    = 0;    
    eo_errman_Error(eo_errman_GetHandle(), eo_errortype_warning, NULL, "EOMtheEMSconfigurator", &errdes); 
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------




