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



/* @file       stm32ee_cfgc
	@brief      This file keeps user-defined constants for the configuration of some i2c EEPROMs using stm32 apis.
	@author     marco.accame@iit.it
    @date       09/14/2012
**/

// - modules to be built: contains the HAL_USE_* macros ---------------------------------------------------------------
//#include "hal_brdcfg_modules.h"

//#ifdef HAL_USE_EEPROM

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "hal_stm32xx_include.h"

#include "stdlib.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
#include "stm32ee_hid.h"

// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define stm32ee_BASICTIMEOUT                ((uint32_t)0x00111000)
#define stm32ee_MAXTRIALS                   (300)


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------

const uint32_t          stm32ee_hid_timeout_flag            = stm32ee_BASICTIMEOUT;
const uint32_t          stm32ee_hid_timeout_long            = 10 * stm32ee_BASICTIMEOUT;
const uint32_t          stm32ee_hid_ackaddress_maxtrials    = stm32ee_MAXTRIALS;


#if     defined(USE_STM32F1)


// -- dma


const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_tx_enable  =
{
    .NVIC_IRQChannel                    = DMA1_Channel6_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = ENABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_rx_enable  =
{
    .NVIC_IRQChannel                    = DMA1_Channel7_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = ENABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_tx_disable =
{
    .NVIC_IRQChannel                    = DMA1_Channel6_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = DISABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_rx_disable =
{
    .NVIC_IRQChannel                    = DMA1_Channel7_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = DISABLE
};

const uint32_t          stm32ee_hid_dma_clock       = RCC_AHBPeriph_DMA1; 

//DMA_TypeDef *           stm32ee_hid_dma             = DMA1;
DMA_Channel_TypeDef * const  stm32ee_hid_dma_stream_tx   = DMA1_Channel6;
DMA_Channel_TypeDef * const  stm32ee_hid_dma_stream_rx   = DMA1_Channel7;

#define stm32ee_hid_dma_I2C_DMA_TX_IRQHandler    DMA1_Channel6_IRQHandler
#define stm32ee_hid_dma_I2C_DMA_RX_IRQHandler    DMA1_Channel7_IRQHandler   

const DMA_InitTypeDef         stm32ee_hid_dma_cfg_init    =
{
    .DMA_PeripheralBaseAddr = ((uint32_t)0x40005410),
    .DMA_MemoryBaseAddr     = (uint32_t)0,              /* This parameter will be configured durig communication */
    .DMA_DIR                = DMA_DIR_PeripheralDST,    /* This parameter will be configured durig communication */
    .DMA_BufferSize         = 0xFFFF,                   /* This parameter will be configured durig communication */
    .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc          = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte,
    .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
    .DMA_Mode               = DMA_Mode_Normal,
    .DMA_Priority           = DMA_Priority_VeryHigh,
    .DMA_M2M                = DMA_M2M_Disable    
};

const uint32_t          stm32ee_hid_dma_flags_tx                = 0;
const uint32_t          stm32ee_hid_dma_flags_rx                = 0;

const uint32_t          stm32ee_hid_dma_flags_tx_completed      = (DMA1_IT_TC6);
const uint32_t          stm32ee_hid_dma_flags_rx_completed      = (DMA1_IT_TC7);

const uint32_t          stm32ee_hid_dma_flags_tx_all            = (DMA1_IT_GL6);
const uint32_t          stm32ee_hid_dma_flags_rx_all            = (DMA1_IT_GL7);



#elif   defined(USE_STM32F4)

// -- dma

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_tx_enable  =
{
    .NVIC_IRQChannel                    = DMA1_Stream6_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = ENABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_rx_enable  =
{
    .NVIC_IRQChannel                    = DMA1_Stream0_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = ENABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_tx_disable =
{
    .NVIC_IRQChannel                    = DMA1_Stream6_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = DISABLE
};

const NVIC_InitTypeDef  stm32ee_hid_dma_nvic_rx_disable =
{
    .NVIC_IRQChannel                    = DMA1_Stream0_IRQn,
    .NVIC_IRQChannelPreemptionPriority  = 0,
    .NVIC_IRQChannelSubPriority         = 0,
    .NVIC_IRQChannelCmd                 = DISABLE
};

const uint32_t          stm32ee_hid_dma_clock       = RCC_AHB1Periph_DMA1; 

//DMA_TypeDef *           stm32ee_hid_dma             = DMA1;
DMA_Stream_TypeDef * const   stm32ee_hid_dma_stream_tx   = DMA1_Stream6;
DMA_Stream_TypeDef * const   stm32ee_hid_dma_stream_rx   = DMA1_Stream0;

#define stm32ee_hid_dma_I2C_DMA_TX_IRQHandler    DMA1_Stream6_IRQHandler
#define stm32ee_hid_dma_I2C_DMA_RX_IRQHandler    DMA1_Stream0_IRQHandler   

const DMA_InitTypeDef         stm32ee_hid_dma_cfg_init    =
{
    .DMA_Channel            = DMA_Channel_1,
    .DMA_PeripheralBaseAddr = ((uint32_t)0x40005410),
    .DMA_Memory0BaseAddr    = (uint32_t)0,                  // to be configured during communication 
    .DMA_DIR                = DMA_DIR_MemoryToPeripheral,   // to be configured during communication 
    .DMA_BufferSize         = 0xFFFF,                       // to be configured during communication 
    .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
    .DMA_MemoryInc          = DMA_MemoryInc_Enable,
    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
    .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
    .DMA_Mode               = DMA_Mode_Normal,
    .DMA_Priority           = DMA_Priority_VeryHigh,
    .DMA_FIFOMode           = DMA_FIFOMode_Enable,
    .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
    .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
    .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single
};

const uint32_t          stm32ee_hid_dma_flags_tx                = (DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6);
const uint32_t          stm32ee_hid_dma_flags_rx                = (DMA_FLAG_FEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_HTIF0 | DMA_FLAG_TCIF0);

const uint32_t          stm32ee_hid_dma_flags_tx_completed      = (DMA_FLAG_TCIF6);
const uint32_t          stm32ee_hid_dma_flags_rx_completed      = (DMA_FLAG_TCIF0);

const uint32_t          stm32ee_hid_dma_flags_tx_all            = (0);
const uint32_t          stm32ee_hid_dma_flags_rx_all            = (0);

#endif

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------


//#endif//HAL_USE_EEPROM

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------


