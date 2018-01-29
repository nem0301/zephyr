#include "Common.h"
#include "WorkQueue.h"

K_THREAD_STACK_DEFINE(kMyStackArea, STACKSIZE);
struct k_work_q kMyWorkQueue;
struct device_info wiMyDevice;
struct device_info wiMyDevice2;

K_THREAD_DEFINE(kWorkQueueID, STACKSIZE, WorkQueueThread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);

void InitWorkQueue()
{
	k_work_q_start(&kMyWorkQueue, kMyStackArea,
			K_THREAD_STACK_SIZEOF(kMyStackArea), PRIORITY + 1);

	strcpy(wiMyDevice.name, "name");
	strcpy(wiMyDevice2.name, "name");
	k_work_init(&wiMyDevice.work, (k_work_handler_t)PrintError);
	k_work_init(&wiMyDevice2.work, (k_work_handler_t)PrintError2);
}

void PrintError(struct k_work* item)
{
	struct device_info* theDevice = 
		CONTAINER_OF(item, struct device_info, work);

	printk("%s\n", theDevice->name);
}

void PrintError2(struct k_work* item)
{
	struct device_info* theDevice = 
		CONTAINER_OF(item, struct device_info, work);

	printk("%s2\n", theDevice->name);
}

void WorkQueueThread()
{
	while(1)
	{
		k_work_submit_to_queue(&kMyWorkQueue, &wiMyDevice.work);
		k_work_submit_to_queue(&kMyWorkQueue, &wiMyDevice2.work);

		k_sleep(10000);
	}
}
