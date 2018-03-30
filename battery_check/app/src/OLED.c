#include <zephyr.h>
#include <gpio.h>
#include <spi.h>

#include <string.h>

#include "OLED.h"

#define OLED_SPI_INIT_FAIL		-0x01
#define OLED_SPI_CONFIG_FAIL	-0x02
#define OLED_SPI_SELECT_FAIL	-0x03
#define OLED_SPI_WRITE_FAIL		-0x04

#define OLED_GPIO_INIT_FAIL		-0x11
#define OLED_GPIO_CONFIG_FAIL	-0x12

#define OLED_DC_PIN 16
#define OLED_RES_PIN 20

static struct device *GPIODevice;
static struct device *SPIDevice;
struct spi_config SPIConfig = {
	.config = SPI_MODE_CPOL | SPI_MODE_CPHA |  SPI_WORD(8),
	.max_sys_freq = 2,
};

int InitOLED()
{
	int err;
	SPIDevice = device_get_binding("SPI_1");
	if (!SPIDevice)
	{
		printk("spi get fail\n");
		return OLED_SPI_INIT_FAIL;
	}

	err = spi_configure(SPIDevice, &SPIConfig);
	if (err)
	{
		return OLED_SPI_CONFIG_FAIL;
	}

	err = spi_slave_select(SPIDevice, 1);
	if (err)
	{
		return OLED_SPI_SELECT_FAIL;
	}


	GPIODevice = device_get_binding("GPIO_0");
	if (!GPIODevice)
	{
		return OLED_GPIO_INIT_FAIL;
	}

	// configure
	if (gpio_pin_configure(GPIODevice, OLED_DC_PIN, (GPIO_DIR_OUT)) ||
		gpio_pin_configure(GPIODevice, OLED_RES_PIN, (GPIO_DIR_OUT)))
	{
		return OLED_GPIO_CONFIG_FAIL;
	}

	// power on
	gpio_pin_write(GPIODevice, OLED_RES_PIN, 0);
	gpio_pin_write(GPIODevice, OLED_RES_PIN, 1);

	gpio_pin_write(GPIODevice, OLED_DC_PIN, 1);
	return 0;
}

int WriteOLED(u8_t data, u32_t flag)
{
	gpio_pin_write(GPIODevice, OLED_DC_PIN, flag);

	if (spi_write(SPIDevice, &data, 1))
	{
		return OLED_SPI_WRITE_FAIL;
	}
	return 0;
}

int CommandOLED(u8_t cmd)
{
	return WriteOLED(cmd, 0);
}

int DataOLED(u8_t data)
{
	return WriteOLED(data, 1);
}

int ScrollVertical(u8_t n)
{
	CommandOLED(0xa1);
	DataOLED(n);
	return 0;
}

// dir -1 : right, 0 : stop, 1 : left
// start : start row
// num : number of rows
// speed 0 : fast, 1 : normal, 2 : slow, 3 : slowest
int ScrollHorizontal(u8_t dir, u8_t start, u8_t num, u8_t speed)
{
	CommandOLED(0x96);
	DataOLED(dir);
	DataOLED(start);
	DataOLED(num);
	DataOLED(0);
	DataOLED(speed);
	return 0;
}

int StartScrollHorizontal()
{
	CommandOLED(0x9f);
	return 0;
}

int StopScrollHorizontal()
{
	CommandOLED(0x9e);
	return 0;
}

// 0 : all off, 1 : all on, 2 : reset normal display, 3 : inverse
int SetDisplayMode(u8_t mode)
{
	if (mode > 3) return -1;
	CommandOLED(0xa4 | mode);
	return 0;
}

// 0 : sleep, 1 : wakeup
int  SetSleepMode(u8_t mode)
{
	if (mode > 1) return -1;
	CommandOLED(0xae | mode);
	return 0;
}

