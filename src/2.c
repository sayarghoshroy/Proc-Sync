#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

int thread_run_type = 2;

typedef long long int lint;
int ret;

lint done_displayed[1000006];

pthread_t v_t[1000006], e_t[1000006], b_t[1000006], new_t[1000006];
lint new_trd = 0;

typedef struct voter
{
	lint booth_id;
	lint id_in_booth;
	lint state;
}voter;

typedef struct evm
{
	lint booth_id;
	lint id_in_booth;
	lint state;
	lint slots;
}evm;

typedef struct booth
{
	lint id;
	lint num_evms;
	lint num_voters;
	lint voters_left;
	lint active_evm;
	lint voters_waiting;
	int done;
}booth;

typedef struct evm_args
{
	lint evm_id;
	booth b;
	lint cnt;
}evm_args;

evm_args b[1000006];

typedef struct voter_args
{
	lint voter_id;
	booth b;
}voter_args;

voter_args a[1000006];

typedef struct booth_args
{
	lint id;
}booth_args;


voter voters[1000006];
evm evms[1000006];
booth booths[1000006];

lint total_evms = 0;
lint total_voters = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_2 = PTHREAD_COND_INITIALIZER;

void booth_init(lint idx, lint number_voters, lint number_emvs)
{
	booths[idx].id = idx;
	booths[idx].num_voters = number_voters;
	booths[idx].num_evms = number_emvs;
	booths[idx].voters_left = number_voters;
	booths[idx].active_evm = -1;
	booths[idx].done = 0;
	booths[idx].voters_waiting = 1;
}

void put_in_voters(lint booth_idx, lint num)
{
	for(lint i = 0; i < num; ++i)
	{
		voters[total_voters].booth_id =  booth_idx;
		voters[total_voters].id_in_booth = i;
		voters[total_voters].state = -1;
		++total_voters;
	}
}

void put_in_evms(lint booth_idx, lint num)
{
	for(lint i = 0; i < num; ++i)
	{
		evms[total_evms].booth_id = booth_idx;
		evms[total_evms].id_in_booth = i;
		evms[total_evms].slots = -1;
		evms[total_evms].state = -1;
		++total_evms;
	}
}

void voter_in_slot(booth b, lint idx, lint evm_id)
{
	printf("Voter %lld at booth %lld got allocated EVM %lld\n", voters[idx].id_in_booth + 1, b.id, evm_id + 1);
	pthread_mutex_lock(&mutex);
	voters[idx].state = 2;
	--booths[b.id].voters_waiting;
	pthread_mutex_unlock(&mutex);
}

void *voter_wait_for_evm(void *arguments)
{
	voter_args * now = (voter_args *)arguments;
	booth b = now->b;
	lint voter_id = now->voter_id;

	while(voters[voter_id].state == -1)
		pthread_cond_signal(&cond_1);

	voter_in_slot(booths[b.id], voter_id, booths[b.id].active_evm);
}

void *polling_ready_evm(void *arguments)
{
	evm_args * now = (evm_args *)arguments;
	booth b = now->b;
	lint evm_id = now->evm_id;
	lint slots = now->cnt;
	evms[evm_id].slots = slots;

	if(booths[b.id].done == 1)
		return &ret;

	printf("EVM %lld at Booth %lld is free with slots = %lld\n", evms[evm_id].id_in_booth + 1, b.id, slots);

	while((evms[evm_id].state == -1 || booths[b.id].voters_waiting > 0)  && booths[b.id].done != 1 )
		pthread_cond_signal(&cond_1);

	if(booths[b.id].done == 1)
		return &ret;

	evms[evm_id].state = -1;

	printf("EVM %lld at Booth %lld moving for voting stage.\n", evms[evm_id].id_in_booth + 1, b.id);

	printf("EVM %lld at Booth %lld finished voting stage.\n", evms[evm_id].id_in_booth + 1, b.id);

	evms[evm_id].slots = 1 + rand() % 10;
	now->cnt = evms[evm_id].slots;

	sleep(0.1);
	if(b.done != 1)
	{
		if(pthread_create(&new_t[new_trd++], NULL, polling_ready_evm, now))
			printf("Internal Failure\n");
	}
}


booth_args c[1000006];

lint max(lint a, lint b)
{
	if(a > b)
		return a;
	return b;
}

