#include <zephyr.h>
#include <misc/printk.h>

void main(void)
{
	for (;;)
	{
		printk("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(1000);
	}
}
