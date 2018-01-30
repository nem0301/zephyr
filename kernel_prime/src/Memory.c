#include "Common.h"
#include "Memory.h"

#ifdef MEMORY_SLAB
K_THREAD_DEFINE(kMem1ID, STACKSIZE, Memory1Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kMem2ID, STACKSIZE, Memory2Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
#endif
K_MEM_SLAB_DEFINE(kMySlab, 400, 1, 4);

#ifdef MEMORY_POOL
K_THREAD_DEFINE(kMem3ID, STACKSIZE, Memory3Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kMem4ID, STACKSIZE, Memory4Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
#endif
K_MEM_POOL_DEFINE(kMyPool, 64, 4096, 1, 4);

#ifdef MEMORY_HEAP
K_THREAD_DEFINE(kMem5ID, STACKSIZE, Memory5Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
K_THREAD_DEFINE(kMem6ID, STACKSIZE, Memory6Thread, NULL, NULL, NULL,
		PRIORITY , 0, K_NO_WAIT);
#endif

void Memory1Thread()
{
	while (1)
	{
		char* blockPtr = NULL;
		if (!k_mem_slab_alloc(&kMySlab, (void**)&blockPtr, 100))
		{
			memset(blockPtr, 0, 400);
			printk("thread1 get block\n");
			k_sleep(1000);
		}
		else
		{
			printk("thread1 fail to get block\n");
		}

		if (blockPtr != NULL)
		{
			printk("thread1 free block\n");
			k_mem_slab_free(&kMySlab, (void**)&blockPtr);
		}

		k_sleep(1000);
	}
}

void Memory2Thread()
{
	while (1)
	{
		char* blockPtr = NULL;
		if (!k_mem_slab_alloc(&kMySlab, (void**)&blockPtr, 100))
		{
			memset(blockPtr, 0, 400);
			printk("thread2 get block\n");
			k_sleep(2000);
		}
		else
		{
			printk("thread2 fail to get block\n");
		}

		if (blockPtr != NULL)
		{
			printk("thread2 free block\n");
			k_mem_slab_free(&kMySlab, (void**)&blockPtr);
		}
		k_sleep(1000);
	}
}

void Memory3Thread()
{
	while (1)
	{
		struct k_mem_block block;
		memset(&block, 0, sizeof(struct k_mem_block));
		if (!k_mem_pool_alloc(&kMyPool, &block, 4096, 100))
		{
			memset(block.data, 0, 4096);
			printk("thread3 get block\n");
			k_sleep(1000);
		}
		else
		{
			printk("thread3 fail to get block\n");
		}

		if (block.data != NULL)
		{
			printk("thread3 free block\n");
			k_mem_pool_free(&block);
		}

		k_sleep(1000);
	}
}

void Memory4Thread()
{
	while (1)
	{
		struct k_mem_block block;
		memset(&block, 0, sizeof(struct k_mem_block));
		if (!k_mem_pool_alloc(&kMyPool, &block, 4096, 100))
		{
			memset(block.data, 0, 4096);
			printk("thread4 get block\n");
			k_sleep(2000);
		}
		else
		{
			printk("thread4 fail to get block\n");
		}

		if (block.data != NULL)
		{
			printk("thread4 free block\n");
			k_mem_pool_free(&block);
		}

		k_sleep(1000);
	}
}

void Memory5Thread()
{
	while (1)
	{
		char *memPtr = k_malloc(4096);
		if (memPtr != NULL)
		{
			memset(memPtr, 0, 4096);
			printk("thread5 get block\n");
			k_sleep(1000);
		}
		else
		{
			printk("thread5 fail to get block\n");
		}

		if (memPtr != NULL)
		{
			k_free(memPtr);
		}
		
		k_sleep(1000);
	}
}

void Memory6Thread()
{
	while (1)
	{
		char *memPtr = k_malloc(4096);
		if (memPtr != NULL)
		{
			memset(memPtr, 0, 4096);
			printk("thread6 get block\n");
			k_sleep(2000);
		}
		else
		{
			printk("thread6 fail to get block\n");
		}

		if (memPtr != NULL)
		{
			k_free(memPtr);
		}

		k_sleep(1000);
	}
}
