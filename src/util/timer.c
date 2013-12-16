#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "util/timer.h"

void add_timer(struct timer_list *timer)
{
	/* fix expires, use JIFFIES unit, absolute */
	unsigned long expires = timer->expires;
	timer->expires = expires * TIMES + timer_jiffies;
	init_timer(timer);
	pthread_mutex_lock(&timerlist_lock);
	if (timer_pending(timer))
		goto bug;
	internal_add_timer(timer);
	pthread_mutex_unlock(&timerlist_lock);
	return;
bug:
	pthread_mutex_unlock(&timerlist_lock);
	printf("bug: kernel timer added twice at %p.\n", __builtin_return_address(0));
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
	int ret;
	pthread_mutex_lock(&timerlist_lock);
	timer->expires = expires;
	ret = detach_timer(timer);
	internal_add_timer(timer);
	pthread_mutex_unlock(&timerlist_lock);
	return ret;
}

int del_timer(struct timer_list * timer)
{
	int ret;

	pthread_mutex_lock(&timerlist_lock);
	ret = detach_timer(timer);
	timer->list.next = timer->list.prev = NULL;
	pthread_mutex_unlock(&timerlist_lock);
	return ret;
}

static void *run_timer_list(void *arg)
{
	(void)arg;
	while (1) {
		pthread_mutex_lock(&timerlist_lock);
		struct list_head *head, *curr;
		if (!tv1.index) {
			int n = 1;
			do {
				cascade_timers(tvecs[n]);
			} while (tvecs[n]->index == 1 && ++n < NOOF_TVECS);
		}
repeat:
		head = tv1.vec + tv1.index;
		curr = head->next;
		if (curr != head) {
			struct timer_list *timer;
			void (*fn)(unsigned long);
			unsigned long data;
	
			timer = list_entry(curr, struct timer_list, list);
			fn = timer->function;
			data= timer->data;
	
			detach_timer(timer);
			timer->list.next = timer->list.prev = NULL;
			pthread_mutex_unlock(&timerlist_lock);
			fn(data);
			pthread_mutex_lock(&timerlist_lock);
			goto repeat;
		}
		tv1.index = (tv1.index + 1) & TVR_MASK;
		pthread_mutex_unlock(&timerlist_lock);
		++timer_jiffies;
		usleep(JIFFIES);
	}
	return NULL; /* Never */
}

int begin_timer(void)
{
	pthread_t pid;
	pthread_attr_t attr;
	int rr_max_priority;
	struct sched_param thread_param;
	int ret;
	init_timervecs();
	ret = pthread_attr_init(&attr);
	if (ret != 0) {
		printf("pthread attr init failed\n");
		return -1;
	}
	ret = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	if (ret != 0) {
		printf("pthread attr set scope PTHREAD_SCOPE_SYSTEM failed\n");
		return -1;
	}
	ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (ret != 0) {
		printf("pthread attr set schedule policy failed\n");
		return -1;
	}
	rr_max_priority = sched_get_priority_max(SCHED_RR);
	if (rr_max_priority == -1) {
		printf("get RR priority max failed\n");
		return -1;
	}
	memset(&thread_param, 0, sizeof(struct sched_param));
	thread_param.sched_priority = rr_max_priority;
	pthread_attr_setschedparam(&attr, &thread_param);
	ret = pthread_create(&pid, NULL, run_timer_list, NULL);
	if (ret < 0) {
		printf("thread create failed\n");
		return -1;
	}
	return 0;
}

/* for debug test */
#ifndef NDEBUG
static void func(unsigned long data)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("func called, arg %lu, time %d, %d\n", data, (int)tv.tv_sec, (int)tv.tv_usec);
}

int main(int argc, char *argv[])
{
	int i, sec, delay, sec2;
	if (argc < 4) {
		printf("usage: timer timeout delay timeout2\n");
		return -1;
	}
	sec = atoi(argv[1]);
	delay = atoi(argv[2]);
	sec2 = atoi(argv[3]);

	if (begin_timer() != 0)
		return -1;
	printf("timer running...\n");
	struct timer_list timer;
	memset(&timer, 0, sizeof(struct timer_list));
	timer.expires = sec;
	timer.function = func;
	timer.data = 7878UL;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("now time: %d, %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
	add_timer(&timer);
	for (i = 0; i < (sec + delay); i++)
		sleep(1);
	printf("after pause %d sec\n", sec + delay);
	gettimeofday(&tv, NULL);
	printf("now time: %d, %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
	memset(&timer, 0, sizeof(struct timer_list));
	timer.expires = sec2;
	timer.function = func;
	timer.data = 5656UL;
	add_timer(&timer);
	for (i = 0; i < (sec2 + delay); i++)
		sleep(1);
	return 0;
}
#endif

