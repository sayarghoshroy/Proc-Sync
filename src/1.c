#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

typedef long long int lint;
int players_left, referees_left;
int player_cnt = 0, ref_cnt = 0;
pthread_t t[10000007];

typedef struct passage
{
	int x;
	lint y;
}args;

lint args_id = 0;

args a[10000007];

lint time_elapsed = 0;

// State values
// 0 : Entered, waiting for Organizer
// 1 : With organizer
// 2 : Entered court, warming up or setting up equipment
// 3 : Game is on

int play_list[1000006];
int ref_list[1000006];
int flag_play[1000006];
int players_with_org = 0, ref_with_org = 0, going_in = 0, went_in = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_time()
{
	time_t now;
	time(&now);
	struct tm *local = localtime(&now);
	printf("At time %2d : %2d : %2d, ", local->tm_hour, local->tm_min, local->tm_sec);
	return;
}

int choose()
{
	int lie = rand() % (players_left + referees_left);
	if(lie < players_left)
	{
		//generate player
		--players_left;
		return 0;
	}
	else
	{
		//generate referee
		--referees_left;
		return 1;
	}
}

void startGame(int who, lint id)
{
	if(who == 0)
	{
		//print_time();
		//printf("Player %lld started playing\n", id + 1);
		if(flag_play[id / 2] == 1)
			return;
		else if(id % 2 == 1 && play_list[id - 1] == 3)
		{
			flag_play[id / 2] = 1;
			printf("Referee %lld started the game\n", (id % 2) + 1);
			return;
		}
		else if(id % 2 == 0 && play_list[id + 1] == 3)
		{
			flag_play[id / 2] = 1;
			printf("Referee %lld started the game\n", (id % 2) + 1);
			return;
		}
	}
	else
	{
		//print_time();
		printf("Referee %lld ready to start the game\n", id + 1);
	}
}

void warmUp(int who, lint id)
{
	sleep(1);
	pthread_mutex_lock(&mutex);
	//print_time();
	printf("Player %lld has warmed up\n", id + 1);
	play_list[id] = 3;
	pthread_mutex_unlock(&mutex);
	startGame(who, id);
}

void adjustEquipment(int who, lint id)
{
	sleep(0.5);
	pthread_mutex_lock(&mutex);
	//print_time();
	printf("Equipment Adjusted by Referee %lld\n", id + 1);
	ref_list[id] = 3;
	pthread_mutex_unlock(&mutex);
	startGame(who, id);
}

void enterCourt(int who, lint id)
{
	while(going_in == 1);
	if(who == 0)
	{
		//print_time();
		printf("Player %lld entered court\n", id + 1);
		warmUp(who, id);
	}
	else
	{
		//print_time();
		printf("Referee %lld entered court\n", id + 1);
		adjustEquipment(who, id);
	}
}

void meetOrganizer(int who, lint id)
{
	while(players_with_org < 2 || ref_with_org < 1);
	pthread_mutex_lock(&mutex);
	if(who == 0)
	{
		//print_time();
		printf("Player %lld met Organizer\n", id + 1);
		play_list[id] = 2;
	}
	else
	{
		//print_time();
		printf("Referee %lld met Organizer\n", id + 1);
		ref_list[id] = 2;
	}
	if(going_in == 0)
	{
		going_in = 1;
		went_in = 1;
	}
	else
	{
		if(went_in == 2)
		{
			going_in = went_in = players_with_org = ref_with_org = 0;
		}
		else
			++went_in;
	}
	pthread_mutex_unlock(&mutex);
	enterCourt(who, id);
}

void *enterAcademy(void *arguments)
{
	args * now = (args *)arguments;
	int who = now->x;
	long long int id = now->y;

	if(who == 0)
	{
		//is a player
		//print_time();
		printf("Player %lld entered Academy\n", id +1);
		while(!(id < 2 || play_list[id - 2] == 3));
		play_list[id] = 1;
		pthread_mutex_lock(&mutex);
		++players_with_org;
		pthread_mutex_unlock(&mutex);
		meetOrganizer(who, id);
	}
	else if(who == 1)
	{
		//is a ref
		//print_time();
		printf("Referee %lld entered Academy\n", id + 1);
		while(!(id < 1 || ref_list[id - 1] == 3));
		ref_list[id] = 1;
		pthread_mutex_lock(&mutex);
		++ref_with_org;
		pthread_mutex_unlock(&mutex);
		meetOrganizer(who, id);
	}
}

int main()
{
	int N;
	printf("Please Enter the Value of N i.e Number of Referees\n");
	scanf("%d", &N);
	players_left = 2 * N;
	referees_left = N;

	time_elapsed = 0;
	lint time_gen = rand() % 3;
	int next_up = 2;

	while(1)
	{
		if(time_elapsed == time_gen && (players_left > 0 || referees_left > 0) )
		{
			//generate a person
			next_up = rand() % 3;
			time_gen = time_elapsed + next_up;
			int gen = choose();
			if(gen == 0)
			{
				//make player
				a[args_id].x = 0;
				a[args_id].y = player_cnt;
				play_list[player_cnt++] = 0;
			}
			else
			{
				//make ref
				a[args_id].x = 1;
				a[args_id].y = ref_cnt;
				ref_list[ref_cnt++] = 0;
			}

			if(pthread_create(&t[player_cnt + ref_cnt], NULL, enterAcademy, &a[args_id]))
				printf("Internal Failure\n");
			++args_id;
		}

		if(next_up != 0)
			sleep(1);
		
		if(players_left == 0 && referees_left == 0)
		{
			int break_flag = 1;
			for(lint i = 0; i < N ; ++i)
			{
				if(play_list[i] != 3)
				{
					break_flag = 0;
					break;
				}
			}
			for(lint j = 0; j < N; ++j)
			{
				if(ref_list[j] != 3)
				{
					break_flag = 0;
					break;
				}
			}
			if(break_flag == 1)
			{
				for(lint i = 0; i < 3*N; ++i)
					pthread_join(t[i], NULL);
				break;
			}
		}

		if(next_up != 0)
			++time_elapsed;
	}
	return 0;
}

