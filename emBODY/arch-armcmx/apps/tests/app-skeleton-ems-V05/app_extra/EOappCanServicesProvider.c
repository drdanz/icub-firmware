/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
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
// - doxy
// --------------------------------------------------------------------------------------------------------------------

/* @file       EOMappCanServicesProvider.c
    @brief      This file implements CAN services.
    @author    valentina.gaggero@iit.it
    @date       02/14/2012
**/



// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

//sys
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

//abs
#include "hal.h"
#include "hal_debugPin.h"

//embobj
#include "EoCommon.h"
#include "EOtheMemoryPool.h"
#include "EOtheErrorManager.h"
#include "EOfifoWord_hid.h"
#include "EOconstvector_hid.h"

//embobj-icub
#include "EOicubCanProto.h"
#include "EOicubCanProto_specifications.h"
#include "EOemsCanNetworkTopology.h"
#include "EOemsCanNetworkTopology_hid.h"
#include "EoMotionControl.h"

//embobj-icub-cfg
#include "eOcfg_emsCanNetworkTopology.h"
#include "eOcfg_icubCanProto_messagesLookupTbl.h"


#include "EOappTheNVmapRef.h" //==> included to clear skin array
#include "eOcfg_nvsEP_sk.h"   //==> included to clear skin array
#include "EOarray.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------
#include "EOappCanServicesProvider.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
#include "EOappCanServicesProvider_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables. deprecated: better using _get(), _set() on static variables 
// --------------------------------------------------------------------------------------------------------------------
uint8_t app_canSP_flag = 0;


// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
static eOresult_t s_eo_appCanSP_canPeriphInit(EOappCanSP *p);
//static eOresult_t s_eo_ap_canModule_formAndSendFrame(eo_icubCanProto_msgCommand_t cmd, 
//                                                     void *val_ptr,
//                                                     eo_icubCanProto_msgDestination_t boardAddr,
//                                                     eOcanport_t canPort,
//                                                     eOcanframe_t *frame);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section
static uint32_t error_count = 0;
static uint32_t error_tr5 = 0;
static uint32_t error_general = 0;
// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------
extern EOappCanSP* eo_appCanSP_New(eOappCanSP_cfg_t *cfg)
{
    eOresult_t      res;
    EOappCanSP      *retptr = NULL;
    EOicubCanProto  *icubCanProto_ptr;
    EOemsCanNetTopo *emsCanNetTopo_ptr;


    eo_emsCanNetTopo_cfg_t   emsCanNetTopo_cfg = 
    {
        EO_INIT(.emsCanNetTopo_joints__ptr)     eo_cfg_emsCanNetTopo_constvec_joints__ptr,
        EO_INIT(.emsCanNetTopo_motors__ptr)     eo_cfg_emsCanNetTopo_constvec_motors__ptr,
        EO_INIT(.emsCanNetTopo_sensors__ptr)    eo_cfg_emsCanNetTopo_constvec_sensors__ptr,
        EO_INIT(.emsCanNetTopo_skin__ptr)       eo_cfg_emsCanNetTopo_constvec_skin__ptr,
    };


    eo_icubCanProto_cfg_t icubCanProto_cfg = 
    {
        EO_INIT(.msgClasses_LUTbl__ptr)     icubCanProto_msgClasses_lookupTbl__ptr 
    };



//2) initialise emsCanNetTopology (the EOtheBoardTransceiver MUST be inited)
//    emsCanNetTopo_cfg.nvsCfg = eo_boardtransceiver_hid_GetNvsCfg();
    emsCanNetTopo_ptr = eo_emsCanNetTopo_New(&emsCanNetTopo_cfg);
    if(NULL == emsCanNetTopo_ptr)
    {
        return(retptr);
    }


//3) initialise icubCanProto
    icubCanProto_cfg.emsCanNetTopo__ptr = emsCanNetTopo_ptr; 
    icubCanProto_ptr = eo_icubCanProto_New(&icubCanProto_cfg);
    if(NULL == icubCanProto_ptr)
    {
        return(retptr);
    }

//4) create the obj (i get the memory for the object)
    retptr = eo_mempool_GetMemory(eo_mempool_GetHandle(), eo_mempool_align_32bit, sizeof(EOappCanSP), 1);


    retptr->icubCanProto_ptr = icubCanProto_ptr;
    retptr->emsCanNetTopo_ptr = emsCanNetTopo_ptr;

    // 1) initialise peritheral
    res = (eOresult_t)s_eo_appCanSP_canPeriphInit(retptr);
    if(eores_OK != res)
    {
        return(retptr);
    }

    return(retptr);
}

