//Authored: znfgnu, kacperh
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>

const int Q_SIZE = 5;
const int HOWMANY = 15;

typedef struct {
	int head;
	int tail;
	int data[Q_SIZE];
	int size;
	pthread_mutex_t mutex;
	pthread_cond_t full, empty;
} Queue;

pthread_mutex_t mutex_consumer;	// lock na obydwie kolejki
Queue qtab[2];					// tablica kolejek

void qinit(Queue* q) {
	q->head=q->tail=q->size=0;
	pthread_mutex_init(&(q->mutex), NULL);	// inicjalizowane 1
	pthread_cond_init(&(q->full), NULL);
	pthread_cond_init(&(q->empty), NULL);
}

// funcja wyjmujaca z kolejki
void qpop(Queue *q, int *recv) {
	pthread_mutex_lock(&(q->mutex));
	
	if (q->size == 0) {
		printf("Nie pobrano produktu - bufor pusty\n"); 
		pthread_cond_wait(&(q->empty), &(q->mutex));	// jesli jest pusta, czekamy az nie bedzie
	}
	// w tym miejscu na pewno cos jest w kolejce
	*recv = q->data[q->head++];
	q->size--;
	if (q->head == Q_SIZE) q->head=0;
	printf("Pobrano produkt %d\n", *recv);

	if (q->size == Q_SIZE-1) pthread_cond_signal(&(q->full));	// jesli pobralismy z kolejki, i byla ona przed chwila pusta, a juz nie jest, to odblokowujemy watek, ktory czekal na taka sytuacje (zablokowal sie, bo byla pelna)
	pthread_mutex_unlock(&(q->mutex));
}

void* consumer(void* id) {
	printf("Jeste konsumente %lld\n", (long long)id);
	int cnt = HOWMANY;
	while(cnt--) {
		int recvA, recvB;
		usleep(500000);	// konsument pobiera obie wartosci JEDNOCZESNIE co pol sekundy
		pthread_mutex_lock(&mutex_consumer);	// lockujemy OBYDWIE kolejki, aby pobrac pare wartosci
		qpop(&qtab[0], &recvA);
		qpop(&qtab[1], &recvB);
		printf("%lld < Odebralem (%d, %d)\n", (long long)id, recvA, recvB);
		pthread_mutex_unlock(&mutex_consumer);	// odblokowujemy obydwie kolejki
	}
	return NULL;
}

// czy sie udalo
void qpush(Queue *q, int val) {
	pthread_mutex_lock(&(q->mutex));
	
	if (q->size == Q_SIZE) {
		printf("Nie dodano produktu %d - bufor pelny\n", val);
		pthread_cond_wait(&(q->full), &(q->mutex));	// jesli pelna, czekamy az nie bedzie
	}
	// w tym miejscu na pewno bedzie miejsce w kolejce
	q->data[q->tail++] = val;
	q->size++;
	if (q->tail == Q_SIZE) q->tail=0;

	printf("Dodano produkt %d\n", val);
	if (q->size == 1) pthread_cond_signal(&(q->empty));	// jesli jakis watek czekal, az kolejka bedzie niepusta, odblokowujemy ten watek
	pthread_mutex_unlock(&(q->mutex));
}

void* producer(void* id) {
	printf("Jeste producente\n");
	int cnt = 2*HOWMANY;	// produkujemy 2 razy wiecej, niz konsument pobiera PAR
	while(cnt--) {
		int gen = rand() % 100;
		printf("%c > Generacja %d\n", (char)('A'+(long long)id), gen);
		usleep(100000);		// producent generuje pojedyncza wartosc co 100ms
		qpush(&qtab[(long long)id], gen);
	}
	
	return NULL;
}

int main() {
	pthread_t pthr_consumerA, pthr_producerA, pthr_consumerB, pthr_producerB;
	
	srand(639473);
	
	qinit(&qtab[0]);
	qinit(&qtab[1]);
	pthread_mutex_init(&mutex_consumer, NULL);	
	
	pthread_create(&pthr_producerA, NULL, producer, (void*)0);
	pthread_create(&pthr_producerB, NULL, producer, (void*)1);
	pthread_create(&pthr_consumerA, NULL, consumer, (void*)0);
	pthread_create(&pthr_consumerB, NULL, consumer, (void*)1);
	
	pthread_join(pthr_producerA, NULL);
	pthread_join(pthr_consumerA, NULL);
	pthread_join(pthr_producerB, NULL);
	pthread_join(pthr_consumerB, NULL);
}

