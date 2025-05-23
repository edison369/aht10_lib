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
 *   Sample CFS library
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "aht10_version.h"
#include "aht10_internal.h"

/* for "strncpy()" */
#include <string.h>

/*************************************************************************
** Private Data Structures
*************************************************************************/
char AHT10_Buffer[AHT10_BUFFER_SIZE];

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Library Initialization Routine                                  */
/* cFE requires that a library have an initialization routine      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 AHT10_Init(void)
{
    /*
     * The specification for strncpy() indicates that it should return
     * the pointer to the destination buffer, so it should be impossible
     * for this to ever fail when linked with a compliant C library.
     */
    if (strncpy(AHT10_Buffer, "SAMPLE DATA", sizeof(AHT10_Buffer) - 1) != AHT10_Buffer)
    {
        return CFE_STATUS_NOT_IMPLEMENTED;
    }

    /* ensure termination */
    AHT10_Buffer[sizeof(AHT10_Buffer) - 1] = 0;

    OS_printf("AHT10 Lib Initialized.%s\n", AHT10_VERSION_STRING);

    return CFE_SUCCESS;
}

// Internal Functions
int read_bytes(int fd, uint16_t i2c_address, uint8_t data_address, uint16_t nr_bytes, uint8_t **buff){
  int rv;
  uint8_t value[nr_bytes];
  i2c_msg msgs[] = {{
    .addr = i2c_address,
    .flags = 0,
    .buf = &data_address,
    .len = 1,
  }, {
    .addr = i2c_address,
    .flags = I2C_M_RD,
    .buf = value,
    .len = nr_bytes,
  }};
  struct i2c_rdwr_ioctl_data payload = {
    .msgs = msgs,
    .nmsgs = sizeof(msgs)/sizeof(msgs[0]),
  };
  uint16_t i;

  rv = ioctl(fd, I2C_RDWR, &payload);
  if (rv < 0) {
    printf("ioctl failed...\n");
  } else {

    free(*buff);
    *buff = malloc(nr_bytes * sizeof(uint8_t));

    for (i = 0; i < nr_bytes; ++i) {
      (*buff)[i] = value[i];
    }
  }

  return rv;
}

int set_bytes(uint16_t chip_address, uint8_t **val, int numBytes){

  int fd;
  int rv;

  if(chip_address == 0){
    chip_address = (uint16_t) AHT10_ADDRESS_X38;
  }

  uint8_t writebuff[numBytes];

  for(int i = 0; i<numBytes; i++){
    writebuff[i] = (*val)[i];
  }

  i2c_msg msgs[] = {{
    .addr = chip_address,
    .flags = 0,
    .buf = writebuff,
    .len = numBytes,
  }};
  struct i2c_rdwr_ioctl_data payload = {
    .msgs = msgs,
    .nmsgs = sizeof(msgs)/sizeof(msgs[0]),
  };

  fd = open(&bus_path[0], O_RDWR);
  if (fd < 0) {
    printf("Couldn't open bus...\n");
    return 1;
  }

  rv = ioctl(fd, I2C_RDWR, &payload);
  if (rv < 0) {
    perror("ioctl failed");
  }
  close(fd);

  return rv;
}

int sensor_aht10_get_reg_8(uint8_t register_add, uint8_t **buff){

  int fd;
  int rv;

  uint8_t *tmp;
  tmp = NULL;

  free(*buff);
  *buff = malloc(1 * sizeof(uint8_t));

  uint16_t nr_bytes = (uint16_t) 1;
  uint16_t chip_address = (uint16_t) AHT10_ADDRESS_X38;
  uint8_t data_address = (uint8_t) register_add;

  fd = open(&bus_path[0], O_RDWR);
  if (fd < 0) {
    printf("Couldn't open bus...\n");
    return 1;
  }

  rv = read_bytes(fd, chip_address, data_address, nr_bytes, &tmp);

  close(fd);

  (*buff)[0] = *tmp;
  free(tmp);

  return rv;
}

int sensor_aht10_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg){
  int err;
  uint8_t *val;

  switch (command) {
    case SENSOR_AHT10_SOFT_RST:

      val = NULL;
      val = malloc(1 * sizeof(uint8_t));

      val[0] = AHTXX_SOFT_RESET_REG;

      err = set_bytes(AHT10_ADDRESS_X38, &val, 1);

      OS_TaskDelay(AHTXX_SOFT_RESET_DELAY);
      break;

    case SENSOR_AHT10_NORMAL_MODE:
      OS_TaskDelay(AHTXX_CMD_DELAY);

      val = NULL;
      val = malloc(3 * sizeof(uint8_t));

      val[0] = AHT1X_INIT_REG;
      val[1] = AHTXX_INIT_CTRL_CAL_ON | AHT1X_INIT_CTRL_NORMAL_MODE;
      val[2] = AHTXX_INIT_CTRL_NOP;

      err = set_bytes(AHT10_ADDRESS_X38, &val, 3);

      break;

    default:
      err = -ENOTTY;
      break;
  }

  return err;
}

