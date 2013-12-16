#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "linux_list.h"
#include <pthread.h>
#include <sys/time.h>

struct timer_list {
	struct list_head list;
	unsigned long expires;
	unsigned long data;
	void (*function)(unsigned long);
};

static inline void init_timer(struct timer_list *timer)
{
	timer->list.next = timer->list.prev = NULL;
}

static inline int timer_pending(const struct timer_list *timer)
{
	return timer->list.next != NULL;
}

#define time_after(a,b) ((long)(b) - (long)(a) < 0)
#define time_before(a,b) time_after(b,a)
#define time_after_eq(a,b) ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b) time_after_eq(b,a)

#define TVN_BITS 6 
#define TVR_BITS 8 
#define TVN_SIZE (1 << TVN_BITS) 
#define TVR_SIZE (1 << TVR_BITS) 
#define TVN_MASK (TVN_SIZE - 1) 
#define TVR_MASK (TVR_SIZE - 1) 

#define JIFFIES 500000 /* usec */
#define TIMES (1000000 / JIFFIES) /* 1 sec == JIFFIES * TIMES */

struct timer_vec { 
	int index; 
	struct list_head vec[TVN_SIZE]; 
}; 

struct timer_vec_root { 
	int index; 
	struct list_head vec[TVR_SIZE]; 
}; 

static struct timer_vec tv5; 
static struct timer_vec tv4; 
static struct timer_vec tv3; 
static struct timer_vec tv2; 
static struct timer_vec_root tv1; 

static struct timer_vec * const tvecs[] = { 
	(struct timer_vec *)&tv1, &tv2, &tv3, &tv4, &tv5 
}; 
#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

static inline void init_timervecs(void)
{
	int i;
	for (i = 0; i < TVN_SIZE; i++) {
		INIT_LIST_HEAD(tv5.vec + i);
		INIT_LIST_HEAD(tv4.vec + i);
		INIT_LIST_HEAD(tv3.vec + i);
		INIT_LIST_HEAD(tv2.vec + i);
	}
	for (i = 0; i < TVR_SIZE; i++)
		INIT_LIST_HEAD(tv1.vec + i);
}

static unsigned long timer_jiffies;
pthread_mutex_t timerlist_lock = PTHREAD_MUTEX_INITIALIZER;

static inline void internal_add_timer(struct timer_list *timer)
{
	unsigned long expires = timer->expires;
	unsigned long idx = expires - timer_jiffies;
	struct list_head * vec;
	if (idx < TVR_SIZE) {
		int i = expires & TVR_MASK;
		vec = tv1.vec + i;
	} else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
		int i = (expires >> TVR_BITS) & TVN_MASK;
		vec = tv2.vec + i;
	} else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		vec = tv3.vec + i;
	} else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		vec = tv4.vec + i;
	} else if ((signed long) idx < 0) {
		/*
		 * if you add a timer with expires == jiffies,
		 * or you set a timer to go off in the past
		 */
		vec = tv1.vec + tv1.index;
	} else if (idx <= 0xffffffffUL) {
		int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		vec = tv5.vec + i;
	} else {
		INIT_LIST_HEAD(&timer->list);
		return;
	}
	/* timers in FIFO now */
	list_add(&timer->list, vec->prev);
}

static inline int detach_timer(struct timer_list *timer)
{
	if (!timer_pending(timer))
		return 0;
	list_del(&timer->list);
	return 1;
}

static inline void cascade_timers(struct timer_vec *tv)
{
	struct list_head *head, *curr, *next;

	head = tv->vec + tv->index;
	curr = head->next;

	while (curr != head) {
		struct timer_list *tmp;

		tmp = list_entry(curr, struct timer_list, list);
		next = curr->next;
		list_del(curr); // not needed
		internal_add_timer(tmp);
		curr = next;
	}
	INIT_LIST_HEAD(head);
	tv->index = (tv->index + 1) & TVN_MASK;
}

extern int begin_timer(void);

extern void end_timer(void);

extern void add_timer(struct timer_list *timer);

extern int mod_timer(struct timer_list *timer, unsigned long expires);

extern int del_timer(struct timer_list * timer);

#ifdef __cplusplus
}
#endif

#endif

