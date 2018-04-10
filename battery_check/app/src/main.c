#include <zephyr.h>
#include <misc/printk.h>
#include <kernel.h>
#include <device.h>
#include <ipm.h>
#include <ipm/ipm_quark_se.h>
#include <gpio.h>
#include <spi.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <misc/byteorder.h>

#include <stdio.h>
#include <string.h>

#include "OLED.h"

#define STACKSIZE	8092
#define PRIORITY	1

#define DEVICE_NAME 		"Infomation"
#define DEVICE_NAME_LEN 	(sizeof(DEVICE_NAME) - 1)
#define TEMPERATURE_CUD		"Temperature"
#define HUMIDITY_CUD		"Humidity"
#define BATTERY_CUD			"Battery"

static s16_t gTempValue;
static u16_t gHumidityValue;
static u16_t gBatteryValue;

void DHT11Report(void);
void OLEDProc(void);
void IPMProc(void);

K_THREAD_DEFINE(kDHT11_ID, STACKSIZE, DHT11Report, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);

K_THREAD_DEFINE(kOLED_ID, STACKSIZE, OLEDProc, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);

K_THREAD_DEFINE(kIPM_ID, STACKSIZE, IPMProc, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);

struct DHT11Data
{
	void* reserved;
	u8_t temp;
	u8_t humi;
};

struct StringData
{
	void* reserved;
	char str[10][17];
};

K_FIFO_DEFINE(kDHT11_FIFO);
K_FIFO_DEFINE(kTEXT_FIFO);

void DHT11Report(void)
{
	k_sleep(1100);
	printk("x86\n");

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
				u16_t cnt = 0;
				do
				{
					gpio_pin_read(gpio_dev, 19, &val);
				} while (val == 0 && cnt++ < 300);
				k_busy_wait(30);
				// read data
				gpio_pin_read(gpio_dev, 19, &val);

				if (val == 1)
				{
					result[b] |= (1 << (7 - i));
				}

				cnt = 0;
				do
				{
					gpio_pin_read(gpio_dev, 19, &val);
				} while (val == 1 && cnt++ < 300);
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
			static struct DHT11Data dht11;
			dht11.temp = result[2];
			dht11.humi = result[0];
			gTempValue = dht11.temp;
			gHumidityValue = dht11.humi;
			k_fifo_put(&kDHT11_FIFO, &dht11);
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

	DrawRect(0, 0, 128, 128, 0, 0, 0);
	k_sleep(500);

	static char buf[17];
	while(1)
	{
		struct DHT11Data* data = k_fifo_get(&kDHT11_FIFO, K_NO_WAIT);
		if (data != NULL)
		{
			sprintf(buf, "%d C %d %%", data->temp, data->humi);
			DrawString(0, 0, 0, 127, 0, buf);
		}
		else
		{
			printk("dht11 is null\n");
		}
		struct StringData* strData = k_fifo_get(&kTEXT_FIFO, K_NO_WAIT);
		if (strData != NULL)
		{
			for (int i = 0; i < 10; i++)
			{
				sprintf(buf, "%s", strData->str[i]);
				DrawString(0, (i + 1) * 11, 0, 127, 0, buf);
			}
		}
		else
		{
			printk("bat is null\n");
		}
		k_sleep(1000);
	}
}

QUARK_SE_IPM_DEFINE(ess_ipm, 0, QUARK_SE_IPM_INBOUND);

static void sensor_ipm_callback(void* context, u32_t id, volatile void* data)
{
	volatile u32_t val = *(u32_t*)data;

	gBatteryValue = val;
	static struct StringData strData;
	sprintf(strData.str[0], "%3d%%", val);
	k_fifo_put(&kTEXT_FIFO, &strData);
}

void IPMProc(void)
{
	struct device* ipm;
	ipm = device_get_binding("ess_ipm");
	ipm_register_callback(ipm, sensor_ipm_callback, NULL);
	ipm_set_enabled(ipm, 1);

	k_sleep(K_FOREVER);
}

void BluetoothProc(void);
K_THREAD_DEFINE(kBLUETOOTH_ID, STACKSIZE, BluetoothProc, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);

static ssize_t ReadU16(struct bt_conn* conn, const struct bt_gatt_attr* attr,
	void* buf, u16_t len, u16_t offset)
{
	const u16_t* u16 = attr->user_data;
	u16_t value = sys_cpu_to_be16(*u16);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static struct bt_uuid_128 gBatteryUUID = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12
);

static struct bt_gatt_attr gAttrs[] = 
{
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ, 
		ReadU16, NULL, &gTempValue),
	BT_GATT_CUD(TEMPERATURE_CUD, BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_HUMIDITY, BT_GATT_PERM_READ, 
		ReadU16, NULL, &gHumidityValue),
	BT_GATT_CUD(HUMIDITY_CUD, BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(&gBatteryUUID.uuid, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(&gBatteryUUID.uuid, BT_GATT_PERM_READ, 
	ReadU16, NULL, &gBatteryValue),
	BT_GATT_CUD(BATTERY_CUD, BT_GATT_PERM_READ),
};

static struct bt_gatt_service gEnvSvc = BT_GATT_SERVICE(gAttrs);

static const struct bt_data gAD[] = 
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

static struct bt_data gSD[] = 
{
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void BluetoothReady(int err)
{
	if (err)
	{
		printk("Bluetooth init failed err=%d\n", err);
		return;
	}

	bt_gatt_service_register(&gEnvSvc);

	err = bt_le_adv_start(BT_LE_ADV_CONN, gAD, ARRAY_SIZE(gAD),
		gSD, ARRAY_SIZE(gSD));
	
	if (err)
	{
		printk("Advertising failed to start err=%d\n", err);
		return;
	}
}

void BluetoothProc(void)
{
	int rc = bt_enable(BluetoothReady);
	if (rc)
	{
		printk("Bluetooth init failed rc=%d\n", rc);
		return;
	}

	k_sleep(K_FOREVER);
}