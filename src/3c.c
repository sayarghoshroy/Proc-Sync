#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

long long int arr[10000009];
long long int temp[10000009];
long long int N;

typedef struct passage
{
	long long int x, y;
}args;

pthread_attr_t attr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void merge(long long int L, long long int mid, long long int R)
{
	pthread_mutex_lock(&mutex);

	long long int p1=L,p2=mid+1,pos=0,i;
	while(p1!=mid+1||p2!=R+1)
	{
		if(p1==mid+1)
		{
			temp[pos]=arr[p2];
			p2++;
		}
		else if(p2==R+1)
		{
			temp[pos]=arr[p1];
			p1++;
		}
		else if(arr[p1]<arr[p2])
		{//ascending order
			temp[pos]=arr[p1];
			p1++;
		}
		else if(arr[p1] >= arr[p2])
		{
			temp[pos]=arr[p2];
			p2++;
		}
		pos++;
	}
	for(i=L;i<=R;i++)
		arr[i]=temp[i-L];

	pthread_mutex_unlock(&mutex);
}

void *select_sort(void *arguments)
{
	args* now = (args *)arguments;
	long long int L = now->x;
	long long int R = now->y;
	pthread_mutex_lock(&mutex);

	int i, j;
	for(i = L; i <= R; ++i)
	{
		int min_pos = i;
		for(j = i + 1; j <= R; ++j)
		{
			if(arr[min_pos] > arr[j])
				min_pos = j;
		}
		int swap = arr[min_pos];
		arr[min_pos] = arr[i];
		arr[i] = swap;
	}
	pthread_mutex_unlock(&mutex);
}

void *mergesort(void *arguments)
{
	args* now = (args *)arguments;
	long long int L = now->x;
	long long int R = now->y;

	int rc1, rc2;
	pthread_t t1, t2;
	long long int mid=L+(R-L)/2;

	args a, b;

	if((R-L) >= 5)
	{
		a.x = L;
		a.y = mid;
		if( (rc1 = pthread_create(&t1, &attr, mergesort, (void *)&a)))
			printf("Internal Failure\n");
		b.x = mid+1;
		b.y = R;
		if( (rc2 = pthread_create(&t2, &attr, mergesort, (void *)&b)))
			printf("Internal Failure\n");
		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
		merge(L,mid,R);
	}
	else if((R-L) < 5)
	{
		a.x = L;
		a.y = R;
		if( (rc1 = pthread_create(&t1, &attr, select_sort, (void *)&a)))
			printf("Internal Failure\n");
		pthread_join(t1, NULL);
	}
}

void display()
{
	printf("The Array After Sorting:\n");
	long long int i;
	for(i=0;i<N;i++)
		printf("%lld ",arr[i]);
	printf("\n");
}

void write_it()
{
	FILE *fp;
	fp = fopen("3c_op.txt", "w");
	long long int i;
	for(i=0;i<N;++i)
		fprintf(fp, "%lld ",arr[i]);
	fprintf(fp, "\n");
}

int main()
{
	long long int i;
	scanf("%lld",&N);

	for(i=0;i<N;i++)
		scanf("%lld", &arr[i]);
	args a;
	a.x = 0;
	a.y = N-1;

	pthread_t t;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_create(&t, &attr, mergesort, (void *)(void *)&a);

	pthread_join(t, NULL);
	pthread_attr_destroy(&attr);
	display();
	write_it();

	return 0;
}