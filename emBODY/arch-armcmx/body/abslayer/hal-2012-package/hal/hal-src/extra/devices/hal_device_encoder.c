/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero, Marco Accame
 * email:   valentina.gaggero@iit.it, marco.accame@iit.it
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


/* @file       hal_device_encoder.c
	@brief      This file implements internal implementation of the hal encoder module.
	@author     marco.accame@iit.it, valentina.gaggero@iit.it
    @date       02/07/2013
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

#ifdef HAL_USE_DEVICE_ENCODER

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "string.h"
#include "hal_gpio.h"
#include "hal_brdcfg.h"
#include "hal_utility_bits.h" 



 
// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_encoder.h"



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_device_encoder_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define HAL_device_encoder_encoder2index(t)              ((uint8_t)((t)))



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

extern const hal_encoder_cfg_t hal_encoder_cfg_default = { .priority = hal_int_priority15, .callback_on_rx = NULL, .arg = NULL };

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

typedef struct
{
    hal_encoder_cfg_t       config;
    hal_mux_port_t          muxport;
    hal_mux_sel_t           muxsel;
    hal_spi_port_t          spiport;
    hal_encoder_position_t  position;
    uint8_t                 rxframe[4];
} hal_encoder_internals_t;


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_device_encoder_supported_is(hal_encoder_t encoder);
static void s_hal_device_encoder_initted_set(hal_encoder_t encoder);
static hal_boolval_t s_hal_device_encoder_initted_is(hal_encoder_t encoder);


static void s_hal_encoder_onreceiv(void* p);

static hal_encoder_position_t s_hal_encoder_frame2position(uint8_t* frame);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static const variables
// --------------------------------------------------------------------------------------------------------------------

static const hal_spi_cfg_t s_hal_device_encoder_spicfg_master =
{
    .ownership                  = hal_spi_ownership_master,
    .direction                  = hal_spi_dir_rxonly,
    .activity                   = hal_spi_act_framebased,
    .speed                      = hal_spi_speed_0562kbps, 
    .sizeofframe                = 3,
    .onframetransm              = NULL,
    .onframereceiv              = s_hal_encoder_onreceiv,
    .capacityoftxfifoofframes   = 0,
    .capacityofrxfifoofframes   = 1,
    .dummytxvalue               = 0
};   


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static uint16_t s_hal_device_encoder_initted = 0;

static hal_encoder_internals_t* s_hal_device_encoder_internals[hal_encoders_num] = { NULL };

// static hal_encoder_internals_t s_hal_device_encoder_internals[hal_encoders_num] = 
// {
//     {
//         .config     = { .priority = hal_int_priorityNONE, .callback_on_rx = NULL, .arg = NULL }, 
//         .muxport    = hal_mux_port1, 
//         .muxsel     = hal_mux_selNONE, 
//         .spiport    = hal_spi_port1, 
//         .position   = 0, 
//         .rxframe    = {0}
//     }
// };



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hal_result_t hal_encoder_init(hal_encoder_t encoder, const hal_encoder_cfg_t *cfg)
{
    //hal_result_t res = hal_res_NOK_generic;
    hal_encoder_internals_t* intitem = s_hal_device_encoder_internals[HAL_device_encoder_encoder2index(encoder)];

    if(hal_false == s_hal_device_encoder_supported_is(encoder))
    {
        return(hal_res_NOK_generic);
    }
    
    if(NULL == cfg)
    {
        cfg = &hal_encoder_cfg_default;
    }
     
        
    // configure the required mux port and spi port.
    
    // if it does not have ram yet, then attempt to allocate it.
    if(NULL == intitem)
    {
        intitem = s_hal_device_encoder_internals[HAL_device_encoder_encoder2index(encoder)] = hal_heap_new(sizeof(hal_encoder_internals_t));
        // minimal initialisation of the internal item
        // nothing to init.      
    }       
    
    memcpy(&intitem->config, cfg, sizeof(hal_encoder_cfg_t));   
    intitem->muxport   = hal_brdcfg_device_encoder__theconfig.muxport[HAL_device_encoder_encoder2index(encoder)];
    intitem->muxsel    = hal_brdcfg_device_encoder__theconfig.muxsel[HAL_device_encoder_encoder2index(encoder)];
    intitem->spiport   = hal_brdcfg_device_encoder__theconfig.spiport[HAL_device_encoder_encoder2index(encoder)];
    intitem->position  = 0;
    

    
     
    #warning HAL-WIP --> configure mux and spi. obviously if already initted we dont init the spi and the port anymore.
    
    hal_mux_init(intitem->muxport, NULL);
    
    hal_spi_init(intitem->spiport, &s_hal_device_encoder_spicfg_master);
    
 
    s_hal_device_encoder_initted_set(encoder);
    return(hal_res_OK);
}


extern hal_result_t hal_encoder_start(hal_encoder_t encoder)
{  
    hal_encoder_internals_t* intitem = s_hal_device_encoder_internals[HAL_device_encoder_encoder2index(encoder)];   
    
    if(hal_false == s_hal_device_encoder_initted_is(encoder))
    {
        return(hal_res_NOK_generic);
    }

    // do something 
    
       
    hal_mux_enable(intitem->muxport, intitem->muxsel);
    
    hal_spi_on_framereceiv_set(intitem->spiport, s_hal_encoder_onreceiv, (void*)encoder);
    
    hal_spi_start(intitem->spiport, 1); // 1 solo frame ...
    
    // quando il frame sara' stato ricevuto allora si chiamera' la callback che prima scrive in ram. poi ...
    
    return(hal_res_OK);
}



extern hal_result_t hal_encoder_read(hal_encoder_t encoder, hal_encoder_position_t* value)
{
    hal_encoder_internals_t* intitem = s_hal_device_encoder_internals[HAL_device_encoder_encoder2index(encoder)];
    
    if(hal_false == s_hal_device_encoder_initted_is(encoder))
    {
        return(hal_res_NOK_generic);
    }

    // do something 


    if(NULL != value)
    {
        *value = intitem->position; 
        return(hal_res_OK);
    }
    
    return(hal_res_NOK_generic);
   
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// ---- isr of the module: begin ----
// empty-section
// ---- isr of the module: end ------


extern hal_result_t hal_device_encoder_hid_static_memory_init(void)
{
    s_hal_device_encoder_initted = 0;
    memset(&hal_encoder_internals_t, 0, sizeof(hal_encoder_internals_t));
    return(hal_res_OK);  
}

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_device_encoder_supported_is(hal_encoder_t encoder)
{
    return(hal_utility_bits_byte_bitcheck(hal_brdcfg_device_encoder__theconfig.supported_mask, HAL_device_encoder_encoder2index(encoder)) );
}


static void s_hal_device_encoder_initted_set(hal_encoder_t encoder)
{
    hal_utility_bits_halfword_bitset(&s_hal_device_encoder_initted, HAL_device_encoder_encoder2index(encoder));
}


static hal_boolval_t s_hal_device_encoder_initted_is(hal_encoder_t encoder)
{
    return(hal_utility_bits_halfword_bitcheck(s_hal_device_encoder_initted, HAL_device_encoder_encoder2index(encoder)));
}



static void s_hal_encoder_onreceiv(void* p)
{
    int32_t tmp = (int32_t)p;                   // tmp is used just to remove a warning about conversione from pointer to smaller integer
    hal_encoder_t encoder = (hal_encoder_t)tmp;
    hal_encoder_internals_t* intitem = s_hal_device_encoder_internals[HAL_device_encoder_encoder2index(encoder)];
    
    hal_spi_stop(intitem->spiport);
    hal_mux_disable(intitem->muxport);
    
    // ok ... now i get the frame of three bytes.
    
    hal_spi_get(intitem->spiport, intitem->rxframe, NULL);
    
    intitem->position = s_hal_encoder_frame2position(intitem->rxframe);
    
    // ok. now i call the callbcak on execution of encoder
    
    if(NULL != intitem->config.callback_on_rx)
    {
        intitem->config.callback_on_rx(intitem->config.arg);
    }
    
}


static hal_encoder_position_t s_hal_encoder_frame2position(uint8_t* frame)
{
    uint32_t pos = 0;
    
    pos = frame[0] | (frame[1] << 8) | (frame[2] << 16);
    return(pos);
}


#endif//HAL_USE_DEVICE_ENCODER

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------