int SetCursor(u8_t x, u8_t y, u8_t w, u8_t h)
{
	CommandOLED(0x15);
	DataOLED(x);
	DataOLED(x + w - 1);

	CommandOLED(0x75);
	DataOLED(y);
	DataOLED(y + h - 1);
	return 0;
}

int DrawRect(u8_t x, u8_t y, u8_t w, u8_t h,
			u8_t r, u8_t g, u8_t b)
{
	SetCursor(x, y, w, h);
	CommandOLED(0x5c);

	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			DataOLED(b);
			DataOLED(g);
			DataOLED(r);
		}
	}
	return 0;
}

int DrawChar(u8_t x, u8_t y, u8_t r, u8_t g, u8_t b, char c)
{
	SetCursor(x, y, 8, 11);
	CommandOLED(0x5c);

	int i = c * 11;
	for (int y = 0; y < 11; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if ((CharMap[i] & (1 << x)) != 0)
			{
				DataOLED(b);
				DataOLED(g);
				DataOLED(r);
			}
			else
			{
				DataOLED(0);
				DataOLED(0);
				DataOLED(0);
			}
		}
		i++;
	}
	return 0;
}

int DrawString(u8_t x, u8_t y, u8_t r, u8_t g, u8_t b, const char* str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		DrawChar(x + (i * 8), y, r, g, b, str[i]);
	}
	return 0;
}

