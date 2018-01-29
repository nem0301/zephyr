struct device_info
{
	struct k_work work;
	char name[16];
};

void InitWorkQueue();
void PrintError(struct k_work* item);
void PrintError2(struct k_work* item);
void WorkQueueThread();
