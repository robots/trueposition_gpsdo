#ifndef I2C_h_
#define I2C_h_

enum {
	I2C_DIR_TX,
	I2C_DIR_RX,
};

void i2c_init(uint32_t speed);
void i2c_start(uint8_t transmissionDirection,  uint8_t slaveAddress);
void i2c_stop(void);
void i2c_write_byte(uint8_t data);
uint8_t i2c_read_byte(void);

#endif
