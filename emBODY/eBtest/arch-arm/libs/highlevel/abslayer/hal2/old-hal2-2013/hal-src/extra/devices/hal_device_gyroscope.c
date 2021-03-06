/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
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

/* @file       hal_device_gyroscope.c
	@brief      This file implements internals of the gyroscope module.
	@author     marco.accame@iit.it
    @date       10/24/2012
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

#ifdef HAL_USE_DEVICE_GYROSCOPE

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "string.h"
#include "hal_base_hid.h" 
#include "hal_brdcfg.h"

#include "stdio.h"

#include "hal_utility_bits.h"


 
// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_gyroscope.h"



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_device_gyroscope_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define HAL_device_gyroscope_id2index(t)              ((uint8_t)((t)))



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

extern const hal_gyroscope_cfg_t hal_gyroscope_cfg_default  = 
{ 
    .dummy = 0 
};

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------


typedef struct
{
    hal_gyroscope_cfg_t           config;
} hal_device_gyroscope_internal_item_t;

typedef struct
{
    uint8_t                                 initted;
    hal_device_gyroscope_internal_item_t*   items[hal_gyroscopes_number];   
} hal_device_gyroscope_theinternals_t;


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_device_gyroscope_supported_is(hal_gyroscope_t id);
static void s_hal_device_gyroscope_initted_set(hal_gyroscope_t id);
static hal_boolval_t s_hal_device_gyroscope_initted_is(hal_gyroscope_t id);

static hal_result_t s_hal_device_gyroscope_hw_init(hal_gyroscope_t id, const hal_gyroscope_cfg_t *cfg);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static const variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------


static hal_device_gyroscope_theinternals_t s_hal_device_gyroscope_theinternals =
{
    .initted            = 0,
    .items              = { NULL }   
};


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hal_result_t hal_gyroscope_init(hal_gyroscope_t id, const hal_gyroscope_cfg_t *cfg)
{
//    hal_result_t res = hal_res_NOK_generic; // dont remove ...
//    hal_device_gyroscope_internals_t *info = NULL;

    if(hal_false == s_hal_device_gyroscope_supported_is(id))
    {
        return(hal_res_NOK_generic);
    }
     
    if(NULL == cfg)
    {
        cfg  = &hal_gyroscope_cfg_default;
    }

    if(hal_true == s_hal_device_gyroscope_initted_is(id))
    {
        return(hal_res_OK);
    } 
    
    if(hal_res_OK != s_hal_device_gyroscope_hw_init(id, cfg))
    {
        return(hal_res_NOK_generic);
    }
    
    s_hal_device_gyroscope_initted_set(id);

    return(hal_res_OK);
}


// static int32_t s_hal_device_gyroscope_convert(int32_t v)
// {
//     // the range is +-250 dps. it means that 32k is mapped into 250 -> 32k/250 = 131.072
//     // if i have 1 degree i must read 131.072 ...
//     // if i want to transform in milli-degree, i must read 131.072 / 1000 = 0.131072 = G
//     // to have the measure in milli-degree i must multiply the read for G.
//     // G = 8590 / (64*1024) = 0.131072998046875 .... i multiply and then i shift 16 times

//     uint8_t neg = (v < 0) ? (1) : (0);
//     int32_t r = (0 == neg) ? (8590*v) : (8590*(-v));
//     // now r is positive
//     r >>= 16;
//     r = (0 == neg) ? (r) : (-r);
//     
//     return(r);  
// }
    

extern hal_result_t hal_gyroscope_read(hal_gyroscope_t id, hal_gyroscope_angularrate_t* angrate)
{
    hal_result_t res = hal_res_NOK_generic; 
 
    if(NULL == angrate)
    {
        return(hal_res_NOK_generic);
    }
    
    angrate->xar = 0;
    angrate->yar = 0;
    angrate->zar = 0;
    int32_t xar = 0;
    int32_t yar = 0;
    int32_t zar = 0;
       
    res = hal_brdcfg_device_gyroscope__theconfig.devcfg[HAL_device_gyroscope_id2index(id)].chipif.read(&xar, &yar, &zar);
    
    if(hal_res_OK == res)
    {
 
        angrate->xar = xar; //  factor is about 8.75 or 35/4
        angrate->yar = yar;
        angrate->zar = zar;        
    }
    
    return(res);
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// ---- isr of the module: begin ----
// empty-section
// ---- isr of the module: end ------


extern hal_result_t hal_device_gyroscope_hid_static_memory_init(void)
{
    memset(&s_hal_device_gyroscope_theinternals, 0, sizeof(s_hal_device_gyroscope_theinternals));
    return(hal_res_OK);  
}


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------


static hal_boolval_t s_hal_device_gyroscope_supported_is(hal_gyroscope_t id)
{
    return(hal_utility_bits_byte_bitcheck(hal_brdcfg_device_gyroscope__theconfig.supported_mask, HAL_device_gyroscope_id2index(id)) );
}

static void s_hal_device_gyroscope_initted_set(hal_gyroscope_t id)
{
    hal_utility_bits_byte_bitset(&s_hal_device_gyroscope_theinternals.initted, HAL_device_gyroscope_id2index(id));
}

static hal_boolval_t s_hal_device_gyroscope_initted_is(hal_gyroscope_t id)
{
    return(hal_utility_bits_byte_bitcheck(s_hal_device_gyroscope_theinternals.initted, HAL_device_gyroscope_id2index(id)));
}


static hal_result_t s_hal_device_gyroscope_hw_init(hal_gyroscope_t id, const hal_gyroscope_cfg_t *cfg)
{
    if((NULL != hal_brdcfg_device_gyroscope__theconfig.devcfg[HAL_device_gyroscope_id2index(id)].chipif.init) && (NULL != hal_brdcfg_device_gyroscope__theconfig.devcfg[HAL_device_gyroscope_id2index(id)].chipif.read))
    {
        return(hal_brdcfg_device_gyroscope__theconfig.devcfg[HAL_device_gyroscope_id2index(id)].chipif.init(hal_brdcfg_device_gyroscope__theconfig.devcfg[HAL_device_gyroscope_id2index(id)].chipif.initpar));
    }
    else
    {
        return(hal_res_NOK_generic);
    }
}


#endif//HAL_USE_DEVICE_GYROSCOPE


// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



