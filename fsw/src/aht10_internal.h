/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *  An example of an internal (private) header file for SAMPLE Lib
 */
#ifndef AHT10_INTERNAL_H
#define AHT10_INTERNAL_H

/* Include all external/public definitions */
#include "aht10.h"

/*************************************************************************
** Macro Definitions
*************************************************************************/

#define AHT10_BUFFER_SIZE 16

/*************************************************************************
** Internal Data Structures
*************************************************************************/
extern char AHT10_Buffer[AHT10_BUFFER_SIZE];

/*************************************************************************
** Function Declarations
*************************************************************************/

/**
 * Library initialization routine/entry point
 */
int32 AHT10_Init(void);

int sensor_aht10_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg);

int read_bytes(int fd, uint16_t i2c_address, uint8_t data_address, uint16_t nr_bytes, uint8_t **buff);
int set_bytes(uint16_t chip_address, uint8_t **val, int numBytes);
int sensor_aht10_get_reg_8(uint8_t register_add, uint8_t **buff);

int readStatusRegister(uint8_t **buff);
uint8_t get_calibration_bit(void);
uint8_t get_busy_bit(void);

#endif
