#include "Common.h"
#include "Timer.h"

K_TIMER_DEFINE(kMyTimer, ExpireFunction, NULL);


#ifdef CLOCK
K_THREAD_DEFINE(kKernelClockID, STACKSIZE, KernelClockThread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kHWClockID, STACKSIZE, HWClockThread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif

void InitTimer()
{
#ifdef TIMER
	k_timer_start(&kMyTimer, K_SECONDS(2), K_SECONDS(10));
#endif
}

void ExpireFunction(struct k_timer* timer)
{
	printk("expire\n");
}

void TimerThread()
{
	while (1)
	{
		k_sleep(1000);
	}
}

void KernelClockThread()
{
	while (1)
	{
		s64_t timeStamp;
		s64_t msSpent;
		
		timeStamp = k_uptime_get();
		k_sleep(5000);

		msSpent = k_uptime_delta(&timeStamp);
		printk("spent : %lld\n", msSpent);
	}
}
void HWClockThread()
{
	while (1)
	{
		u32_t startTime;
		u32_t stopTime;
		u32_t cycleSpent;
		u32_t nsSpent;

		startTime = k_cycle_get_32();
		k_busy_wait(5000000);
		k_sleep(3001);
		stopTime = k_cycle_get_32();

		cycleSpent = stopTime - startTime;
		nsSpent = SYS_CLOCK_HW_CYCLES_TO_NS(cycleSpent);
		printk("cycle : %u, ns : %u\n", cycleSpent, nsSpent);
	}
}