extern eOresult_t eo_appCanSP_GetConnectedJoints(EOappCanSP *p,EOarray *connectedJointsList)
{
    return(eo_emsCanNetTopo_GetConnectedJoints(p->emsCanNetTopo_ptr, connectedJointsList));
}


extern eOresult_t eo_appCanSP_GetConnectedMotors(EOappCanSP *p, EOarray *connectedMotorsList)
{
    return(eo_emsCanNetTopo_GetConnectedMotors(p->emsCanNetTopo_ptr, connectedMotorsList));
}

extern eOresult_t eo_appCanSP_GetConnectedSensors(EOappCanSP *p, EOarray *connectedSensorsList)
{
    return(eo_emsCanNetTopo_GetConnectedSensors(p->emsCanNetTopo_ptr, connectedSensorsList));
}

extern eOresult_t eo_appCanSP_GetConnectedSkin(EOappCanSP *p, EOarray *connectedSkinList)
{
    return(eo_emsCanNetTopo_GetConnectedSkin(p->emsCanNetTopo_ptr, connectedSkinList));
}


extern eOresult_t eo_appCanSP_SendCmd2Joint(EOappCanSP *p, eOmc_jointId_t jId, eo_icubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOcanframe_t                                canFrame;
    eo_icubCanProto_msgDestination_t            dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t  canLoc;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_emsCanNetTopo_GetJointCanLocation_ByJointId(p->emsCanNetTopo_ptr, jId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message (one for all msg)
    dest.axis = canLoc.axis;
    dest.canAddr = canLoc.canaddr;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);

    return(res);
}


extern eOresult_t eo_appCanSP_SendCmd2Motor(EOappCanSP *p, eOmc_motorId_t mId, eo_icubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOcanframe_t                                canFrame;
    eo_icubCanProto_msgDestination_t            dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t  canLoc;

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_emsCanNetTopo_GetMotorCanLocation_ByMotorId(p->emsCanNetTopo_ptr, mId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message (one for all msg)
    dest.axis = canLoc.axis;
    dest.canAddr = canLoc.canaddr;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);

    return(res);
}


extern eOresult_t eo_appCanSP_SendCmd(EOappCanSP *p, eo_appCanSP_canLocation *canLocation, eo_icubCanProto_msgCommand_t msgCmd, void *val_ptr)
{
    eOresult_t                                  res;
    eOcanframe_t                                canFrame;
    eo_icubCanProto_msgDestination_t            dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t  *canLoc = (eo_emsCanNetTopo_jointOrMotorCanLocation_t *)canLocation;

    //set destination of message 
    dest.axis = canLoc->axis;
    dest.canAddr = canLoc->canaddr;

    if((NULL == p) | (NULL == canLoc))
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc->emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);

    return(res);
}

extern eOresult_t eo_appCanSP_GetJointCanLocation(EOappCanSP *p, eOmc_jointId_t jId, eo_appCanSP_canLocation *canLoc, eObrd_types_t *boardType)
{
    return(eo_emsCanNetTopo_GetJointCanLocation_ByJointId(p->emsCanNetTopo_ptr, jId, 
                                                          (eo_emsCanNetTopo_jointOrMotorCanLocation_t*)canLoc, 
                                                          boardType));
}


extern eOresult_t eo_appCanSP_GetMotorCanLocation(EOappCanSP *p, eOmc_motorId_t mId, eo_appCanSP_canLocation *canLoc, eObrd_types_t *boardType)
{
    return(eo_emsCanNetTopo_GetMotorCanLocation_ByMotorId(p->emsCanNetTopo_ptr, mId, 
                                                          (eo_emsCanNetTopo_jointOrMotorCanLocation_t*)canLoc, 
                                                          boardType));
}

