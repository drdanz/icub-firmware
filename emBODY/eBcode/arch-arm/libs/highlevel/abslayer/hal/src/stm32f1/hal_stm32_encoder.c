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

/* @file     hal_stm32_encoder.c
	@brief      This file implements communication between spi peripherals and encoders.
	@author     marco.accame@iit.it
    @date       09/09/2011
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
#include "hal_brdcfg_modules.h"

#ifdef HAL_USE_ENCODER

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "string.h"
#include "stm32f1.h"

#include "hal_spi4encoder.h"
#include "hal_stm32_spi4encoder_hid.h"

#include "hal_brdcfg.h"
#include "hal_stm32_base_hid.h" 
#include "hal_stm32_sys_hid.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "hal_encoder.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "hal_stm32_encoder_hid.h" 


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

const hal_encoder_cfg_t hal_encoder_cfg_default = 
{
    .priority                   = hal_int_priority15, 
    .bitrate                    = hal_encoder_bitrate_500kbps,
    .callback_on_rx             = NULL, 
    .arg                        = NULL
};



// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define HAL_encoder_t2index(t)               ((uint8_t)(t))

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_encoder_supported_is(hal_encoder_t encoder);
static void s_hal_encoder_initted_set(hal_encoder_t encoder);
static hal_boolval_t s_hal_encoder_initted_is(hal_encoder_t encoder);

static void s_hal_encoder_on_rx_spi(void* p);

//static void s_hal_encoder_on_rx_spi1(void* p);
//static void s_hal_encoder_on_rx_spi2(void* p);
//static void s_hal_encoder_on_rx_spi3(void* p);

static hal_spi_port_t s_hal_encoder_from_encoder_to_spiport(hal_encoder_t enc);
static hal_spi_mux_t s_hal_encoder_from_encoder_to_spiportmux(hal_encoder_t enc);


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static hal_encoder_cfg_t s_hal_encoder_cfgs[hal_encoders_num] = {hal_int_priorityNONE, hal_encoder_bitrate_500kbps, NULL, NULL};

static hal_boolval_t s_hal_encoder_initted[hal_encoders_num] = { hal_false };


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

extern hal_result_t hal_encoder_init(hal_encoder_t enc, const hal_encoder_cfg_t *cfg)
{
    hal_result_t res = hal_res_NOK_generic;
    hal_spi_port_t spix = s_hal_encoder_from_encoder_to_spiport(enc);
    hal_spi4encoder_cfg_t spi4enc_cfg;

    if(hal_true != s_hal_encoder_supported_is(enc))
    {
        return(hal_res_NOK_unsupported);
    }

    if(NULL == cfg)
    {
        cfg = &hal_encoder_cfg_default;
    }
    
    if((hal_encoder_bitrate_500kbps != cfg->bitrate) && (hal_encoder_bitrate_1000kbps != cfg->bitrate))
    {
        return(hal_res_NOK_unsupported);
    }

    memcpy(&s_hal_encoder_cfgs[HAL_encoder_t2index(enc)], cfg, sizeof(hal_encoder_cfg_t));


    // the mapping of the encoder onto the spi is fixed and is defined in here.

    switch(enc)
    {
        case hal_encoder1:
        case hal_encoder2:
        case hal_encoder3:
        {
            spi4enc_cfg.baudrate        = (hal_encoder_bitrate_500kbps == cfg->bitrate) ? hal_spi_baudrate500kbps : hal_spi_baudrate1000kbps;
            spi4enc_cfg.priority        = cfg->priority;
            spi4enc_cfg.callback_on_rx  = s_hal_encoder_on_rx_spi;
            spi4enc_cfg.arg             = NULL;//(hal_spi4encoder_encoderReadData_t*) &(hal_SPI4ENCODER_GET(hal_spi_port1)->enc_data);
        } break;

        case hal_encoder4:
        case hal_encoder5:
        case hal_encoder6:
        {
            spi4enc_cfg.baudrate        = (hal_encoder_bitrate_500kbps == cfg->bitrate) ? hal_spi_baudrate500kbps : hal_spi_baudrate1000kbps;
            spi4enc_cfg.priority        = cfg->priority;
            spi4enc_cfg.callback_on_rx  = s_hal_encoder_on_rx_spi;
            spi4enc_cfg.arg             = NULL;//&s_hal_encoder_ids[HAL_encoder_t2index(enc)];
        } break;

        case hal_encoder7:
        case hal_encoder8:
        case hal_encoder9:
        {
            spi4enc_cfg.baudrate        = (hal_encoder_bitrate_500kbps == cfg->bitrate) ? hal_spi_baudrate500kbps : hal_spi_baudrate1000kbps;
            spi4enc_cfg.priority        = cfg->priority;
            spi4enc_cfg.callback_on_rx  = s_hal_encoder_on_rx_spi;
            spi4enc_cfg.arg             = NULL;
        } break;

        default:
        {
            spi4enc_cfg.baudrate        = (hal_encoder_bitrate_500kbps == cfg->bitrate) ? hal_spi_baudrate500kbps : hal_spi_baudrate1000kbps;
            spi4enc_cfg.priority        = cfg->priority;
            spi4enc_cfg.callback_on_rx  = NULL;
            spi4enc_cfg.arg             = NULL;
        } break;
    }

    
    res = hal_spi4encoder_init(spix, &spi4enc_cfg);

    if(hal_res_OK == res)
    {
        s_hal_encoder_initted_set(enc);
    }


    return(res);
}



extern hal_result_t hal_encoder_read_block(hal_encoder_t enc, uint32_t *result)
{
    hal_spi_port_t spix = s_hal_encoder_from_encoder_to_spiport(enc);
    hal_spi_mux_t encoder = s_hal_encoder_from_encoder_to_spiportmux(enc);

    uint8_t value[3] = {0, 0, 0};
    uint16_t dummytx = 0;
    uint8_t i;
    
    SPI_TypeDef * spi_per = hal_SPI4ENCODER_PERIPH_GET(spix);

    if(hal_true != s_hal_encoder_initted_is(enc))
    {
        return(hal_res_NOK_generic);
    }

    if(NULL == result)
    {
        return(hal_res_NOK_generic);
    }


	hal_base_hid_osal_scheduling_suspend(); //suspend scheduling

    if(1 == hal_SPI4ENCODER_ENCODER_IS_BUSY_SET(spix))
    {
        hal_base_hid_osal_scheduling_restart(); //resume scheduling
        return(hal_res_NOK_generic);
    }

    hal_SPI4ENCODER_ENCODER_BUSY_SET(spix);	//saved encoder id in spix interface datas

    hal_base_hid_osal_scheduling_restart(); //resume scheduling


    
    hal_brdcfg_spi4encoder__encoder_enable(spix, encoder);
    hal_SPI4ENCODER_ENA(spix);	 //enable peripheral
    
    for(i=0; i<3; i++)
    {
        while (SPI_I2S_GetFlagStatus(spi_per, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(spi_per, dummytx);
    	
        while (SPI_I2S_GetFlagStatus(spi_per, SPI_I2S_FLAG_RXNE) == RESET);
        value[i] = SPI_I2S_ReceiveData(spi_per);
    }
    
    
    hal_spi4encoder_hid_disa(spix);//disable peripheral
    hal_brdcfg_spi4encoder__encoder_disable(spix, encoder);
    
    //formatting result
   *result = ( (value[0] & 0x7F)  << 16 ) | (value[1] << 8) | (value[2] & 0xE0);
   *result = (*result) >> 5;
    

	hal_base_hid_osal_scheduling_suspend(); //suspend scheduling

    hal_SPI4ENCODER_ENCODER_BUSY_RESET(spix);

    hal_base_hid_osal_scheduling_restart(); //resume scheduling
    
    return(hal_res_OK);
}




extern hal_result_t hal_encoder_read_start(hal_encoder_t enc)
{
    hal_spi_port_t spix = s_hal_encoder_from_encoder_to_spiport(enc);             
    hal_spi_mux_t encoder = s_hal_encoder_from_encoder_to_spiportmux(enc);    // enc 1, 4, 7 has mux 0, enc 2, 5, 8 has mux 1, enc 3, 6, 9 hax mux 2

	uint16_t dummytx = 0xABCD;
    SPI_TypeDef * spi_per = hal_SPI4ENCODER_PERIPH_GET(spix);


    if(hal_true != s_hal_encoder_initted_is(enc))
    {
        return(hal_res_NOK_generic);
    }

    // verify if we can start reading the encoder on spix or if someone else already has started the reading
	
	hal_base_hid_osal_scheduling_suspend(); //suspend scheduling

    if(1 == hal_SPI4ENCODER_ENCODER_IS_BUSY_SET(spix))
    {
        hal_base_hid_osal_scheduling_restart(); //resume scheduling
        return(hal_res_NOK_generic);
    }

    hal_SPI4ENCODER_ENCODER_BUSY_SET(spix);	//saved encoder id in spix interface datas

    hal_base_hid_osal_scheduling_restart(); //resume scheduling

    // ok, we can start the reading process.

	hal_SPI4ENCODER_ENCODER_ID_SET(spix, encoder);	//saved encoder id in spix interface datas
	hal_SPI4ENCODER_IT_RX_ENA(spix);//enable interrupt rx

	hal_brdcfg_spi4encoder__encoder_enable(spix, encoder);
	hal_SPI4ENCODER_ENA(spix);	   //enable peripheral
	
	//in order to read data I have to write something. The channel is configured in full duplex mode.
	while (SPI_I2S_GetFlagStatus(spi_per, SPI_I2S_FLAG_TXE) == RESET);
	
	SPI_I2S_SendData(spi_per, dummytx);

    // ok, the isr shall read three times one byte and then issue a callback to me.
    return(hal_res_OK);

}



extern hal_result_t hal_encoder_get_value(hal_encoder_t enc, uint32_t *result )
{
    hal_spi_port_t spix = s_hal_encoder_from_encoder_to_spiport(enc);             
    hal_spi_mux_t encoder = s_hal_encoder_from_encoder_to_spiportmux(enc);    // enc 1, 4, 7 has mux 0, enc 2, 5, 8 has mux 1, enc 3, 6, 9 hax mux 2

    if(hal_true != s_hal_encoder_initted_is(enc))
    {
        return(hal_res_NOK_generic);
    }

 	if(NULL == result)
	{
		return(hal_res_NOK_generic);
    }

//  we allow a read of the value of the encoder also if the isr is astive .... be careful
//	while(hal_SPI4ENCODER_IS_IT_RX_ENA(spix)) ;


	*result = hal_SPI4ENCODER_ENCDATA_GET(spix).encoder_result[encoder];
    return(hal_res_OK);
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// ---- isr of the module: begin ----
// empty-section
// ---- isr of the module: end ------

extern uint32_t hal_encoder_hid_getsize(const hal_cfg_t *cfg)
{
    return(hal_spi4encoder_hid_getsize(cfg));
} 


extern hal_result_t hal_encoder_hid_setmem(const hal_cfg_t *cfg, uint32_t *memory)
{
    // removed dependancy from NZI ram
    memset(s_hal_encoder_cfgs, 0, sizeof(s_hal_encoder_cfgs));
    memset(s_hal_encoder_initted, hal_false, sizeof(s_hal_encoder_initted));

    return(hal_spi4encoder_hid_setmem(cfg, memory));
}

extern hal_encoder_t hal_encoder_hid_from_spiportmux_to_encoder(hal_spi_port_t spiport, hal_spi_mux_t spimux)
{
    uint8_t v = 3*((uint8_t)spiport) + (uint8_t)spimux;
   
    return((hal_encoder_t)v);
}


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------

static hal_boolval_t s_hal_encoder_supported_is(hal_encoder_t enc)
{
    return(hal_base_hid_word_bitcheck(hal_brdcfg_encoder__supported_mask, HAL_encoder_t2index(enc)) );
}

static void s_hal_encoder_initted_set(hal_encoder_t enc)
{
    s_hal_encoder_initted[HAL_encoder_t2index(enc)] = hal_true;
}

static hal_boolval_t s_hal_encoder_initted_is(hal_encoder_t enc)
{
    return(s_hal_encoder_initted[HAL_encoder_t2index(enc)]);
}

static void s_hal_encoder_on_rx_spi(void* p)
{
    hal_encoder_t enc = (hal_encoder_t)((uint32_t)p);  // ok, i know that there is a conversion to a smaller integer but i dont care
    uint8_t index = HAL_encoder_t2index(enc);

    if(NULL != s_hal_encoder_cfgs[index].callback_on_rx)
    {
        s_hal_encoder_cfgs[index].callback_on_rx(s_hal_encoder_cfgs[index].arg);   
    }
}


static hal_spi_port_t s_hal_encoder_from_encoder_to_spiport(hal_encoder_t enc)
{
    uint8_t v = (uint8_t)enc;

    v /= hal_spi_muxes_number;            // max 3 mux on each spi
    
    return((hal_spi_port_t)v);

}


static hal_spi_mux_t s_hal_encoder_from_encoder_to_spiportmux(hal_encoder_t enc)
{
    uint8_t v = (uint8_t)enc;

    v %= hal_spi_muxes_number;             // max 3 mux on each spi
    
    return((hal_spi_mux_t)v);

}
 
#endif//HAL_USE_ENCODER

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------
