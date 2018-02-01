#include "Common.h"
#include "Sync.h"


#ifdef SYNC_SEMA
K_THREAD_DEFINE(kSync1ID, STACKSIZE, Sync1Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kSync2ID, STACKSIZE, Sync2Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
#endif
K_SEM_DEFINE(kMySem, 0, 1);

#ifdef SYNC_MUTEX
K_THREAD_DEFINE(kSync3ID, STACKSIZE, Sync3Thread, NULL, NULL, NULL,
		PRIORITY - 2, 0, K_NO_WAIT);
K_THREAD_DEFINE(kSync4ID, STACKSIZE, Sync4Thread, NULL, NULL, NULL,
		PRIORITY - 1, 0, K_NO_WAIT);
K_THREAD_DEFINE(kSync5ID, STACKSIZE, Sync5Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kSync6ID, STACKSIZE, Sync6Thread, NULL, NULL, NULL,
		PRIORITY - 2, 0, K_NO_WAIT);
#endif
K_MUTEX_DEFINE(kMyMutex);

#ifdef SYNC_ALERT
K_THREAD_DEFINE(kSync7ID, STACKSIZE, Sync7Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kSync8ID, STACKSIZE, Sync8Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif
int AlertHandler(struct k_alert* alert);
K_ALERT_DEFINE(kMyAlert, AlertHandler, 10);

int i = 0;

// producer
void Sync1Thread()
{
	while (1)
	{
		if (kMySem.count == 1) 
		{ 
			printk("count is limit\n");
			k_sleep(1000);
			continue; 
		}

		k_sem_give(&kMySem);
		i++;
		printk("thread1 %d\n", i);
		k_sleep(1000);

		k_sleep(1000);
	}
}

// consumer
void Sync2Thread()
{
	while (1)
	{
		if (!k_sem_take(&kMySem, K_MSEC(50)))
		{
			printk("thread2 %d\n", i);
		}
		else
		{
			printk("thread2 input data not available\n");
		}

		k_sleep(1000);
	}
}

void Sync3Thread()
{
	k_sleep(5000);
	while (1)
	{
		printk("thread3 try lock\n");
		while (k_mutex_lock(&kMyMutex, K_FOREVER)); // waiting lock and donation
		printk("thread3 lock\n");
		printk("thread3 unlock\n");
		k_mutex_unlock(&kMyMutex);

		k_sleep(1000);
	}
}

void Sync4Thread()
{
	while (1)
	{
		printk("thread4\n");

		k_sleep(1000);
	}
}

void Sync5Thread()
{
	while (1)
	{
		printk("thread5 try lock\n");
		k_mutex_lock(&kMyMutex, K_FOREVER);
		printk("thread5 lock\n");
		k_busy_wait(10000000);
		printk("thread5 unlock\n");
		k_mutex_unlock(&kMyMutex);

		k_sleep(1000);
	}
}

void Reentrant(int cnt)
{
	if (cnt < 0) return;
	printk("Reentrant %d\n", cnt);
	while (k_mutex_lock(&kMyMutex, K_FOREVER));
	Reentrant(cnt - 1);
	k_mutex_unlock(&kMyMutex);
}

void Sync6Thread()
{
	while (1)
	{
		Reentrant(10);

		k_sleep(1000);
	}
}

bool tf = true;
int AlertHandler(struct k_alert* alert)
{
	k_sleep(3000);
	if (tf)
	{
		printk("alert %d\n", alert->send_count);
		tf = false;
		return 0;
	}
	else
	{
		printk("pend %d\n", alert->send_count);
		return 1;
	}
}

void Sync7Thread()
{
	while (1)
	{
		k_alert_send(&kMyAlert);
		printk("send alert %d\n", kMyAlert.send_count);
		k_sleep(1000);
	}
}

void Sync8Thread()
{
	while (1)
	{
		printk("wait alert %d\n", kMyAlert.send_count);
		int ret = k_alert_recv(&kMyAlert, K_FOREVER);
		printk("ret = %d, count = %d\n", ret, kMyAlert.send_count);
		k_sleep(900);
		if (kMyAlert.send_count < 3)
		{
			tf = true;
		}
	}
}