extern eOresult_t eo_appCanSP_read(EOappCanSP *p)
{
#define MOTOR_MSG_MAX_RX 10

    eOresult_t res;
    hal_can_frame_t rec_frame;
    uint8_t  i;
    uint8_t remainingMsg = 100;
    hal_can_port_t canPort = hal_can_port2;
    void *memRef = NULL;
    EOarray *sk_array = NULL;
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    //1) init operation
    res = eo_appTheNVmapRef_GetSkinNVMemoryRef(eo_appTheNVmapRef_GetHandle(), 0, skinNVindex_sstatus__arrayof10canframe, &memRef);
    if(eores_OK != res)
    {
        return(res);
    }

    sk_array = (EOarray*)memRef;
    eo_array_Reset(sk_array);

    /* put here other init stuf before start reading from CAN */


    //2) read messages
    for(i=0; ((i<MOTOR_MSG_MAX_RX) && (0!= remainingMsg)) ; i++) //while(0!= remainingMsg)
    {
        res = (eOresult_t)hal_can_get(canPort, &rec_frame, &remainingMsg);
        if(eores_OK != res) 
        {
            if(eores_NOK_nodata == res)
            {
                return(eores_OK);
            }
            else
            {
                return(res); //error management
            }
                                     
        }
//        rec_frame.id = 0x040;
//        rec_frame.data[0] = 0;
        DEBUG_PIN3_ON;
        DEBUG_PIN4_ON;
        res = eo_icubCanProto_ParseCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&rec_frame, (eOcanport_t)canPort);
        if(eores_OK != res)
        {
            if(eores_NOK_generic == res) //array is full!!!
            {
                //verifico se e' un errore sul triangolo 5.
                if((rec_frame.id & 0x00f) == 5)
                {
                    error_tr5 ++;
                }
                else
                {
                    error_count++;
                }    
            }
            else
            {
                error_general++;
            }

        }
    }
    return(res);

}

extern void eo_appCanSP_ResetCanFifo(EOappCanSP *p)
{
    eOresult_t      res;
    hal_can_frame_t rec_frame;
    uint8_t         i, recCanFrame;
    hal_can_port_t  canPort = hal_can_port2;
    uint8_t         remainingMsg = 0;

    res = (eOresult_t)hal_can_get(canPort, &rec_frame, &remainingMsg);
    for(i = 0; i<remainingMsg; i++)
    {
        hal_can_get(canPort, &rec_frame, NULL);
    }
}
    
extern eOresult_t eo_appCanSP_readOnlySkin_TEST(EOappCanSP *p)
{
#define MOTOR_MSG_MAX_RX 10

    eOresult_t res;
    hal_can_frame_t rec_frame;
    uint8_t  i, recCanFrame;
    uint8_t remainingMsg = 0;
    hal_can_port_t canPort = hal_can_port2;
    void *memRef = NULL;
    EOarray *sk_array = NULL;
    char str[64];
    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    //1) init operation
    res = eo_appTheNVmapRef_GetSkinNVMemoryRef(eo_appTheNVmapRef_GetHandle(), 0, skinNVindex_sstatus__arrayof10canframe, &memRef);
    if(eores_OK != res)
    {
        return(res);
    }

    sk_array = (EOarray*)memRef;
    eo_array_Reset(sk_array);

    /* put here other init stuf before start reading from CAN */
    recCanFrame = 0;
    while((recCanFrame < MOTOR_MSG_MAX_RX) && (1 == app_canSP_flag))
    {
        if(p->newrecframe_port2)
        {
            
            res = (eOresult_t)hal_can_get(canPort, &rec_frame, &remainingMsg);
            
//             snprintf(str, sizeof(str)-1, "n = %d", remainingMsg);
//             //eo_errman_Error(eo_errman_GetHandle(), eo_errortype_warning, "", str); 
//                 hal_trace_puts(str);
            
            p->newrecframe_port2 = 0;
            if(remainingMsg>=14)
            {
//                eo_errman_Error(eo_errman_GetHandle(), eo_errortype_fatal, "", "");  
            }
            if(res == eores_OK)
            {
                recCanFrame++;
                res = eo_icubCanProto_ParseCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&rec_frame, (eOcanport_t)canPort);
                if(eores_OK != res)
                {
                    if(eores_NOK_generic == res) //array is full!!!
                    {
//                        eo_errman_Error(eo_errman_GetHandle(), eo_errortype_fatal, "", "");  
                    }
                }
            }
            else
            {
                remainingMsg = 0;
            }
            for(i = 0; ((i<remainingMsg) && (i < (MOTOR_MSG_MAX_RX- recCanFrame))); i++)
            {
                res = (eOresult_t)hal_can_get(canPort, &rec_frame, NULL);
                if(res == eores_OK)
                {
                    recCanFrame++;
                    res = eo_icubCanProto_ParseCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&rec_frame, (eOcanport_t)canPort);
                    if(eores_OK != res)
                    {
                        if(eores_NOK_generic == res) //array is full!!!
                        {
//                            eo_errman_Error(eo_errman_GetHandle(), eo_errortype_fatal, "", "");  
                        }
                    }
                }

            }
        }

            
            
