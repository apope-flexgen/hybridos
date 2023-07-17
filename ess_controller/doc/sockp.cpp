#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * related blogpost at: http://sidekick.windforwings.com/2012/05/inter-thread-communication-socketpairs.html
 */

typedef void * q_t;

#define NUM_MSGS 100000

typedef int (*tmq_walker)(void *value, const void *ctx);

q_t tmq_create();
void tmq_destroy(q_t *q_in, tmq_walker free_walker_fn, const void *ctx);
int tmq_q(q_t q_in, void *value);
void * tmq_dq(q_t q_in);

typedef struct tmq_node {
	void *value;
	struct tmq_node *next;
} tmq_node_t; 

typedef struct tmq {
	tmq_node_t *head;
	tmq_node_t *read_pos;
	tmq_node_t *tail;

	pthread_mutex_t head_lock;
	pthread_mutex_t tail_lock;
} tmq_t;

q_t tmq_create() {
	tmq_t *ll = (tmq_t*)calloc((size_t)1, sizeof(tmq_t));

	ll->head = ll->read_pos = ll->tail = (tmq_node_t *) calloc((size_t)1, sizeof(tmq_node_t));
	if(pthread_mutex_init(&(ll->head_lock), nullptr)) {
		free(ll->head);
		return nullptr;
	}
	if(pthread_mutex_init(&(ll->tail_lock), nullptr)) {
		free(ll->head);
		pthread_mutex_destroy(&(ll->head_lock));
		return nullptr;
	}
	return ll;
}

void tmq_destroy(q_t *pp_ll, tmq_walker free_walker_fn, const void *ctx) {
	tmq_node_t *curr_node;
	tmq_node_t *next_node;

	if((nullptr == pp_ll) || (nullptr == *pp_ll)) return;

	tmq_t *ll = (tmq_t *)(*pp_ll);
	if(nullptr != free_walker_fn) {
		void *val_to_free;
		while(nullptr != (val_to_free = tmq_dq(ll))) {
			free_walker_fn(val_to_free, ctx);
		}
	}

	curr_node = ll->head;
	while(nullptr != curr_node) {
		next_node = curr_node->next;
		free(curr_node);
		curr_node = next_node;
	}

	pthread_mutex_destroy(&(ll->head_lock));
	pthread_mutex_destroy(&(ll->tail_lock));

	free(ll);
	*pp_ll = nullptr;
}

int tmq_q(q_t q_in, void *value) {
	tmq_t *ll = (tmq_t *)q_in;

	tmq_node_t *new_node = (tmq_node_t*)calloc((size_t)1, sizeof(tmq_node_t));
	if(nullptr == new_node) return -1;

	new_node->value = value;

	pthread_mutex_lock(&(ll->tail_lock));

	ll->tail = ll->tail->next = new_node;
	
	while(ll->head != ll->read_pos) {
		tmq_node_t *tmp = ll->head->next;
		free(ll->head);
		ll->head = tmp;
	}

	pthread_mutex_unlock(&(ll->tail_lock));

	return 0;
}


void * tmq_dq(q_t q_in) { 
	tmq_t *ll = (tmq_t *)q_in;
	void * ret;

	pthread_mutex_lock(&(ll->head_lock));
	if(ll->read_pos == ll->tail) {
		pthread_mutex_lock(&(ll->tail_lock));
		if(ll->read_pos == ll->tail) {
			pthread_mutex_unlock(&(ll->tail_lock));
			pthread_mutex_unlock(&(ll->head_lock));
			return nullptr;
		}
		pthread_mutex_unlock(&(ll->tail_lock));
	}

	ret = ll->read_pos->next->value;
	ll->read_pos = ll->read_pos->next;
	pthread_mutex_unlock(&(ll->head_lock));

	return ret;
}

//////////////////////////////////////////////////
// TEST ROUTINES BEGIN
/////////////////////////////////////////////////
//#define NUM_MSGS 10000000

static void * dequeuer(void *ll);
static void * enqueuer(void *ll);

void _tmq_test() {
	pthread_t q_thr;
	pthread_t dq_thr;

	q_t ll = tmq_create();
	pthread_create(&q_thr, nullptr, enqueuer, ll);
	pthread_create(&dq_thr, nullptr, dequeuer, ll);
	pthread_join(q_thr, nullptr);
	pthread_join(dq_thr, nullptr);
	tmq_destroy(&ll, nullptr, nullptr);
}


static void * enqueuer(void *ll) {
	long index;
	for(index=1; index < NUM_MSGS; index++) {
		//printf("eq %ld\n", (long)index);
		tmq_q(ll, (void *)index);
	}
	return nullptr;
}


static void * dequeuer(void *ll) {
	long index;
	void *test;
	printf("q dq for %d messages\n", NUM_MSGS);
	for(index=1; index < NUM_MSGS; index++) {
		void * ret = tmq_dq(ll);
		if(nullptr == ret) {
			index--;
			printf("sleeping\n");
			sleep(1); // may require some sleep when spinning.
			continue;
		}
		//printf("dq %ld idx %ld\n", (long)ret, index);
		assert(ret == (void *)index);
	}
	return nullptr;
}



static void * enqueuer_sockpair(void *pfd) {
	int fd = *((int *)pfd);
	long index;
	char data[1024*1];
	for(index=1; index < NUM_MSGS; index++) {
		write(fd, data, (1024*1));
	}
	return nullptr;
}


static void * dequeuer_sockpair(void *pfd) {
	int fd = *((int *)pfd);
	long index;
	char data[1024*1];
	for(index=1; index < NUM_MSGS; index++) {
		read(fd, data, (1024*1));
	}
	return nullptr;
}

static void _socketpair_test() {
	int fd[2];
	pthread_t q_thr;
	pthread_t dq_thr;

	printf("socketpair msg passing for %d messages\n", NUM_MSGS);

	socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	pthread_create(&q_thr, nullptr, enqueuer_sockpair, (void *)(&(fd[0])));
	pthread_create(&dq_thr, nullptr, dequeuer_sockpair, (void *)(&(fd[1])));
	pthread_join(q_thr, nullptr);
	pthread_join(dq_thr, nullptr);
	close(fd[0]);
	close(fd[1]);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Usage:\n");
		printf(" for in-memory queue: %s m\n", argv[0]);
		printf(" for socketpair: %s s\n", argv[0]);
		return 1;
	}
	if('m' == *(argv[1])) _tmq_test();
	else if('s' == *(argv[1])) _socketpair_test();
	return 0;
}