int readStatusRegister(uint8_t **buff){
  int err;

  OS_TaskDelay(AHTXX_CMD_DELAY);
  uint8_t *tmp;
  tmp = NULL;

  free(*buff);
  *buff = malloc(sizeof(uint8_t));

  err = sensor_aht10_get_reg_8(AHTXX_STATUS_REG, &tmp);
  (*buff)[0] = *tmp;

  return err;
}

uint8_t get_calibration_bit(void){
  uint8_t *value;
  value = NULL;

  readStatusRegister(&value);

  return ((*value) & AHTXX_STATUS_CTRL_CAL_ON); //0x08=loaded, 0x00=not loaded
}

uint8_t get_busy_bit(void){

  OS_TaskDelay(AHTXX_CMD_DELAY);

  uint8_t *value;
  value = NULL;

  readStatusRegister(&value);

  if(((*value) & AHTXX_STATUS_CTRL_BUSY) == AHTXX_STATUS_CTRL_BUSY){
    return AHTXX_BUSY_ERROR; //0x80=busy, 0x00=measurement completed
  }else{
    return AHTXX_NO_ERROR;
  }
}

// Public Functions
int i2c_dev_register_sensor_aht10(const char *bus_path, const char *dev_path){
  i2c_dev *dev;

  dev = i2c_dev_alloc_and_init(sizeof(*dev), bus_path, AHT10_ADDRESS_X38);
  if (dev == NULL) {
    return -1;
  }

  dev->ioctl = sensor_aht10_ioctl;

  return i2c_dev_register(dev, dev_path);
}

int sensor_aht10_begin(int fd){
  int err;
  
  OS_TaskDelay(100);                  //wait for sensor to initialize
  // Do a soft reset before setting Normal Mode
  ioctl(fd, SENSOR_AHT10_SOFT_RST, NULL);
  ioctl(fd, SENSOR_AHT10_NORMAL_MODE, NULL);
  if(get_calibration_bit() == AHTXX_STATUS_CTRL_CAL_ON){
    err = 0;
  }else{
    err = 1;
  }
  return err;
}

int readMeasurement(uint8_t **buff){
  int fd;
  int rv;

  static const char bus_path[] = "/dev/i2c-1";

  /* send measurement command */
  uint8_t *val;
  val = NULL;
  val = malloc(4 * sizeof(uint8_t));

  val[0] = AHTXX_START_MEASUREMENT_CTRL_NOP;
  val[1] = AHTXX_START_MEASUREMENT_REG;
  val[2] = AHTXX_START_MEASUREMENT_CTRL;
  val[3] = AHTXX_START_MEASUREMENT_CTRL_NOP;

  set_bytes(AHT10_ADDRESS_X38, &val, 4);

  /* check busy bit */
  uint8_t status = get_busy_bit();                                                //update status byte, read status byte & check busy bit

  if      (status == AHTXX_BUSY_ERROR) {OS_TaskDelay(AHTXX_MEASUREMENT_DELAY - AHTXX_CMD_DELAY);}
  else if (status != AHTXX_NO_ERROR)   {return 1;}                                           //no reason to continue, received data smaller than expected

  /* read data from sensor */
  uint16_t nr_bytes = (uint16_t) 6;

  uint8_t *tmp;
  tmp = NULL;

  free(*buff);
  *buff = malloc(nr_bytes * sizeof(uint8_t));

  uint16_t chip_address = (uint16_t) AHT10_ADDRESS_X38;
  uint8_t data_address = (uint8_t) 0x00;  // No register address to read

  fd = open(&bus_path[0], O_RDWR);
  if (fd < 0) {
    printf("Couldn't open bus...\n");
    return 1;
  }

  rv = read_bytes(fd, chip_address, data_address, nr_bytes, &tmp);

  close(fd);

  for (int i = 0; i < nr_bytes; ++i) {
    (*buff)[i] = tmp[i];
  }
  free(tmp);

  /* check busy bit after measurement dalay */
  if (get_busy_bit() != AHTXX_NO_ERROR){
    return 1;
  } //no reason to continue, sensor is busy

  return rv;

}

float sensor_aht10_get_temp(void){

  uint8_t *val;
  int rv;
  uint8_t rawData[5];

  val = NULL;
  val = malloc(6 * sizeof(uint8_t));
  rv = readMeasurement(&val);

  if (rv >= 0){
    for (int i = 0; i < 5; i++) {
      rawData[i] = (val)[i+1];
    }
  }else{
    return -1;
  }

  uint32_t temperature   = rawData[2] & 0x0F;                //20-bit raw temperature data
           temperature <<= 8;
           temperature  |= rawData[3];
           temperature <<= 8;
           temperature  |= rawData[4];

  return ((float)temperature / 0x100000) * 200 - 50;
}
