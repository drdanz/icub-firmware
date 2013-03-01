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

// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _HAL_DEVICE_ENCODER_HID_H_
#define _HAL_DEVICE_ENCODER_HID_H_


/* @file       hal_device_encoder_hid.h
    @brief      This header file implements hidden interface to a encoder
    @author     marco.accame@iit.it, valentina.gaggero@iit.it
    @date       02/07/2013
 **/


// - external dependencies --------------------------------------------------------------------------------------------

#include "hal_base.h"

#include "hal_mux.h"
#include "hal_spi.h"


// - declaration of extern public interface ---------------------------------------------------------------------------
 
#include "hal_encoder.h"



// - #define used with hidden struct ----------------------------------------------------------------------------------
// empty-section


// - definition of the hidden struct implementing the object ----------------------------------------------------------

typedef struct
{
    uint8_t                 supported_mask;
    hal_mux_port_t          muxport[hal_encoders_num];              /**< which mux port is used for each encoder */
    hal_mux_sel_t           muxsel[hal_encoders_num];               /**< which mux selection is used of the mux port */
    hal_spi_port_t          spiport[hal_encoders_num];              /**< which spi port is used for each encoder */
} hal_device_encoder_hid_brdcfg_t;


// - declaration of extern hidden variables ---------------------------------------------------------------------------
// empty-section

// - declaration of extern hidden functions ---------------------------------------------------------------------------

extern hal_result_t hal_device_encoder_hid_static_memory_init(void);



#endif  // include guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



