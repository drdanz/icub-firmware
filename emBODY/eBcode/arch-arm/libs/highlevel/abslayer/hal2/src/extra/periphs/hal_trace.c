/*
 * Copyright (C) 2012 iCub Facility - Istituto Italiano di Tecnologia
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

/* @file       hal_trace.c
	@brief      This file implements internal implementation of the hal trace module.
	@author     marco.accame@iit.it
    @date       09/12/2011
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"
// - middleware interface: contains hl, stm32 etc. --------------------------------------------------------------------
#include "hal_middleware_interface.h"

#ifdef HAL_USE_TRACE

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "hl_sys.h" 

 
// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_trace.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_trace_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

extern volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;

const hal_trace_cfg_t hal_trace_cfg_default =
{
    .dummy  = 0
};


// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

//typedef struct
//{
//    uint8_t         nothing;     
//} hal_trace_theinternals_t;

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_bool_t s_hal_trace_supported_is(void);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static const variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

// static hal_trace_theinternals_t s_hal_trace_theinternals =
// {
//     .nothing = 0
// };

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------


extern hal_result_t hal_trace_init(const hal_trace_cfg_t* cfg)
{
    
    if(hal_true != s_hal_trace_supported_is())
    {
        return(hal_res_NOK_unsupported);
    }
          
//    if(NULL == cfg)
//    {
//        cfg = &hal_trace_cfg_default;
//    }
  
    ITM_RxBuffer = ITM_RXBUFFER_EMPTY;
    
    return(hal_res_OK);
}



extern int hal_trace_getchar(void) 
{
#if     !defined(HAL_BEH_REMOVE_RUNTIME_VALIDITY_CHECK)     
    if(hal_true != s_hal_trace_supported_is())
    {
        return(-1);
    }
#endif

    while(ITM_CheckChar() != 1);
    return(ITM_ReceiveChar());
}    


extern int hal_trace_putchar(int ch) 
{
#if     !defined(HAL_BEH_REMOVE_RUNTIME_VALIDITY_CHECK) 
    if(hal_true != s_hal_trace_supported_is())
    {
        return(-1);
    }
#endif    
    return(ITM_SendChar(ch));    
}

extern int hal_trace_puts(const char * str) 
{
#if     !defined(HAL_BEH_REMOVE_RUNTIME_VALIDITY_CHECK)     
    if(hal_true != s_hal_trace_supported_is())
    {
        return(0);
    }
#endif
    
    return(hl_sys_itm_puts(str));   
}




// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------


// ---- isr of the module: begin ----
// empty-section
// ---- isr of the module: end ------



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hal_bool_t s_hal_trace_supported_is(void)
{
    return(hal_trace__theboardconfig.supported); 
}



#endif//HAL_USE_TRACE

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



