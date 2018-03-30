#include <zephyr.h>
#include <misc/printk.h>
#include <kernel.h>
#include <device.h>
#include <ipm.h>
#include <ipm/ipm_quark_se.h>
#include <gpio.h>
#include <spi.h>

#include <string.h>

#include "OLED.h"

#define STACKSIZE	8092
#define PRIORITY	1

void PrintReport(void);
void OLEDProc(void);

#ifdef THREAD
K_THREAD_DEFINE(kDHT11ID, STACKSIZE, PrintReport, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
#endif

K_THREAD_DEFINE(kOLEDID, STACKSIZE, OLEDProc, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);

QUARK_SE_IPM_DEFINE(ess_ipm, 0, QUARK_SE_IPM_INBOUND);
int Counting()
{
	u32_t count = 0;

	if (k_is_in_isr())
	{
	}
	else
	{
		count = (u32_t)k_thread_custom_data_get();
		count++;
		k_thread_custom_data_set((void*)count);
	}
	return count;
}

static void sensor_ipm_callback(void* context, u32_t id, volatile void* data)
{
	//volatile struct sensor_value* val = data;

	switch (id)
	{
	}
}

void PrintReport(void)
{
	k_sleep(3100);
	printk("x86\n");

	struct device *ipm;
	ipm = device_get_binding("ess_ipm");
	ipm_register_callback(ipm, sensor_ipm_callback, NULL);
	ipm_set_enabled(ipm, 1);

	struct device* gpio_dev;

	gpio_dev = device_get_binding("GPIO_0");
	if (!gpio_dev)
	{
		printk("cannot find %s\n", "GPIO_0");
		return;
	}

	/* capture initial time stamp */

	/* do work for some (short) period of time */
	/* capture final time stamp */

	 // k_busy_wait call takes 5us
	 // configure call takes 10us 
	 // read/write call takes 3us

	while (1)
	{
		gpio_pin_configure(gpio_dev, 19, (GPIO_DIR_OUT));
		gpio_pin_write(gpio_dev, 19, 1);
		k_sleep(1000);
		gpio_pin_write(gpio_dev, 19, 0);
		k_busy_wait(18000 - 3);
		gpio_pin_configure(gpio_dev, 19, (GPIO_DIR_IN));
		k_busy_wait(25);

		u32_t val1, val2;
		// verifiy
		gpio_pin_read(gpio_dev, 19, &val1);
		k_busy_wait(72);
		gpio_pin_read(gpio_dev, 19, &val2);
		k_busy_wait(72);

		if (val1 != 0 || val2 != 1)
		{
			printk("check fail\n");
			continue;
		}

		u8_t result[5] = {0, };
		for (int b = 0; b < 5; b++)
		{
			for (int i = 0; i < 8; i++)
			{
				u32_t val;
				do
				{
					gpio_pin_read(gpio_dev, 19, &val);
				} while (val == 0);
				k_busy_wait(30);
				// read data
				gpio_pin_read(gpio_dev, 19, &val);

				if (val == 1)
				{
					result[b] |= (1 << (7 - i));
				}

				do
				{
					gpio_pin_read(gpio_dev, 19, &val);
				} while (val == 1);
			}
		}
		
		u8_t checksum = 0;
		for (int i = 0; i < 4; i++)
		{
			// test
			checksum += result[i];
		}

		if (checksum == result[4])
		{
			printk("TEMP : %d C, HUMI : %d %%\n", result[2], result[0]);
		}
		else
		{
			printk("check sum fail\n");
		}

		k_sleep(1000);
	}
}

void OLEDProc(void)
{
	k_sleep(370);

	InitOLED();
	CommandOLED(0xaf);
	k_sleep(300);
	CommandOLED(0xa0);
	DataOLED(0b10110000);

	ScrollVertical(32);

	CommandOLED(0xb2);
	DataOLED(0xa4);
	DataOLED(0x00);
	DataOLED(0x00);

	DrawString(0, 0, 0, 127, 0, "test============");
	DrawRect(0, 0, 128, 128, 127, 127, 127);
	k_sleep(500);
	DrawRect(0, 0, 128, 128, 0, 0, 0);
	k_sleep(500);

	DrawString(0, 0, 0, 127, 0, "================");
	DrawString(0, 0, 0, 127, 0, "string7890123456");
	DrawString(0, 11, 0, 127, 0, "string7890123456");
	while(1);
}
