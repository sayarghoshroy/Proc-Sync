#include <stdio.h>

long long int arr[10000009];
long long int temp[10000009];
long long int N;

void merge(long long int L,long long int mid,long long int R)
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

void display()
{
	long long int i;
	for(i=0;i<N;i++)
		printf("%lld ",arr[i]);
	printf("\n");
}

void write_it()
{
	FILE *fp;
	fp = fopen("3a_op.txt", "w");
	long long int i;
	for(i=0;i<N;++i)
		fprintf(fp, "%lld ",arr[i]);
	fprintf(fp, "\n");
}

void mergesort(long long int L, long long int R)
{
	if(L!=R)
	{
		long long int mid=L+(R-L)/2;
		mergesort(L,mid);
		mergesort(mid+1,R);
		merge(L,mid,R);
	}
	return;
}

int main()
{
	long long int i;
	scanf("%lld",&N);
	for(i=0;i<N;i++)
		scanf("%lld",&arr[i]);
	mergesort(0,N-1);
	display();
	write_it();
	return 0;
}