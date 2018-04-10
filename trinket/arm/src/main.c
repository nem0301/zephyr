#include <zephyr.h>
#include <misc/printk.h>
#include <kernel.h>
#include <device.h>
#include <gpio.h>

#include <uart.h>

static struct device* uart0;
static void bt_uart_isr(struct device* unused)
{
	ARG_UNUSED(unused);

	// while (uart_irq_update(uart0) &&
	// 		uart_irq_is_pending(uart0))
	// {
	// 	if (!uart_irq_rx_ready(uart0))
	// 	{
	// 		if (uart_irq_tx_ready(uart0))
	// 		{
	// 			printk("transmit ready\n");
	// 		}
	// 		else
	// 		{
	// 			printk("spurious interrupt\n");
	// 		}
	// 		break;
	// 	}

	// }

	u8_t buf[10];
	uart_fifo_read(uart0, buf, 1);
	printk("%c", buf[0]);
}

void main(void)
{
	// struct device* gpio_dev = device_get_binding("GPIO_0");
	// if (!gpio_dev)
	// {
	// 	printk("gpio");
	// }

	// gpio_pin_configure(gpio_dev, 4, (GPIO_DIR_OUT));
	// gpio_pin_write(gpio_dev, 4, 1);

	uart0 = device_get_binding("SERCOM2");

	if (!uart0)
	{
		printk("uart0 fail \n");
	}

	uart_irq_rx_disable(uart0);
	uart_irq_tx_disable(uart0);

	uart_irq_callback_set(uart0, bt_uart_isr);

	uart_irq_rx_enable(uart0);

	while (1)
	{
		printk("test\n");
		k_sleep(1000);
	}
}

