#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

long long int *arr;
long long int temp[10000009];
long long int N;

void merge(long long int L, long long int mid, long long int R)
{
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
		else if(arr[p1]>=arr[p2])
		{
			temp[pos]=arr[p2];
			p2++;
		}
		pos++;
	}
	for(i=L;i<=R;i++)
		arr[i]=temp[i-L];
}

void select_sort(long long int L, long long int R)
{
	long long int i, j;
	for(i = L; i <= R; ++i)
	{
		long long int min_pos = i;
		for(j = i + 1; j <= R; ++j)
		{
			if(arr[min_pos] > arr[j])
				min_pos = j;
		}
		long long int swap = arr[min_pos];
		arr[min_pos] = arr[i];
		arr[i] = swap;
	}
}

void mergesort(long long int L, long long int R)
{
	pid_t pid_a = -1, pid_b = -1;
	int status;
	long long int mid=L+(R-L)/2;
	if((R-L) >= 5)
	{
		if((pid_a = fork()) < 0)
		{
			printf("Internal Error Occured");
			exit(1);
		}
		else if(pid_a == 0)
		{
			mergesort(L,mid);
			exit(0);
		}

		else if(pid_a > 0)
		{
		 	if((pid_b = fork()) < 0)
			{
		 		printf("Internal Error Occured");
		 		exit(1);
			}
		 	else if(pid_b == 0)
		 	{
				mergesort(mid+1,R);
				exit(0);
	 		}
	 	}
	}
	else if((R-L) < 5)
	{
		select_sort(L, R);
		return;
	}

	waitpid(pid_a, &status, 0);
	waitpid(pid_b, &status, 0);
	merge(L,mid,R);
	return;
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
	fp = fopen("3b_op.txt", "w");
	long long int i;
	for(i=0;i<N;++i)
		fprintf(fp, "%lld ",arr[i]);
	fprintf(fp, "\n");
}

int main()
{
	long long int i;
	int shmid;
	key_t key = IPC_PRIVATE;

	scanf("%lld",&N);
	size_t SHM_SIZE = (N * 2) * sizeof(long long int);

	shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
	if(shmid < 0)
	{
		printf("Internal Failure\n");
		exit(1);
	}

	arr = (long long int *)shmat(shmid, NULL, 0);
	if(arr == (long long int *)(-1))
	{
		printf("Internal Failure\n");
		exit(1);
	}

	for(i=0;i<N;i++)
		scanf("%lld",&arr[i]);

	mergesort(0,N-1);
	display();
	write_it();

	if(shmdt(arr) != -1)
	{
		if(shmctl(shmid, IPC_RMID, NULL) != -1)
			printf("Shared Segment Deleted\n");
		else
			printf("Deletion Failure\n");
	}
	else
		printf("Memory Detachment Failed\n");
	return 0;
}