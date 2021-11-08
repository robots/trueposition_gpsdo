#include "platform.h"

#include "gpio.h"
#include "bsp_i2c.h"

#include "i2c.h"

#define I2C_EVENT_MASTER_MODE_SELECT                      ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */
#define I2C_EVENT_MASTER_BYTE_RECEIVED                    ((uint32_t)0x00030040)  /* BUSY, MSL and RXNE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING                 ((uint32_t)0x00070080) /* TRA, BUSY, MSL, TXE flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTED                 ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */
#define I2C_EVENT_MASTER_MODE_ADDRESS10                   ((uint32_t)0x00030008)  /* BUSY, MSL and ADD10 flags */
#define I2C_FLAG_BUSY                   ((uint32_t)0x00020000)

void i2c_init(uint32_t speed)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	gpio_init(i2c_gpio, i2c_gpio_cnt);

	I2C1->CR1 &= ~I2C_CR1_PE;

	uint32_t f = (SystemFrequency_APB1Clk/1000000) & 0x3f;
	I2C1->CR2 = f;
	
	if (speed <= 100000) {
		uint8_t x = SystemFrequency_APB1Clk / (speed << 1);
		if (x < 4) {
			x = 4;
		}
		I2C1->TRISE = f + 1;
		I2C1->CCR = x;
	} else {
		while(1); // not implemented :-D
	}

	I2C1->CR1 = I2C_CR1_ACK; 
	I2C1->OAR1 = 0x15 << 1;
	I2C1->CR1 |= I2C_CR1_PE;
}

static int i2c_test_event(uint32_t ev)
{
  uint32_t e;

  e = (I2C1->SR2 << 16) | I2C1->SR1;

	return e == ev;
}

void i2c_start(uint8_t dir,  uint8_t addr)
{
	while (i2c_test_event(I2C_FLAG_BUSY));

	I2C1->CR1 |= I2C_CR1_START;

	while (!i2c_test_event(I2C_EVENT_MASTER_MODE_SELECT));

	I2C1->DR = (addr << 1) | dir;

	if (dir == I2C_DIR_TX) {
		while (!i2c_test_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	} else {
		while (!i2c_test_event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

void i2c_stop(void)
{
	I2C1->CR1 |= I2C_CR1_STOP;
}

void i2c_write_byte(uint8_t data)
{
	I2C1->DR = data;
	while (!i2c_test_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t i2c_read_byte(void)
{
	while (!i2c_test_event(I2C_EVENT_MASTER_BYTE_RECEIVED));
	return I2C1->DR;
}

