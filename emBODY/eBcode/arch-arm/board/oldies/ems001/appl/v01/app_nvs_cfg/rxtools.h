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

// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _RXTOOLS_H_
#define _RXTOOLS_H_

#include "stdint.h"

typedef struct
{   // time is in milli-sec
    uint32_t    receivertime;
//    uint64_t    transmittime;
    uint32_t    receiverprog;    
} rxtools_rec_status_t;

typedef struct
{   // time is in milli-sec
    uint32_t    txtime; // in here time is the one provided by received rop thus we must divide that by 1000
    uint32_t    txprog; 
} rxtools_tx_inrop_t;

typedef struct
{   // time is in milli-sec
    uint8_t     flags;
    int32_t     deltaprognumber;
    uint32_t    deltarxtime; 
} rxtools_results_t;

enum { rxtools_flag_error_prognum = 0x01, rxtools_flag_error_rxtime = 0x02, rxtools_flag_error_txtime = 0x04 }; 

//  0 is ok
// -1 is .. something wrong
// -2 is ... invalid params
// flag 0x01 is missing sequence number
// flag 0x02 is to delay in reception
// time is in milli-sec
extern int32_t rxtools_verify_reception(rxtools_rec_status_t* status, rxtools_tx_inrop_t* txinrop, uint32_t maxtimegap, rxtools_results_t* results);


#endif  // include-guard


