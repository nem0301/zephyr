#include "Common.h"
#include "Thread.h"

#ifdef THREAD
K_THREAD_DEFINE(kPrintID, STACKSIZE, Print1, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kWakeUpID, STACKSIZE, WakeUp, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif

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

void Print1(void)
{
	k_sched_time_slice_set(10, 5); // init time slice
	while (1)
	{
		printk("test1 %d\n", Counting());
		k_sleep(10000);
	}
}


void WakeUp(void)
{
	while (1)
	{
		printk("wake up %d\n", Counting());
		k_sleep(10000);
	}
}
