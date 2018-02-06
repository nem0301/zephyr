#include "Common.h"
#include "DataPassing.h"

struct data_item_t
{
	void* reserved;
	int value;
};

#ifdef DATA_FIFO
K_THREAD_DEFINE(kFifo1ID, STACKSIZE, Fifo1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kFifo2ID, STACKSIZE, Fifo2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kFifo3ID, STACKSIZE, Fifo3Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif
K_FIFO_DEFINE(kMyFifo);

#ifdef DATA_LIFO
K_THREAD_DEFINE(kLifo1ID, STACKSIZE, Lifo1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kLifo2ID, STACKSIZE, Lifo2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kLifo3ID, STACKSIZE, Lifo3Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif
K_LIFO_DEFINE(kMyLifo);

#ifdef DATA_STACK
K_THREAD_DEFINE(kStack1ID, STACKSIZE, Stack1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kStack2ID, STACKSIZE, Stack2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif
K_STACK_DEFINE(kMyStack, 10);

#ifdef DATA_MESSAGE
K_THREAD_DEFINE(kMessage1ID, STACKSIZE, Message1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kMessage2ID, STACKSIZE, Message2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif
struct msgq_item_t
{
	u32_t filed1;
	u32_t filed2;
};
K_MSGQ_DEFINE(kMyMsgq, sizeof(struct msgq_item_t), 10, 4);

#ifdef DATA_MAILBOX
K_THREAD_DEFINE(kMailBox1ID, STACKSIZE, MailBox1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kMailBox2ID, STACKSIZE, MailBox2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kMailBox3ID, STACKSIZE, MailBox3Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif

K_THREAD_DEFINE(kPipe1ID, STACKSIZE, Pipe1Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(kPipe2ID, STACKSIZE, Pipe2Thread, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);


void Fifo1Thread()
{
	struct data_item_t data;
	data.value = 1;
	while (1)
	{
		printk("thread1 put fifo %d\n", data.value);
		k_fifo_put(&kMyFifo, &data);
		k_sleep(1000);
	}
}

void Fifo2Thread()
{
	struct data_item_t data;
	data.value = 2;
	while (1)
	{
		printk("thread2 put fifo %d\n", data.value);
		k_fifo_put(&kMyFifo, &data);
		k_sleep(1000);
	}
}

void Fifo3Thread()
{
	while (1)
	{
		struct data_item_t* data = k_fifo_get(&kMyFifo, K_FOREVER);
		printk("thread3 get fifo %d\n", data->value);
		k_sleep(500);
	}
}

void Lifo1Thread()
{
	struct data_item_t data;
	data.value = 1;
	while (1)
	{
		printk("thread1 put lifo %d\n", data.value);
		k_lifo_put(&kMyLifo, &data);
		k_sleep(1000);
	}
}

void Lifo2Thread()
{
	struct data_item_t data;
	data.value = 2;
	while (1)
	{
		printk("thread2 put lifo %d\n", data.value);
		k_lifo_put(&kMyLifo, &data);
		k_sleep(1000);
	}
}

void Lifo3Thread()
{
	while (1)
	{
		struct data_item_t* data = k_lifo_get(&kMyLifo, K_FOREVER);
		printk("thread3 get lifo %d\n", data->value);
		k_sleep(500);
	}
}


void Stack1Thread()
{
	int buffer[10];
	for (int i = 0; i < 10; i++)
	{
		buffer[i] = i;
		k_stack_push(&kMyStack, (u32_t)&buffer[i]);
	}
	while (1)
	{
		k_sleep(1000);
	}
}

void Stack2Thread()
{
	while (1)
	{
		int* data;
		k_stack_pop(&kMyStack, (u32_t*)&data, K_FOREVER);
		printk("data=%d\n", *data);
		k_sleep(1000);
	}
}

// msgq is to copy data. async
void Message1Thread()
{
	struct msgq_item_t data;
	memset(&data, 0, sizeof(struct msgq_item_t));
	while (1)
	{
		data.filed1 += 1;
		data.filed2 += 2;

		printk("megq1 = %d %d\n", data.filed1, data.filed2);
		while (k_msgq_put(&kMyMsgq, &data, K_NO_WAIT))
		{
			k_msgq_purge(&kMyMsgq);
		}

		k_sleep(500);
	}
}

void Message2Thread()
{
	struct msgq_item_t data;
	while (1)
	{
		k_msgq_get(&kMyMsgq, &data, K_FOREVER);
		printk("megq2 = %d %d\n", data.filed1, data.filed2);
		k_sleep(1000);
	}
}

K_MBOX_DEFINE(kMyMailBox);

K_SEM_DEFINE(kMySemMBox, 1, 1);

K_MEM_POOL_DEFINE(kMyPoolMBox, 4096, 4096, 2, 2);

void MailBox1Thread()
{
	char buffer[100];
	char text[100] = "text is not empty";
	int size = 10;

	struct k_mbox_msg sendMsg;
	struct k_mbox_msg sendMsg2;

	char hwBuffer[4096];

	while (1)
	{
		memset(&sendMsg, 0, sizeof(sendMsg));
		memset(&sendMsg2, 0, sizeof(sendMsg2));

		printk("th1 try to take sem\n");
		if (!k_sem_take(&kMySemMBox, K_NO_WAIT))
		{
			k_mem_pool_alloc(&kMyPoolMBox, &sendMsg2.tx_block, 4096, K_FOREVER);

			memcpy(sendMsg2.tx_block.data, hwBuffer, 4096);

			sendMsg2.size = 4096;
#ifdef DATA_MAILBOX
			sendMsg2.tx_target_thread = kMailBox3ID;
#endif

			printk("th1 async put\n");
			k_mbox_async_put(&kMyMailBox, &sendMsg2, &kMySemMBox);
		}
		else
		{
			printk("th1 waiting return to sem\n");
		}

		size = 10;
		memcpy(buffer, text, size-1);
		buffer[size-1] = '\0';

		sendMsg.info = size;
		sendMsg.size = size;
		sendMsg.tx_data = buffer;
#ifdef DATA_MAILBOX
		sendMsg.tx_target_thread = kMailBox2ID;
#endif

		printk("th1 sync put to th2 : %s \n", buffer);
		k_mbox_put(&kMyMailBox, &sendMsg, K_FOREVER);

		if (sendMsg.size < size)
		{
			printk("th1 data stil there\n");
		}

		k_sleep(1000);
	}
}

void MailBox2Thread()
{
	char buffer[100];
	struct k_mbox_msg recvMsg;
	int even = 0;

	k_sleep(2000);
	printk("th2 wake up\n");

	while (1)
	{
		recvMsg.info = 100;
		recvMsg.size = 100;
#ifdef DATA_MAILBOX
		recvMsg.rx_source_thread = kMailBox1ID;
#endif

		if (even % 2)
		{
			k_mbox_get(&kMyMailBox, &recvMsg, buffer, K_FOREVER);

			printk("th2 recv data : %s\n", buffer);

			if (recvMsg.info != recvMsg.size)
			{
				printk("th2 some messages are droped %d %lu\n", recvMsg.info, recvMsg.size);
			}
		}
		else
		{
			k_mbox_get(&kMyMailBox, &recvMsg, NULL, K_FOREVER);

			if (even % 4 == 0)
			{
				k_mbox_data_get(&recvMsg, buffer);
			}
			else
			{
				k_mbox_data_get(&recvMsg, NULL);
			}
		}

		even++;

		printk("\n");

		k_sleep(5000);
	}
}

void MailBox3Thread()
{
	struct k_mbox_msg recvMsg;
	struct k_mem_block recvBlock;

	while (1)
	{
		recvMsg.size = 4096;
#ifdef DATA_MAILBOX
		recvMsg.rx_source_thread = kMailBox1ID;
#endif

		k_mbox_get(&kMyMailBox, &recvMsg, NULL, K_FOREVER);

		k_mbox_data_block_get(&recvMsg, &kMyPoolMBox, &recvBlock, K_FOREVER);

		k_mem_pool_free(&recvBlock);
		k_sleep(5000);
	}
}

K_PIPE_DEFINE(kMyPipe, 16, 4);

void Pipe1Thread()
{
	unsigned char *data = k_malloc(128);
	size_t totalSize = 128;
	size_t n = 0;;

	int rc;

	for (unsigned char i = 0; i < totalSize; i++)
	{
		data[0] = i;
	}

	while (1)
	{
		rc = k_pipe_put(&kMyPipe, data, totalSize, &n, 16, K_NO_WAIT);

		if (rc < 0)
		{
			printk("th1 incomplete sent\n");
		}
		else if (n < totalSize)
		{
			printk("th1 %ld/%ld\n", n, totalSize);
		}
		else
		{
			printk("th1 done\n");
		}

		k_sleep(1000);
	}
}

void Pipe2Thread()
{
	unsigned char buffer[128];
	size_t n;
	int rc;

	while (1)
	{
		rc = k_pipe_get(&kMyPipe, buffer, sizeof(buffer), &n, 16, K_MSEC(100));

		if ((rc < 0) || n < 16)
		{
			printk("th2 incomplete received\n");
		}
		else if (n < 128)
		{
			printk("th2 %ld/%ld\n", n, 128);
		}
		else
		{
			printk("th2 done\n");
		}


		k_sleep(1000);
	}
}