const u8_t CharMap[1408] =
{
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,          //
	12,30,30,30,12,12,0,12,12,0,0,  // !
	54,54,20,0,0,0,0,0,0,0,0,       // "
	0,18,18,63,18,18,63,18,18,0,0,  // #
	12,30,51,3,30,48,51,30,12,12,0, // $
	0,0,35,51,24,12,6,51,49,0,0,    // %
	12,30,30,12,54,27,27,27,54,0,0, // &
	12,12,6,0,0,0,0,0,0,0,0,        // '
	24,12,6,6,6,6,6,12,24,0,0,      // (
	6,12,24,24,24,24,24,12,6,0,0,   // )
	0,0,0,51,30,63,30,51,0,0,0,     // *
	0,0,0,12,12,63,12,12,0,0,0,     // +
	0,0,0,0,0,0,0,12,12,6,0,        // ,
	0,0,0,0,0,63,0,0,0,0,0,         // -
	0,0,0,0,0,0,0,12,12,0,0,        // .    
	0,0,32,48,24,12,6,3,1,0,0,      // /

	12,30,51,51,51,51,51,30,12,0,0, // 0
	12,14,15,12,12,12,12,12,63,0,0, // 1
	30,51,48,24,12,6,3,51,63,0,0,   // 2
	30,51,48,48,28,48,48,51,30,0,0, // 3
	16,24,28,26,25,63,24,24,60,0,0, // 4
	63,3,3,31,48,48,48,51,30,0,0,   // 5
	28,6,3,3,31,51,51,51,30,0,0,    // 6
	63,49,48,48,24,12,12,12,12,0,0, // 7
	30,51,51,51,30,51,51,51,30,0,0, // 8
	30,51,51,51,62,48,48,24,14,0,0, // 9

	0,0,12,12,0,0,12,12,0,0,0,      // :
	0,0,12,12,0,0,12,12,6,0,0,      // ;
	0,0,24,12,6,3,6,12,24,0,0,      // <
	0,0,0,63,0,0,63,0,0,0,0,        // =
	0,0,3,6,12,24,12,6,3,0,0,       // >
	30,51,51,59,59,59,27,3,30,0,0,  // @
	30,51,51,24,12,12,0,12,12,0,0,  // ?

	12,30,51,51,63,51,51,51,51,0,0,          // A ** TO BE FILLED **
	31,51,51,51,31,51,51,51,31,0,0, // B
	28,54,35,3,3,3,35,54,28,0,0,    // C
	15,27,51,51,51,51,51,27,15,0,0, // D
	63,51,35,11,15,11,35,51,63,0,0, // E
	63,51,35,11,15,11,3,3,3,0,0,    // F
	28,54,35,3,59,51,51,54,44,0,0,  // G
	51,51,51,51,63,51,51,51,51,0,0, // H
	30,12,12,12,12,12,12,12,30,0,0, // I
	60,24,24,24,24,24,27,27,14,0,0, // J
	51,51,51,27,15,27,51,51,51,0,0, // K
	3,3,3,3,3,3,35,51,63,0,0,       // L
	33,51,63,63,51,51,51,51,51,0,0, // M
	51,51,55,55,63,59,59,51,51,0,0, // N
	30,51,51,51,51,51,51,51,30,0,0, // O
	31,51,51,51,31,3,3,3,3,0,0,     // P
	30,51,51,51,51,51,63,59,30,48,0,// Q
	31,51,51,51,31,27,51,51,51,0,0, // R
	30,51,51,6,28,48,51,51,30,0,0,  // S
	63,63,45,12,12,12,12,12,30,0,0, // T
	51,51,51,51,51,51,51,51,30,0,0, // U
	51,51,51,51,51,30,30,12,12,0,0, // V
	51,51,51,51,51,63,63,63,18,0,0, // W
	51,51,30,30,12,30,30,51,51,0,0, // X
	51,51,51,51,30,12,12,12,30,0,0, // Y
	63,51,49,24,12,6,35,51,63,0,0,  // Z

	30,6,6,6,6,6,6,6,30,0,0,          // [
	0,0,1,3,6,12,24,48,32,0,0,        // "\"
	30,24,24,24,24,24,24,24,30,0,0,   // ]
	8,28,54,0,0,0,0,0,0,0,0,          // ^
	0,0,0,0,0,0,0,0,0,63,0,           // _
	6,12,24,0,0,0,0,0,0,0,0,          // `

	0,0,0,14,24,30,27,27,54,0,0,      // a
	3,3,3,15,27,51,51,51,30,0,0,      // b
	0,0,0,30,51,3,3,51,30,0,0,        // c
	48,48,48,60,54,51,51,51,30,0,0,  // d
	0,0,0,30,51,63,3,51,30,0,0,      // e
	28,54,38,6,15,6,6,6,15,0,0,      // f
	0,0,30,51,51,51,62,48,51,30,0,   // g
	3,3,3,27,55,51,51,51,51,0,0,     // h
	12,12,0,14,12,12,12,12,30,0,0,   // i
	48,48,0,56,48,48,48,48,51,30,0,  // j
	3,3,3,51,27,15,15,27,51,0,0,     // k
	14,12,12,12,12,12,12,12,30,0,0,  // l
	0,0,0,29,63,43,43,43,43,0,0,     // m
	0,0,0,29,51,51,51,51,51,0,0,     // n
	0,0,0,30,51,51,51,51,30,0,0,     // o
	0,0,0,30,51,51,51,31,3,3,0,      // p
	0,0,0,30,51,51,51,62,48,48,0,    // q
	0,0,0,29,55,51,3,3,7,0,0,        // r
	0,0,0,30,51,6,24,51,30,0,0,      // s
	4,6,6,15,6,6,6,54,28,0,0,        // t
	0,0,0,27,27,27,27,27,54,0,0,     // u
	0,0,0,51,51,51,51,30,12,0,0,     // v
	0,0,0,51,51,51,63,63,18,0,0,     // w
	0,0,0,51,30,12,12,30,51,0,0,     // x
	0,0,0,51,51,51,62,48,24,15,0,    // y
	0,0,0,63,27,12,6,51,63,0,0,      // z

	56,12,12,12,7,12,12,12,56,0,0,   // {
	12,12,12,12,12,12,12,12,12,0,0,  // |
	7,12,12,12,56,12,12,12,7,0,0,    // }
	38,45,25,0,0,0,0,0,0,0,0,        // ~
};