//         if(eores_OK != res) 
//         {
//             if(eores_NOK_nodata == res)
//             {
//                 return(eores_OK);
//             }
//             else
//             {
//                 return(res); //error management
//                 //eo_errman_Error(p, eo_errortype_fatal, "", "");  
//             }
//                                      
//         }
//         if(res == eores_OK)
//         {
//             res = eo_icubCanProto_ParseCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&rec_frame, (eOcanport_t)canPort);
//             if(eores_OK != res)
//             {
//                 if(eores_NOK_generic == res) //array is full!!!
//                 {
//                     eo_errman_Error(eo_errman_GetHandle(), eo_errortype_fatal, "", "");  
//                 }
//             }
//         }
    }
    return(res);

}


extern eOresult_t eo_appCanSP_ReadSkin(EOappCanSP *p, eOsk_skinId_t skId)
{
    eOresult_t res;
    eo_emsCanNetTopo_sensorCanLocation_t canLoc;
    uint8_t remainingMsg = 1; 
    hal_can_frame_t rec_frame;


    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    // 1) get port on witch skin is connected to.
    res = eo_emsCanNetTopo_GetskinCanLocation_BySkinId(p->emsCanNetTopo_ptr, skId, &canLoc);
    if(eores_OK != res)
    {
        return(res);    
    }
    
    while((remainingMsg > 0) || (eores_OK == res))
    {
        res = (eOresult_t)hal_can_get((hal_can_port_t)canLoc.emscanport, &rec_frame, &remainingMsg);
        if(eores_OK != res) 
        {
            if(eores_NOK_nodata == res)
            {
                return(eores_OK);
            }
            else
            {
                return(res); //error management
            }
                                     
        }
        res = eo_icubCanProto_ParseSkinCanFrame(p->icubCanProto_ptr, (eOcanframe_t*)&rec_frame, canLoc.emscanport);
    }
    return(res);
}



                  
extern eOresult_t eo_appCanSP_ConfigSkin(EOappCanSP *p, eOsk_skinId_t skId)
{
    eOresult_t res;
    uint8_t i;
    eOcanframe_t canFrame;
    eo_icubCanProto_msgDestination_t dest;
    eo_emsCanNetTopo_sensorCanLocation_t canLoc;
    eo_icubCanProto_msgCommand_t msgCmd = 
    {
        EO_INIT(.class) 4,
        EO_INIT(.cmdId) ICUBCANPROTO_POL_SK_CMD__TACT_SETUP
    };

    //eo_icubCanProto_msgCmdClass_pollingSensorBoard

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_emsCanNetTopo_GetskinCanLocation_BySkinId(p->emsCanNetTopo_ptr, 0, &canLoc);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message (one for all msg)
    dest.axis = 0;
    dest.canAddr = 0;
    
    // 1) form can frame to configure all skin boards
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }

    for(i=8; i<15; i++)
    {
//        dest.canAddr = i;
        canFrame.id = 0x200 |(i&0xF);
        res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
        if(eores_OK != res)
        {
            return(res);
        }
    }
    return(eores_OK);
}




