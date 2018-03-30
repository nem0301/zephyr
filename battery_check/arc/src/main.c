#include <zephyr.h>
#include <misc/printk.h>
#include <misc/byteorder.h>
#include <kernel.h>
#include <device.h>
#include <ipm.h>
#include <ipm/ipm_quark_se.h>
#include <gpio.h>

#include <adc.h>

#include <string.h>

#define STACKSIZE	8092
#define PRIORITY	7
#define ADC_DEV_NAME	"ADC_0"

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

QUARK_SE_IPM_DEFINE(ess_ipm, 0, QUARK_SE_IPM_OUTBOUND);
static u32_t seq_buffer[2][10];

static struct adc_seq_entry sample = {
	.sampling_delay = 30,
	.channel_id = 10,
	.buffer_length = 10 * sizeof(seq_buffer[0][0])
};

static struct adc_seq_table table = {
	.entries = &sample,
	.num_entries = 1,
};

// static void _print_sample_in_hex(const u32_t *buf, u32_t length)
// {
// 	const u32_t *top;
// 	printk("Buffer content:\n");
// 	for (top = buf + length; buf < top; buf++)
// 		printk("0x%02u ", *buf / (0xfff0fff /100));
// 	printk("\n");
// }

static void PrintPercentOfBattery(const u32_t* buf, u32_t length)
{
	static u32_t count = 0;
	const u32_t *top;
	u32_t cnt = 0;
	u32_t sum = 0;;
	u32_t delta = 3256;
	for (top = buf + length; buf < top; buf++)
	{
		// printk("%08x ", *buf);
		u32_t val1 = *buf >> 16;
		u32_t val2 = *buf & 0xfff;
		if (val1 < delta)
		{
			val1 = 0;
		}
		else
		{
			val1 = ((val1 - delta) * 100) / 838;
		}
		if (val2 < delta)
		{
			val2 = 0;
		}
		else
		{
			val2 = ((val2 - delta) * 100) / 838;
		}
		sum += val1 + val2;
		cnt += 2;
	}
	// printk("\n");
	printk("%08u : %u\%\n", count++, sum / cnt);
}

static long _abs(long x)
{
	return x < 0 ? -x : x;
}

void main(void)
{
	struct device* adc_dev = device_get_binding(ADC_DEV_NAME);
	// struct deviec* ipm;

	int result;
	unsigned int loops = 10;
	unsigned int bufi0 = ~0, bufi;

	// ipm = device_get_binding("ess_ipm");
	// if (ipm == NULL)
	// {
	// 	printk("Failed to get ESS IPM device\n");
	// 	return;
	// }

	printk("arc\n");
	if (!adc_dev) {
		printk("arc Cannot get ADC device\n");
	} 
	else
	{
		printk("get device\n");
	}

	struct device* gpio_dev = device_get_binding("GPIO_0");
	if (!gpio_dev)
	{
		printk("gpio");
	}

	gpio_pin_configure(gpio_dev, 4, (GPIO_DIR_OUT));
	gpio_pin_write(gpio_dev, 4, 1);
	while (1)
	{
		adc_enable(adc_dev);
		loops++;
		bufi = loops & 0x1;
		sample.buffer = (void*)seq_buffer[bufi];
		result =  adc_read(adc_dev, &table);
		// printk("loop %u: buffer %u, result = %d\n", loops, bufi, result);
		// _print_sample_in_hex(seq_buffer[bufi], 10);
		PrintPercentOfBattery(seq_buffer[bufi], 10);
		if (bufi0 != ~0)
		{
			unsigned int cnt;
			long delta;

			for (cnt = 0; cnt < 10; cnt++)
			{
				delta = _abs((long)seq_buffer[bufi][cnt] - seq_buffer[bufi0][cnt]);
				// printk("loop %u delta %u = %08x\n", loops, cnt, delta);
			}
		}
		k_sleep(2000);
		bufi0 = bufi;
		adc_disable(adc_dev);
	}
}
