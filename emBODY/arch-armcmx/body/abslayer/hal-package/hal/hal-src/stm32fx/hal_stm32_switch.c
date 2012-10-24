/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
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


/*  @file       hal_stm32_encoder.c
	@brief      This file implements communication between spi peripherals and encoders.
	@author     valentina.gaggero@iit.it
    @date       10/18/2011
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

#ifdef HAL_USE_SWITCH

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "string.h"
#include "hal_stm32xx_include.h"
#include "hal_stm32_base_hid.h" 
#include "hal_i2c.h" 

#include "hal_sys.h"
#include "hal_stm32_eth_hid.h"

#include "hal_stm32_i2c_hid.h"

#include "hal_brdcfg.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_switch.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_stm32_switch_hid.h" 



// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

const hal_switch_cfg_t hal_switch_cfg_default = 
{ 
    .dummy = 0 
};



// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

//#define HAL_switch_t2index(t)               ((uint8_t)(t))     ho un solo switch

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_switch_supported_is(void);
static void s_hal_switch_initted_set(void);
static hal_boolval_t s_hal_switch_initted_is(void);
static void s_hal_switch_started_set(void);
static hal_boolval_t s_hal_switch_started_is(void);

static hal_result_t s_hal_switch_reg_config(void);
static hal_result_t s_hal_switch_lowlevel_init(const hal_switch_cfg_t *cfg);
static hal_result_t s_hal_switch_lowlevel_start(void);





// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static hal_switch_cfg_t s_hal_switch_cfg            = {.dummy = 0};
static hal_boolval_t s_hal_switch_initted           =  hal_false;
static hal_boolval_t s_hal_switch_started           =  hal_false;


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hal_result_t hal_switch_init(const hal_switch_cfg_t *cfg)
{
    hal_result_t res = hal_res_NOK_generic;


    if(hal_true != s_hal_switch_supported_is())
    {
        return(hal_res_NOK_unsupported);
    }


    if(hal_true == s_hal_switch_initted_is())
    {
        return(hal_res_OK);
    }

    if(NULL == cfg)
    {
        cfg = &hal_switch_cfg_default;
    }

    memcpy(&s_hal_switch_cfg, cfg, sizeof(hal_switch_cfg_t));

    res = s_hal_switch_lowlevel_init(cfg);

    s_hal_switch_initted_set();

    return(res);
}


extern hal_result_t hal_switch_start(hal_eth_phymode_t *phymode)
{
    hal_result_t res = hal_res_NOK_generic;


    if(hal_true != s_hal_switch_supported_is())
    {
        return(hal_res_NOK_unsupported);
    }


    if(hal_false == s_hal_switch_initted_is())
    {
        return(hal_res_NOK_generic);
    }
    

    if(hal_true == s_hal_switch_started_is())
    {
        return(hal_res_OK);
    }    

    
    hal_brdcfg_switch__configure(phymode);

    s_hal_switch_started_set();

    return(hal_res_OK);
 
}


extern hal_bool_t hal_switch_initted_is(void)
{
    return(s_hal_switch_initted_is());
}


extern hal_bool_t hal_switch_started_is(void)
{
    return(s_hal_switch_started_is());
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// ---- isr of the module: begin ----
// empty-section
// ---- isr of the module: end ------

extern uint32_t hal_switch_hid_getsize(const hal_cfg_t *cfg)
{
    return(0);
} 


extern hal_result_t hal_switch_hid_setmem(const hal_cfg_t *cfg, uint32_t *memory)
{

    // remvoed dependency from nzi ram
    memset(&s_hal_switch_cfg, 0, sizeof(s_hal_switch_cfg));
    s_hal_switch_initted                =  hal_false;
    s_hal_switch_started                =  hal_false;

    return(hal_res_OK);
}


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_switch_supported_is(void)
{
    return(hal_brdcfg_switch__supported); 
}

static hal_boolval_t s_hal_switch_initted_is(void)
{
    return(s_hal_switch_initted);
}

static void s_hal_switch_initted_set(void)
{
    s_hal_switch_initted = hal_true;
}


static hal_boolval_t s_hal_switch_started_is(void)
{
    return(s_hal_switch_started);
}


static void s_hal_switch_started_set(void)
{
    s_hal_switch_started = hal_true;
}



static hal_result_t s_hal_switch_lowlevel_init(const hal_switch_cfg_t *cfg)
{
    hal_brdcfg_switch__initialise();
    return(hal_res_OK);    
}


static hal_result_t s_hal_switch_lowlevel_start(void)
{
    return(s_hal_switch_reg_config());
}

static hal_result_t s_hal_switch_reg_config(void)
{
    const uint8_t fd100 = 0x60;
    const uint8_t fd010 = 0x20;
    uint8_t buff_write = 0x60; // FORCE FULL DUPLEX AND 100T
    uint8_t buff_read = 0xFF; 
    volatile uint32_t i = 1;
    hal_i2c_regaddr_t regadr = {.numofbytes = 1, .bytes.one = 0};

    regadr.bytes.one = 0x01;
    hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);
    if((buff_read&0x01))
    {   // already initted. to be initted again must pass through a reset
        return(hal_res_OK);
    }
 
//     // configure mii port at 10mbps. default is 100mbps
//     regadr.bytes.one = 0x06;
//     hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);
//     buff_write = buff_read | 0x10;
//     hal_i2c_write(hal_i2c_port1, 0xBE, regadr, &buff_write, 1);
//     hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);
   
    #warning --> set 100mbps back .............. with 0x60
    // 1. configure  switch's ports 1 and 2 in full duplex and 100mbps
    buff_write = fd010;      
    regadr.bytes.one = 0x1C;
    hal_i2c_write(hal_i2c_port1, 0xBE, regadr, &buff_write, 1);
    hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);
     
    regadr.bytes.one = 0x2C;
    hal_i2c_write(hal_i2c_port1, 0xBE, regadr, &buff_write, 1);   
    hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);

    // 2. start the switch
    buff_write = 0x1;  
    regadr.bytes.one = 0x01;    
    hal_i2c_write(hal_i2c_port1, 0xBE, regadr, &buff_write, 1);
    

    // 3. read back to verify
    regadr.bytes.one = 0x01;
    hal_i2c_read(hal_i2c_port1, 0xBE, regadr, &buff_read, 1);
    if(!(buff_read&0x01))
    {
        hal_base_hid_on_fatalerror(hal_fatalerror_runtimefault, "s_hal_switch_reg_config(): SWITCH not configured");
    }

    return(hal_res_OK);
}


#endif//HAL_USE_SWITCH

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------
