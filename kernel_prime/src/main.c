#include "Common.h"

// work queue
#include "WorkQueue.h"
// kernel timer
#include "Timer.h"

int main()
{
	InitTimer();
	InitWorkQueue();

	while(1)
	{
		k_sleep(10000);
	}
}

