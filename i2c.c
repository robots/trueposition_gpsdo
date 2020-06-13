#include "platform.h"

#include "gpio.h"
#include "bsp_i2c.h"

#include "i2c.h"


void i2c_init(void)
{
	I2C_InitTypeDef i2c;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	GPIO_InitBatch(i2c_gpio, i2c_gpio_cnt);

	i2c.I2C_ClockSpeed = 100000;
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_OwnAddress1 = 0x15;
	i2c.I2C_Ack = I2C_Ack_Enable;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &i2c);

	I2C_Cmd(I2C1, ENABLE);
}

void i2c_start(uint8_t transmissionDirection,  uint8_t slaveAddress)
{
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2C1, slaveAddress<<1, transmissionDirection);

	if(transmissionDirection== I2C_Direction_Transmitter) {
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}

	if(transmissionDirection== I2C_Direction_Receiver) {
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

void i2c_stop(void)
{
	I2C_GenerateSTOP(I2C1, ENABLE);
}

void i2c_write_byte(uint8_t data)
{
	I2C_SendData(I2C1, data);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t i2c_read_byte(void)
{
	uint8_t data;

	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	data = I2C_ReceiveData(I2C1);

	return data;
}