extern eOresult_t eo_appCanSP_ConfigMc4(EOappCanSP *p)
{
    return(eores_OK);
}


extern eOresult_t eo_appCanSP_Config2foc(EOappCanSP *p)
{
    return(eores_OK);
}


/*****************************************************************************************************************/
/*                   OBSOLETE FUNCTIONS                                                                         */
/*****************************************************************************************************************/


extern eOresult_t eo_appCanSP_jointWholeConfig_Send(EOappCanSP *p, eOmc_jointId_t jId, eOmc_joint_config_t *cfg)
{
    eOresult_t                                  res;
    eOcanframe_t                                canFrame;
    eObrd_types_t                               boardType;
    eo_icubCanProto_msgDestination_t            dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t  canLoc;
    eo_icubCanProto_msgCommand_t                msgCmd = 
    {
        EO_INIT(.class) eo_icubCanProto_msgCmdClass_pollingMotorBoard,
        EO_INIT(.cmdId) 0
    };

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    res = eo_emsCanNetTopo_GetJointCanLocation_ByJointId(p->emsCanNetTopo_ptr, jId, &canLoc, &boardType);
    if(eores_OK != res)
    {
        return(res);
    }

    //currently no joint config param must be sent to 2foc board. (for us called 1foc :) )
    //(currently no pid velocity is sent to 2foc)
    if(eobrd_1foc == boardType)
    {
        return(eores_OK);
    }


    //set destination of message (one for all msg)
    dest.axis = canLoc.axis;
    dest.canAddr = canLoc.canaddr;

    // 1) send pid position 
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_POS_PID;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidposition, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }


    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_POS_PIDLIMITS;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidposition, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 2) send torque pid
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_TORQUE_PID;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidtorque, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }


    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_TORQUE_PIDLIMITS;

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidtorque, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }


//    // 3) send velocity pid DA chiedere. quale msg devo mandare alla 4dc per pid vel
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_VELOCITY_PID;
//
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidvelocity, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//
//
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_VELOCITY_PIDLIMITS;
//
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidvelocity, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }

    // 4) set min position
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_MIN_POSITION;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->minpositionofjoint, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 5) set max position   
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_MAX_POSITION;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->maxpositionofjoint, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

//    // 6) set vel shift    ==>REMOVED BECAIUSE OBSOLETE
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_VEL_SHIFT;
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->velocityshiftfactor, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }

    // 6) set vel timeout
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_VEL_TIMEOUT;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->velocitysetpointtimeout, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 7) set impedance
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_IMPEDANCE_PARAMS;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->impedance, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_IMPEDANCE_OFFSET;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->impedance, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

//    // 7) set speed etim shift    COSA METTO COME CAMPO DELL NVS???
//    res = eo_icubCanProto_formCanFrame4MotorBoard(ICUBCANPROTO_POL_MB_CMD__SET_SPEED_ESTIM_SHIFT, (void*)&cfg->velocitysetpointtimeout,
//                                                  boardAddr, axis, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }

//questi sono dei comandi, quando li mando?

//    // 8) set controller idle
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__CONTROLLER_IDLE;
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//
//    // 9) set pwm pad
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__DISABLE_PWM_PAD;
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }

    return(eores_OK);
}







extern eOresult_t eo_appCanSP_ConfigMotor(EOappCanSP *p, eOmc_motorId_t mId, eOmc_motor_config_t *cfg)
{
    eOresult_t res;
    eOcanframe_t canFrame;
    eo_icubCanProto_msgDestination_t dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t canLoc;
    eo_icubCanProto_msgCommand_t msgCmd = 
    {
        EO_INIT(.class) eo_icubCanProto_msgCmdClass_pollingMotorBoard,
        EO_INIT(.cmdId) 0
    };

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }

    res = eo_emsCanNetTopo_GetMotorCanLocation_ByMotorId(p->emsCanNetTopo_ptr, mId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    //set destination of message (one for all msg)
    dest.axis = canLoc.axis;
    dest.canAddr = canLoc.canaddr;


    // 1) send current pid
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_PID;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidcurrent, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 2) send current pid limits
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_PIDLIMITS;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->pidcurrent, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 3) set max velocity   
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_MAX_VELOCITY;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->maxvelocityofmotor, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    // 4) set current limit  
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_LIMIT;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)&cfg->maxcurrentofmotor, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }


    // 5) set perriodic msg contents  ??? DOVE PRENDO L'INFO
