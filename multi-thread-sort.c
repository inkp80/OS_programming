#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

int *arr;
int *temp;
int *result;
int key[3]={1,1,1};
int minValue;
int cnt = 0;
typedef struct{
	int from_index;
	int to_index;
	int flag;
} parameters;

typedef struct{
	int list1_start;
	int list1_end;
	int list2_start;
	int list2_end;
	int list3_start;
	int list3_end;
} mergeidx;

void minSet(int *a, int *b)
{
	if (a > b)
		minValue = *b;
	else 
		minValue = *a;
}

void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void *sortFunc(void* para) //Thread only acept void type parameter
{
	parameters *p = (parameters *)para;
	int startIdx, endIdx;
	startIdx = p->from_index;
	endIdx = p->to_index;
	for (int i = startIdx; i < endIdx; i++)
	{
		for (int j = i + 1; j<=endIdx; j++)
		{
			if (arr[i]>arr[j])
				swap(&arr[i], &arr[j]);
		}
	}

	key[p->flag]=0;	

	pthread_exit(0);
}

void *merge(void* para)
{
	mergeidx *m = (mergeidx *)para;
	while (key[0] || key[1] || key[2]);


	int list_1, list_2, list_3;
	list_1 = m->list1_start;
	list_2 = m->list2_start;
	list_3 = m->list3_start;

	int cur = 0;
		
	while (cur!=m->list2_end+1)
	{

		if (list_1 == m->list1_end+1)
		{
			temp[cur] = arr[list_2];
			list_2++;
			cur++;
			continue;
		}
		if (list_2 == m->list2_end+1)
		{
			temp[cur] = arr[list_1];
			list_1++;
			cur++;
			continue;
		}

		if (arr[list_1] > arr[list_2])
		{
			temp[cur] = arr[list_2];
			list_2++;
			cur++;
		}
		else
		{
			temp[cur] = arr[list_1];
			list_1++;
			cur++;
		}
	}

	list_1 = m->list1_start;

	cur = 0;
	while (cur != cnt)
	{
		if (list_1 == m->list2_end+1)
		{
			result[cur] = arr[list_3];
			list_3++;
			cur++;
			continue;
		}
		if (list_3 == m->list3_end+1)
		{
			result[cur] = temp[list_1];
			list_1++;
			cur++;
			continue;
		}

		if (temp[list_1] > arr[list_3])
		{
			result[cur] = arr[list_3];
			list_3++;
			cur++;
		}
		else
		{
			result[cur] = temp[list_1];
			list_1++;
			cur++;
		}
	}
	

	pthread_exit(0);

}

void main()
{
	freopen("sorting input.txt", "r", stdin);
	freopen("output.txt","w",stdout);
	arr = (int*)malloc(sizeof(int)* 1000);
	temp = (int*)malloc(sizeof(int)* 1000);
	result = (int*)malloc(sizeof(int)* 1000);


	while (scanf("%d", &arr[cnt]) != EOF)
		cnt++;

	printf("input : \n");
	for (int i = 0; i < cnt; i++)
		printf("%d ", arr[i]);
	puts("");

	parameters *task1 = (parameters *)malloc(sizeof(parameters));
	parameters *task2 = (parameters *)malloc(sizeof(parameters));
	parameters *task3 = (parameters *)malloc(sizeof(parameters));
	mergeidx *midx = (mergeidx *)malloc(sizeof(mergeidx));
	
	task1->from_index = 0; task1->to_index = (cnt / 3)-1; task1->flag=0;
	task2->from_index = (cnt / 3); task2->to_index = (cnt / 3) * 2 - 1; task2->flag=1;
	task3->from_index = (cnt / 3) * 2; task3->to_index = cnt - 1; task3->flag=2;
	
	//end of index = cnt - 1


	midx->list1_start = task1->from_index;
	midx->list1_end = task1->to_index;
	midx->list2_start = task2->from_index;
	midx->list2_end = task2->to_index;
	midx->list3_start = task3->from_index;
	midx->list3_end = task3->to_index;


	pthread_t tid[4];

	pthread_create(&tid[0], NULL, (void *)sortFunc, (void *)task1);
	pthread_create(&tid[1], NULL, (void *)sortFunc, (void *)task2);
	pthread_create(&tid[2], NULL, (void *)sortFunc, (void *)task3);
	pthread_create(&tid[3], NULL, (void *)merge, (void *)midx);
	
	pthread_join(tid[3], NULL);

	printf("result : \n");
	for (int i = 0; i < cnt; i++)
		printf("%d ", result[i]);
	puts("");


	free(arr);
	free(temp);
	free(result);
	free(task1);
	free(task2);
	free(task3);
	free(midx);
 	
	return;
}