void booth_done(lint idx)
{
	int flag = 0;
	while(1)
	{
		flag = 1;
		for(lint i = 0; i < total_voters; ++i)
		{
			if(voters[i].booth_id == idx && voters[i].state != 2)
			{
				flag = 0;
				break;
			}
		}
		for(lint i = 0; i < total_evms; ++i)
		{
			if(evms[i].booth_id == idx && evms[i].state == 1)
			{
				flag = 0;
				break;
			}
		}
		if(flag == 1)
		{
			pthread_mutex_lock(&mutex);
			booths[idx].done = 1;
			pthread_mutex_unlock(&mutex);

			sleep(0.5);
			pthread_mutex_lock(&mutex_a);
			if(done_displayed[idx] == 0)
			{	
				done_displayed[idx] = 1;
				printf("Voters at Booth %lld are done with voting\n", idx);
			}
			pthread_mutex_unlock(&mutex_a);
			return;
		}
	}
}

lint min(lint a, lint b)
{
	if(a < b)
		return a;
	return b;
}

void *booth_operates(void *arguments)
{
	booth_args * now = (booth_args *)arguments;
	lint idx = now->id;

	while(1)
	{
		if(booths[idx].done == 1)
			break;
		if(booths[idx].voters_left == 0)
		{
			booth_done(idx);
			return &ret;
		}

		pthread_mutex_lock(&mutex);

		booths[idx].active_evm = (rand() % booths[idx].num_evms);

		lint evm_consider = -1;

		for(lint j = 0; j < total_evms; ++j)
		{
			if(evms[j].booth_id == idx && evms[j].id_in_booth == booths[idx].active_evm)
			{
				evm_consider = j;
				break;
			}
		}
		
		lint start = booths[idx].num_voters - booths[idx].voters_left;
		booths[idx].voters_waiting = min(start + evms[evm_consider].slots, booths[idx].num_voters) - start;

		for(lint j = start; j < min(start + evms[evm_consider].slots, booths[idx].num_voters); ++j)
		{
			for(lint k = 0; k < total_voters; ++k)
			{
				if(voters[k].booth_id == idx && voters[k].id_in_booth == j)
				{
					voters[k].state = 1;
					break;
				}
			}
		}

		booths[idx].voters_left = max(booths[idx].voters_left - evms[evm_consider].slots, 0);
		pthread_mutex_unlock(&mutex);

		while(booths[idx].voters_waiting > 0)
			pthread_cond_signal(&cond_2);

		evms[evm_consider].state = 1;

		if(booths[idx].voters_left == 0)
		{
			booth_done(idx);
			return &ret;
		}
		
		sleep(1);
	}
}

int main()
{
	lint N, x ,y;
	printf("Enter Number of Booths - ");
	scanf("%lld", &N);

	for(lint i = 1; i <= N; ++i)
	{
		scanf("%lld", &x);//number of voters
		scanf("%lld", &y);//number of evms
		booth_init(i, x, y);
		put_in_voters(i, x);
		put_in_evms(i, y);
	}

	for(lint i = 0; i < total_evms; ++i)
	{
		b[i].evm_id = i;
		b[i].b = booths[evms[i].booth_id];
		b[i].cnt = 1 + rand() % 10;
		if(pthread_create(&e_t[i], NULL, polling_ready_evm, &b[i]))
			printf("Internal Failure\n");
	}

	sleep(0.2);

	for(lint i = 0; i < total_voters; ++i)
	{
		a[i].voter_id = i;
		a[i].b = booths[voters[i].booth_id];
		if(pthread_create(&v_t[i], NULL, voter_wait_for_evm, &a[i]))
			printf("Internal Failure\n");
	}

	sleep(0.2);

	for(lint i = 1; i <= N; ++i)
	{
		c[i].id = i;
		if(pthread_create(&b_t[i], NULL, booth_operates, &c[i]))
			printf("Internal Failure");
	}

	while(1)
	{
		int flag = 0;
		for(lint i = 1; i <= N; ++i)
		{
			if(booths[i].done == 0)
			{
				flag = 1;
				break;
			}
		}
		if(flag == 1)
			continue;
		else
			break;
	}

	sleep(2);

	for(lint i = 0; i < total_voters; ++i)
		pthread_cancel(v_t[i]);
	for(lint i = 0; i < new_trd; ++i)
		pthread_cancel(new_t[i]);
	for(lint i = 0; i < total_evms; ++i)
		pthread_cancel(e_t[i]);
	for(lint i = 0; i < N; ++i)
		pthread_cancel(b_t[i]);
	return 0;
}