#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#define CRC8_POLYNOMIAL 0x31 
#define CRC8_INIT 0xFF


const int addr = 0x62;
const int rxbuflen = 16;

uint8_t generate_crc(const uint8_t* data, uint16_t count);

int set_sensor_altitude(uint16_t value);
int get_sensor_altitude(uint16_t* value);
int get_serial_number(uint32_t* value);

int start_measurements();
int stop_measurements();
int read_measurements(uint16_t* co2, uint16_t* temp, uint16_t* rh);
int get_status_ready();

typedef enum SCD40CMD{
  cmd_start_periodic_measurement = 0x21b1,
  cmd_read_measurement = 0xec05,
  cmd_stop_periodic_measurement = 0x3f86,
  cmd_set_temperature_offset = 0x241d,
  cmd_get_temperature_offset = 0x2318,
  cmd_set_sensor_altitude = 0x2427,
  cmd_get_sensor_altitude = 0x2322,
  cmd_set_ambient_pressure =0xe000,
  cmd_perform_forced_recalibration = 0x362f,
  cmd_set_automatic_self_calibration_enabled = 0x2416,
  cmd_get_automatic_self_calibration_enabled = 0x2313,
  cmd_start_low_power_periodic_measurement = 0x21ac,
  cmd_get_data_ready_status = 0xe4b8,
  cmd_persist_settings = 0x3615,
  cmd_get_serial_number = 0x3682,
  cmd_perform_self_test = 0x3639,
  cmd_perform_factory_reset = 0x3632,
  cmd_reinit = 0x3646, 
  cmd_measure_single_shot = 0x219d ,
  cmd_measure_single_shot_rht_only = 0x2196, 
} SCD40CMD;
