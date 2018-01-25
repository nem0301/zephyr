#include <zephyr.h>
#include <misc/printk.h>
#include <kernel.h>

#include <string.h>

#define STACKSIZE	1024
#define PRIORITY	7


// thread
void Print1(void);
void WakeUp(void);

K_THREAD_DEFINE(kPrintID, STACKSIZE, Print1, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kWakeUpID, STACKSIZE, WakeUp, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);

// work queue
K_THREAD_STACK_DEFINE(kMyStackArea, STACKSIZE);
struct k_work_q kMyWorkQueue;
void PrintError(struct k_work *item);
void PrintError2(struct k_work *item);

struct device_info
{
	struct k_work work;
	char name[16];
} wiMyDevice;

struct device_info wiMyDevice2;

int main()
{
	k_work_q_start(&kMyWorkQueue, kMyStackArea,
			K_THREAD_STACK_SIZEOF(kMyStackArea), PRIORITY + 1);

	strcpy(wiMyDevice.name, "name");
	strcpy(wiMyDevice2.name, "name");
	k_work_init(&wiMyDevice.work, (k_work_handler_t)PrintError);
	k_work_init(&wiMyDevice2.work, (k_work_handler_t)PrintError2);

	while(1)
	{
		k_sleep(1000);
	}
}

void PrintError(struct k_work *item)
{
	struct device_info* theDevice = 
		CONTAINER_OF(item, struct device_info, work);

	printk("%s\n", theDevice->name);
}

void PrintError2(struct k_work *item)
{
	struct device_info* theDevice = 
		CONTAINER_OF(item, struct device_info, work);

	printk("%s2\n", theDevice->name);
}


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
	k_sched_time_slice_set(10, 5);
	while (1)
	{
		printk("test1 %d\n", Counting());
		//k_sleep(10000);
		s64_t timeStamp;
		s64_t msSpent;
		
		timeStamp = k_uptime_get();
		k_sleep(5000);

		msSpent = k_uptime_delta(&timeStamp);
		printk("spent : %lld\n", msSpent);
	}
}


void WakeUp(void)
{
	while (1)
	{
		printk("wake up %d\n", Counting());
		k_work_submit_to_queue(&kMyWorkQueue, &wiMyDevice.work);
		k_work_submit_to_queue(&kMyWorkQueue, &wiMyDevice2.work);

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