//    res = eo_icubCanProto_formCanFrame4MotorBoard(ICUBCANPROTO_POL_MB_CMD__SET_CURRENT_LIMIT, (void*)&cfg->maxcurrentofmotor,
//                                                  boardAddr, axis, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }

#warning: VALE-> manca il campo i2t in motor config!!!!
    // 6) set i2t param      MANCANO!!!
//    res = eo_icubCanProto_formCanFrame4MotorBoard(ICUBCANPROTO_POL_MB_CMD__SET_I2T_PARAMS, (void*)&cfg->velocityshiftfactor,
//                                                  boardAddr, axis, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }


//enable pwm and control loop

    // 5) set pwm pad
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__ENABLE_PWM_PAD;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }


    // 6) set controller run
    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__CONTROLLER_RUN;
    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }
    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

//#warning aggiunto un messaggio x test!!!    
//    // 7) set controller run
//    msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__DISABLE_PWM_PAD;
//    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, NULL, &canFrame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
    return(eores_OK);
}







//extern eOresult_t eo_ap_canMod_readMotorBoardMessages(uint8_t *readMsg, uint8_t *remainingMsg)
//{
//    hal_result_t res = hal_res_OK;
//    hal_can_frame_t rec_frame;
//    uint8_t  i;
//
//    for(i=0; i<MOTOR_MSG_MAX_RX; i++)
//    {
//        res = hal_can_get(hal_can_port1, &rec_frame, remainingMsg);
//        if(hal_res_OK != res) 
//        {
//            if(hal_res_NOK_nodata == res)
//            {
//                *remainingMsg = 0;
//                return(eores_OK);
//            }
//            else
//            {
//                return((eOresult_t)res); //error management
//            }
//            
//        }
//#warning X_TEST
//        rec_frame.id = 0x040;
//        rec_frame.data[0] = 0;
//        DEBUG_PIN3_ON;
//        eo_icubCanProto_parseCanFrame((eOcanframe_t*)&rec_frame, (eOcanport_t)hal_can_port1);
//    }
//    *readMsg = i;
//
//    return(eores_OK);
//}

//extern eOresult_t eo_ap_canMod_readSensorBoardMessages(uint8_t *readMsg, uint8_t *remainingMsg)
//{
//
//    hal_can_frame_t rec_frame;
//    uint8_t i;
//    hal_result_t res = hal_res_OK;
//
//    for(i=0; i<SENSOR_MSG_MAX_RX; i++)
//    {
//        res = hal_can_get(hal_can_port2, &rec_frame, remainingMsg);
//        if(hal_res_OK != res) 
//        {
//            if(hal_res_NOK_nodata != res)
//            {
//                *remainingMsg = 0;
//                return(eores_OK);
//            }
//            else
//            {
//                return((eOresult_t)res); //error management
//            }
//            
//        }
//        eo_icubCanProto_parseCanFrame((eOcanframe_t*)&rec_frame, (eOcanport_t)hal_can_port1);
//    }
//    *readMsg = i;
//
//    return(eores_OK);
//}







