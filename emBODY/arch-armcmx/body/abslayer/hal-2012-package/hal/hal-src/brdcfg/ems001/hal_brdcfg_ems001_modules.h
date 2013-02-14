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
  
// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _HAL_BRDCFG_EMS001_MODULES_H_
#define _HAL_BRDCFG_EMS001_MODULES_H_

// - doxy begin -------------------------------------------------------------------------------------------------------

/* @file       hal_brdcfg_ems001_modules.h
    @brief      This header file defines the modules to be compiled in hal
    @author     marco.accame@iit.it
    @date       11/18/2011
 **/


// - modules to be built ----------------------------------------------------------------------------------------------

// -- mpu
#define     HAL_USE_ARCH
#define     HAL_USE_BASE
#define     HAL_USE_CAN
#define     HAL_USE_CRC
#undef     HAL_USE_DMA
#define     HAL_USE_ETH
#define     HAL_USE_FLASH
#define     HAL_USE_GPIO
#define     HAL_USE_I2C
#undef     HAL_USE_SPI
#define     HAL_USE_SYS
#define     HAL_USE_TIMER
#define     HAL_USE_TRACE
#define     HAL_USE_WATCHDOG

// -- devices
#undef  HAL_USE_DEVICE_ACCELEROMETER
#define     HAL_USE_DEVICE_CANTRANSCEIVER
#undef  HAL_USE_DEVICE_DISPLAY  
#define     HAL_USE_DEVICE_EEPROM
#undef     HAL_USE_DEVICE_ENCODER
#define     HAL_USE_DEVICE_ETHTRANSCEIVER
#undef  HAL_USE_DEVICE_GYROSCOPE
#define     HAL_USE_DEVICE_LED
#undef     HAL_USE_DEVICE_MUX
#define     HAL_USE_DEVICE_SWITCH
#undef  HAL_USE_DEVICE_TERMOMETER


// -- utilities
#define     HAL_USE_UTILITY_BITS
#define     HAL_USE_UTILITY_FIFO
#define     HAL_USE_UTILITY_HEAP
#define     HAL_USE_UTILITY_CRC16
#define     HAL_USE_UTILITY_CRC32

// -- chips
#undef   HAL_USE_CHIP_GENERIC_ETHTRANSCEIVER
#undef   HAL_USE_CHIP_ST_L3G4200D
#undef   HAL_USE_CHIP_ST_LIS3DH
#define     HAL_USE_CHIP_MICREL_KS8893
#define     HAL_USE_CHIP_XX_EEPROM


// - exceptions -------------------------------------------------------------------------------------------------------

#if defined(VERY_SMALL )
//    #undef  HAL_USE_CAN
    #undef  HAL_USE_DISPLAY
//    #undef  HAL_USE_ENCODER
//    #undef  HAL_USE_ETH
//    #undef  HAL_USE_EEPROM
//    #undef  HAL_USE_FLASH
//    #undef  HAL_USE_LED
//    #undef  HAL_USE_SWITCH
//    #undef  HAL_USE_TIMER
//    #undef  HAL_USE_WATCHDOG
#endif

#ifdef  HAL_SLIM_MODE
    #undef  HAL_USE_CAN
    #undef  HAL_USE_DISPLAY
    #undef  HAL_USE_ENCODER
    #undef  HAL_USE_ETH
    #undef  HAL_USE_LED
    #undef  HAL_USE_TIMER
    #undef  HAL_USE_WATCHDOG
#endif//HAL_SLIM_MODE

// - cross dependencies -----------------------------------------------------------------------------------------------

#ifdef  HAL_USE_EEPROM
    #define HAL_USE_I2C
#endif//HAL_USE_EEPROM      


#ifdef  HAL_USE_LED
    #define HAL_USE_GPIO
#endif//HAL_USE_LED 

#ifdef  HAL_USE_SWITCH
    #define HAL_USE_I2C
#endif//HAL_USE_SWITCH   


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


