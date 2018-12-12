#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <math.h>

typedef struct {
	int load;
	int size;
	int *elements;
} Buffer;

// typedef struct {
// 	//pthread_t tid; 
// 	int id;
// 	int items;
// } ProducerThread;

Buffer buffer;

static int numBuffer;
static int counter = 0;
static int numConsumer;
static int consumerItems;
static int consumerItemsAll;
static int numProducer;
static int numItems;
static int numItemsAll;
static int Ptime;
static int Ctime;

static int *producerData;
static int *consumerData;

static sem_t empty;
static sem_t full;

static pthread_mutex_t mutexlock;

void produce(int *tid);
void consumer(int *tid);
// static pthread_cond_t empty;
// static pthread_cond_t full;

/* 
 * Function to remove item.
 * Item removed is returned
 */
int dequeue_item(Buffer *buffer)
{
	if(buffer->size == 0) {
		exit(1);
	}   	 
	buffer->size--;
	int item = buffer->elements[buffer->size];
	return item;
}

/* 
 * Function to add item.
 * Item added is returned.
 * It is up to you to determine
 * how to use the ruturn value.
 * If you decide to not use it, then ignore
 * the return value, do not change the
 * return type to void. 
 */
int enqueue_item(int item, Buffer *buffer)
{
	if(buffer->size == buffer->load) { // Check if buffer is full
		exit(1);
	}
	buffer->elements[buffer->size] = item;
	buffer->size++;
	return item;
}

void produce(int * tid) {
	// ProducerThread pt_info = *(ProducerThread*) arg;
	int i = 0;
	while( i < numItems) {

		sem_wait(&empty);
		pthread_mutex_lock(&mutexlock);
		//printf("\npthread id: %d items: %d iterator: %d", pt_info.id, pt_info.items, i);
		producerData[++numItemsAll - 1] = numItemsAll;
		printf("\n\t%d was produced by producer-> %d", enqueue_item(numItemsAll, &buffer), *tid);
		i++;
		sleep(Ptime);
		// in = (in + 1) % numBuffer;
		//counter++;
		// printf("\npthread id: %d items: %d iterator: %d", pt_info.id, pt_info.items, i);
		// pt_info.items++;
		//printf("\nAfter pthread id: %d items: %d iterator: %d", pt_info.id, pt_info.items, i);
		pthread_mutex_unlock(&mutexlock);
		sem_post(&full);
	}
	pthread_exit(0);
}

void consume(int * tid) {
	int i = 0;
	int item = 0;
	while ( i < consumerItems ) {
		sem_wait(&full);
		pthread_mutex_lock(&mutexlock);
		// while( counter == 0 ) pthread_cond_wait(&full, &mutexlock);
		// in = (in - 1) % numBuffer;
		// out = (out + 1) % BUFFER_SIZE;
		// counter--;
		// printf("\nconsume in buffer: %d", in);
		item = dequeue_item(&buffer);
		consumerData[++consumerItemsAll - 1] = item;
		printf("\n\t%d was consumed by consumer-> %d", item, * tid);
		i++;
		sleep(Ctime);
		// buffer[in] = 0;
		pthread_mutex_unlock(&mutexlock);
		sem_post(&empty);
	} 
	pthread_exit(0);
}

int main(int argc, char** argv) {

	if ( argc != 7 ) {
		printf("Must input 6 arguments like so: ./pandc N P C X Ptime Ctime\n");
		return 1;
	}

	numBuffer = pow(2, atoi(argv[1]));
	numProducer = pow(2, atoi(argv[2]));
	numConsumer = pow(2, atoi(argv[3]));
	numItems = atoi(argv[4]); // Items each producer thread will produce
	Ptime = atoi(argv[5]);
	Ctime = atoi(argv[6]);

	// (P*X)/C = Consumer items
	consumerItems = (numProducer * numItems) / numConsumer;
	// buffer = (int*) malloc(numBuffer * sizeof(int));
	// buffer_cpy = buffer;
	buffer.load = numBuffer; 
	buffer.size = 0;
	buffer.elements = (int*)malloc(numBuffer * sizeof(int) * buffer.load);

	numItemsAll = 0;
	consumerItemsAll = 0;

	sem_init(&empty, 0, numBuffer);
	sem_init(&full, 0, 0);

	time_t time_start;
	time(&time_start);

	printf("Current Time: %s\n", ctime(&time_start));

	printf("Number of Buffers: %d\nNumber of Producers: %d\nNumber of Consumers: %d\n", numBuffer, numProducer, numConsumer);
	printf("Number of items by each Producer: %d\n", numItems);
	printf("Number of items by each Consumer: %d\n", numConsumer);
	printf("Time each Producer Sleeps: %d\n", Ptime);
	printf("Time each Consumer Sleeps: %d", Ctime);

	pthread_t producers[numItems];
	pthread_t consumers[consumerItems];
	pthread_mutex_init(&mutexlock, NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// pthread_cond_init(&full, NULL);
	// pthread_cond_init(&empty, NULL);

	// ProducerThread pt_info[numItems];

	producerData = (int *)(malloc(sizeof(int) * numProducer * numItems));
	consumerData = (int *)(malloc(sizeof(int) * (numConsumer + 1) * consumerItems));

	int producerInfo[numItems];
	int consumerInfo[consumerItems];

	int i; 
	for (i = 0; i < numProducer; i++) {
		// pt_info[i].id = i;
		// pt_info[i].items = 0; // Note: items are not produced yet!
		pthread_create(&producers[i], &attr, (void*)produce, &producerInfo[i]);
	}

	for (i = 0; i < numConsumer; i++) {
		pthread_create(&consumers[i], &attr, (void*)consume, &consumerInfo[i]);
	}

	for (i = 0; i < numProducer; i++) {
		if(pthread_join(producers[i], NULL) != 0) {
			printf("\nProducer thread join failed\n");
		}
		printf("\nProducer Thread joined: %d", producerInfo[i]);
	}

	for( i = 0; i < numConsumer; i++) {
		if(pthread_join(consumers[i], NULL) != 0) {
			printf("\nConsumer thread join failed\n");
		}
		printf("\nConsumer Thread joined: %d", consumerInfo[i]);
	}

	time_t time_end;
	time(&time_end);

	printf("\nCurrent Time: %s\n", ctime(&time_end));

	printf("--------------------------------\n");
	printf("Producer Array\t| Consumer Array\n");
	printf("--------------------------------\n");
	for(i = 0; i < (numProducer * numItems); i++) {
		printf("%d\t\t| %d\n", producerData[i], consumerData[i]);
	}

	printf("Total Runtime: %0.f seconds\n", difftime(time_end, time_start));

	return 0;
}