extern eOresult_t eo_appCanSP_SendSetPoint(EOappCanSP *p, eOmc_jointId_t jId, eOmc_setpoint_t *setPoint)
{
    eOresult_t res;
    eOcanframe_t canFrame;
    eo_icubCanProto_msgDestination_t dest;
    eo_emsCanNetTopo_jointOrMotorCanLocation_t canLoc;
    void *val_ptr = NULL;
    eo_icubCanProto_msgCommand_t msgCmd = 
    {
        EO_INIT(.class) eo_icubCanProto_msgCmdClass_pollingMotorBoard,
        EO_INIT(.cmdId) 0
    };

    if(NULL == p)
    {
        return(eores_NOK_nullpointer);
    }
    res = eo_emsCanNetTopo_GetJointCanLocation_ByJointId(p->emsCanNetTopo_ptr, jId, &canLoc, NULL);
    if(eores_OK != res)
    {
        return(res);
    }

    dest.axis = canLoc.axis;
    dest.canAddr = canLoc.canaddr;


    switch( setPoint->type)
    {
        case eomc_setpoint_position:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__POSITION_MOVE; 
            val_ptr =  &(setPoint->to.position);    
        }break;

        case eomc_setpoint_velocity:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__VELOCITY_MOVE;                 
            val_ptr =  &(setPoint->to.velocity);    
        }break;

        case eomc_setpoint_torque:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_DESIRED_TORQUE;           
            val_ptr =  &(setPoint->to.torque.value);    
        }break;

        case eomc_setpoint_current:
        {
            msgCmd.cmdId = ICUBCANPROTO_POL_MB_CMD__SET_DISIRED_CURRENT;                             
            val_ptr =  &(setPoint->to.current.value);    
        }break;

        default:
        {
            return(eores_NOK_unsupported);
        }
    }

    res = eo_icubCanProto_FormCanFrame(p->icubCanProto_ptr, msgCmd, dest, (void*)val_ptr, &canFrame);
    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_put((hal_can_port_t)canLoc.emscanport, (hal_can_frame_t*)&canFrame, hal_can_send_normprio_now);
    if(eores_OK != res)
    {
        return(res);
    }

    return(eores_OK);
}




// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------
static void s_eo_appCanSP_setFalgPort1(void *arg)
{
    EOappCanSP *p = (EOappCanSP *)arg;
    p->newrecframe_port1 = 1;
}


static void s_eo_appCanSP_setFalgPort2(void *arg)
{
    EOappCanSP *p = (EOappCanSP *)arg;
    p->newrecframe_port2 = 1;
}

static eOresult_t s_eo_appCanSP_canPeriphInit(EOappCanSP *p)
{
    hal_can_cfg_t can_cfg_port1, can_cfg_port2;
    eOresult_t res;

    can_cfg_port1.runmode            = hal_can_runmode_isr_1txq1rxq;
    can_cfg_port1.baudrate           = hal_can_baudrate_1mbps; 
    can_cfg_port1.priorx             = hal_int_priority11;
    can_cfg_port1.priotx             = hal_int_priority11;
    can_cfg_port1.callback_on_rx     = s_eo_appCanSP_setFalgPort1;
    can_cfg_port1.arg                = (void*)p;
    
    can_cfg_port2.runmode            = hal_can_runmode_isr_1txq1rxq;
    can_cfg_port2.baudrate           = hal_can_baudrate_1mbps; 
    can_cfg_port2.priorx             = hal_int_priority11;
    can_cfg_port2.priotx             = hal_int_priority11;
    can_cfg_port2.callback_on_rx     = s_eo_appCanSP_setFalgPort2;
    can_cfg_port2.arg                = (void*)p;

    res = (eOresult_t)hal_can_init(hal_can_port1, &can_cfg_port1);

    if(eores_OK != res)
    {
       return(res);
    }

    res = (eOresult_t)hal_can_init(hal_can_port2, &can_cfg_port2);

    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_enable(hal_can_port1);

    if(eores_OK != res)
    {
        return(res);
    }

    res = (eOresult_t)hal_can_enable(hal_can_port2);

    if(eores_OK != res)
    {
        return(res);
    }

    return(eores_OK);
}




//__inline static eOresult_t s_eo_ap_canModule_formAndSendFrame(eo_icubCanProto_msgCommand_t cmd, 
//                                                              void *val_ptr,
//                                                              eo_icubCanProto_msgDestination_t dest,
//                                                              eOcanport_t canPort,
//                                                              eOcanframe_t *frame)
//{
//CURRENTLY NOT USED!!!
//    eOresult_t res;
//
//    res = eo_icubCanProto_FormCanFrame(icubCanProto_ptr, cmd, dest, val_ptr, frame);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//
//    res = (eOresult_t)hal_can_put((hal_can_port_t)canPort, (hal_can_frame_t*)&frame, hal_can_send_normprio_now);
//    if(eores_OK != res)
//    {
//        return(res);
//    }
//
//    return(eores_OK);
//    
//}
// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



