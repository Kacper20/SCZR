//authored znfgnu kacperh

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
const int Q_SIZE = 10;
const int HOWMANY = 100;

typedef struct {
	int head;
	int tail;
	int data[Q_SIZE];
	int size;
	pthread_mutex_t mutex;
	pthread_cond_t full, empty;
} Queue;

Queue q_A;

void qinit(Queue* q) {
	q->head=q->tail=q->size=0;
	pthread_mutex_init(&(q->mutex), NULL);	// inicjalizowane 1
	pthread_cond_init(&(q->full), NULL);
	pthread_cond_init(&(q->empty), NULL);
}

int qpop(Queue *q, int *recv) {
	int errcode=1;
	pthread_mutex_lock(&(q->mutex));
	
	if (q->size == 0) {
		printf("Nie pobrano produktu - bufor pusty\n"); 
		pthread_cond_wait(&(q->empty), &(q->mutex));
	}
	*recv = q->data[q->head++];
	q->size--;
	if (q->head == Q_SIZE) q->head=0;
	printf("Pobrano produkt %d\n", *recv);
	errcode = 0;

	if (q->size == Q_SIZE-1) pthread_cond_signal(&(q->full));
	pthread_mutex_unlock(&(q->mutex));
	return errcode;
}

void* consumer(void*) {
	printf("Jeste konsumente\n");
	int cnt = HOWMANY;
	while(cnt--) {
		int recv;
		usleep(500000);
		qpop(&q_A, &recv);
	}
	return NULL;
}

// czy sie udalo
int qpush(Queue *q, int val) {
	int errcode = 1;
	pthread_mutex_lock(&(q->mutex));
	
	if (q->size == Q_SIZE) {
		printf("Nie dodano produktu %d - bufor pelny\n", val);
		pthread_cond_wait(&(q->full), &(q->mutex));
	}
	q->data[q->tail++] = val;
	q->size++;
	if (q->tail == Q_SIZE) q->tail=0;
	errcode = 0;

	printf("Dodano produkt %d\n", val);
	if (q->size == 1) pthread_cond_signal(&(q->empty));
	pthread_mutex_unlock(&(q->mutex));
	
	return errcode;
}

void* producer(void*) {
	printf("Jeste producente\n");
	int cnt = HOWMANY;
	while(cnt--) {
		int gen = rand() % 100;
		usleep(100000);
		qpush(&q_A, gen);
	}
	
	return NULL;
}

int main() {
	pthread_t pthr_consumer, pthr_producer;
	
	srand(639473);
	
	qinit(&q_A);	
	
	pthread_create(&pthr_producer, NULL, producer, NULL);
	pthread_create(&pthr_consumer, NULL, consumer, NULL);
	
	pthread_join(pthr_producer, NULL);
	pthread_join(pthr_consumer, NULL);
}

