#include "scd4x.h"


uint8_t generate_crc(const uint8_t* data, uint16_t count) {
	uint16_t current_byte;
	uint8_t crc = CRC8_INIT;
	uint8_t crc_bit;
	/* calculates 8-Bit checksum with given polynomial */
	for (current_byte = 0; current_byte < count; ++current_byte) {
		crc ^= (data[current_byte]);
		for (crc_bit = 8; crc_bit > 0; --crc_bit) {
			if (crc & 0x80)
				crc = (crc << 1) ^ CRC8_POLYNOMIAL;
			else
				crc = (crc << 1); }
	}
	return crc; 
}


const int addr = 0x62;
const int rxbuflen = 16;

int get_feature(SCD40CMD cmd, uint8_t len, uint8_t* rxbuf){
    int ret;
    uint8_t txdata[2];

    txdata[0] = (uint8_t)(cmd >> 8);
    txdata[1] = (uint8_t)(cmd);

    ret = i2c_write_blocking(i2c_default, addr,txdata, 2, true);
    ret = i2c_read_blocking(i2c_default, addr, rxbuf, len,false);
    return ret;
}

int set_feature(SCD40CMD cmd, uint16_t value ){
    int ret;
    uint8_t txdata[5];
    uint8_t crc;
    txdata[0] = (uint8_t)(cmd >> 8);
    txdata[1] = (uint8_t)(cmd);
    txdata[2] = (uint8_t)(value >> 8);
    txdata[3] = (uint8_t)(value);
    crc = generate_crc(txdata + 2, 2);
    txdata[4] = (uint8_t)(crc);
    // printf("%d %d %d\n", txdata[2], txdata[3], txdata[4]);
    ret = i2c_write_blocking(i2c_default, addr, txdata, 5, true);
    return ret;
}
uint8_t send_cmd(SCD40CMD cmd){
    uint8_t txdata[2];
    txdata[0] = (uint8_t)(cmd >> 8);
    txdata[1] = (uint8_t)(cmd);
    return i2c_write_blocking(i2c_default, addr, txdata, 2, true);
}


int get_sensor_altitude(uint16_t* value){
    uint8_t rxbuf[3];
    get_feature(cmd_get_sensor_altitude, 3, rxbuf);
    if(generate_crc(rxbuf, 2) == rxbuf[2]){
        *value = (uint16_t)((rxbuf[0] << 8) + rxbuf[1]);
        return 0;
    }
    return -1;
}

int set_sensor_altitude(uint16_t value){
    return set_feature(cmd_set_sensor_altitude, value);
}

int start_measurements(){
    return send_cmd(cmd_start_periodic_measurement);
}
int stop_measurements(){
    return send_cmd(cmd_stop_periodic_measurement);
}

int get_serial_number(uint32_t* serial){
    uint8_t rxbuf[9];
    get_feature(cmd_get_serial_number, 9, rxbuf);
    *serial = // (rxbuf[0] << 8 + rxbuf[1]) << 32 + 
            (uint32_t)(((rxbuf[3] << 8) + rxbuf[4]) << 16) +
            (uint32_t)((rxbuf[6] << 8) + rxbuf[7]);
    uint8_t crc = 0;
    crc += generate_crc(rxbuf, 2) == rxbuf[2];
    crc += generate_crc(rxbuf + 3, 2) == rxbuf[5];
    crc += generate_crc(rxbuf + 6, 2) == rxbuf[8];
    return (crc == 3) ? 0 : -1;
}
int read_measurements(uint16_t* co2, uint16_t* temp, uint16_t* rh){
    uint8_t rxbuf[9];
    get_feature(cmd_read_measurement, 9, rxbuf);
    *co2 = (uint16_t)((rxbuf[0] << 8) + rxbuf[1]);
    *temp = (uint16_t)((rxbuf[3] << 8) + rxbuf[4]);
    *rh = (uint16_t)((rxbuf[6] << 8) + rxbuf[7]);
    uint8_t crc = 0;
    crc += generate_crc(rxbuf, 2) == rxbuf[2];
    crc += generate_crc(rxbuf + 3, 2) == rxbuf[5];
    crc += generate_crc(rxbuf + 6, 2) == rxbuf[8];
    return (crc == 3) ? 0 : -1;
}

int get_status_ready(){
    uint16_t data_ready = 0;
    int ret = get_feature(cmd_get_data_ready_status, 2, (uint8_t*) (&data_ready));
    return ((data_ready & 0x0600) == 0x600) ;
}
