/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
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

#include "board.h"

#include "hl_core.h"

#include "stdint.h"

typedef struct
{
    GPIO_TypeDef *  port;
    uint16_t        pin;
} led_data_t;

static const led_data_t s_led_data[8] = 
{
    {GPIOH, GPIO_Pin_2},
    {GPIOG, GPIO_Pin_8},
    {GPIOG, GPIO_Pin_7},
    {GPIOG, GPIO_Pin_6},
    {GPIOI, GPIO_Pin_10},
    {GPIOH, GPIO_Pin_7},
    {GPIOH, GPIO_Pin_6},
    {GPIOH, GPIO_Pin_3}
};


extern void board_led_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // enable clock for GPIOG
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    // configure PG6 to PG8 as outputs push-pull, max speed 50 MHz              
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_6  | GPIO_Pin_7  | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    
    // enable clock for GPIOH
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    // configure PH2-3-6-7 as outputs push-pull, max speed 50 MHz              
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(GPIOH, &GPIO_InitStructure);    
    
    // enable clock for GPIOI
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    // configure PI10 as outputs push-pull, max speed 50 MHz              
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(GPIOI, &GPIO_InitStructure); 
    
}


extern void board_led_on(board_led_t led)
{
    GPIO_WriteBit(s_led_data[led].port, s_led_data[led].pin, Bit_SET);
    //s_led_data[led].port->BSRRL = 1UL << s_led_data[led].pin;
}


extern void board_led_off(board_led_t led) 
{
    GPIO_WriteBit(s_led_data[led].port, s_led_data[led].pin, Bit_RESET);
    //s_led_data[led].port->BSRRH = 1UL << s_led_data[led].pin;
}




