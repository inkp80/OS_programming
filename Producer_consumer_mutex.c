#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<stdio.h>

typedef int buffer_item;			//buffer�� �� item�� �ڷ����� �������̴�.
#define BUFFER_SIZE 10				//�����Ҵ��� ������ ũ�⸦ ���Ѵ�.

buffer_item *buffer;
int rear=0;							//circular4 �迭�� �� ���� �κ��� �ǹ��Ѵ�.
int front=0;						//�պκ��� �ǹ��Ѵ�.
pthread_mutex_t mutex;				//binary semaphore�� 0�� 1�� ������ �����鼭 mutual exlusion�� ������Ų��.	
sem_t empty;						//�迭�� ����ִ� ������ ���� ��Ÿ���� semaphore
sem_t full;							//�迭�� ������ ������ ���� ��Ÿ���� semaphore

pthread_mutex_t m1,m2;

int cnt, ack; // cnt : size of buffer ack : buffer counter

// error check
int error_semWait;
int error_mutexLock;
int error_mutexUnlock;
int error_semPost;

int insert_item(buffer_item *item){

	error_semWait=sem_wait(&empty);			//flag ������ empty semaphore������ wait�Լ� ȣ�Ⱚ�� �����Ѵ�. wait�Լ��� empty�� ���� 1 ���ҽ�Ű��, ���� empty�� 0�̸� empty�� �����Ҷ����� �ڵ� ������ block�Ѵ�.
	error_mutexLock=pthread_mutex_lock(&mutex);	//flag ������ mutex semaphore�� ���� lock�Լ��� ȣ���ϰ� �� ���ϰ��� �����Ѵ�. lock�Լ��� wait�Լ��� ���� mutex�� 1 ����, 0�Ͻ� block�ϴ� �������Ѵ�.
	if(error_semWait!=0 || error_mutexLock!=0)			//flag �� �ϳ��� 0�� �ƴ� ���� �����ߴٸ� �̴� semaphore�� ����� �۵���Ű�� �� �� ���̹Ƿ� -1�� �����ϰ� �Լ��� �����Ѵ�.
		return -1;

	buffer[rear]=*item;					//�迭�� �� �ڿ� item�߰�
	rear=(rear+1)%BUFFER_SIZE;
	printf("insert : %d\n", rear);
	cnt++;
	
	error_mutexUnlock=pthread_mutex_unlock(&mutex);	//unlock�Լ��� ȣ���Ѵ�. lock�Լ��� �ݴ�� mutex�� 1 ������Ų��.
	error_semPost=sem_post(&full);
	if(error_mutexUnlock!=0 || error_semPost!=0)				//���� ���� ����ó���Ѵ�. 
		return -1;
	return 0;								//�������� ����� �ÿ��� 0�� �����Ѵ�.
}

int remove_item(buffer_item *item){

	error_semWait=sem_wait(&full);			//flag ������ empty semaphore������ wait�Լ� ȣ�Ⱚ�� �����Ѵ�. wait�Լ��� empty�� ���� 1 ���ҽ�Ű��, ���� empty�� 0�̸� empty�� �����Ҷ����� �ڵ� ������ block�Ѵ�.
	error_mutexLock=pthread_mutex_lock(&mutex);	//flag ������ mutex semaphore�� ���� lock�Լ��� ȣ���ϰ� �� ���ϰ��� �����Ѵ�. lock�Լ��� wait�Լ��� ���� mutex�� 1 ����, 0�Ͻ� block�ϴ� �������Ѵ�.

	if(error_semWait!=0 || error_mutexLock!=0)			//flag �� �ϳ��� 0�� �ƴ� ���� �����ߴٸ� �̴� semaphore�� ����� �۵���Ű�� �� �� ���̹Ƿ� -1�� �����ϰ� �Լ��� �����Ѵ�.
		return -1;

	*item=buffer[front];				//�迭 �� ���� ���Ҹ� item�� �ְ�
	buffer[front]=-1;					//�� index�� �� �������� �����.
	front=(front+1)%BUFFER_SIZE;
	printf("remove : %d\n", front);
	cnt--;

	error_mutexUnlock=pthread_mutex_unlock(&mutex);	//unlock�Լ��� ȣ���Ѵ�. lock�Լ��� �ݴ�� mutex�� 1 ������Ų��.
	error_semPost=sem_post(&empty);

	if(error_mutexUnlock!=0 || error_semPost!=0)				//���� ���� ����ó���Ѵ�. 
		return -1;
	return 0;
}

void *monitoring()
{
	while(1){
	pthread_mutex_lock(&m2);
	printf("Ack num : %d, size of buffer : %d\n", ack, cnt);
	ack++;
	pthread_mutex_unlock(&m1);
	}
}

void *producer(void *param)
{

	buffer_item item;
	while(1){
		pthread_mutex_lock(&m1);
		int r=rand()%3;			//0~5�� ������ ���� �ð����� thread�� ����Ű��.
		sleep(r);
		item=rand();
		insert_item(&item);
		//if(insert_item(item))	//�����Ǽ��� �迭�� �߰��Ѵ�. �׸��� �� ����� ���� error�� success�޼����� ����.
		//	fprintf(stderr,"report error condition");
		//else
		//	printf("producer produced %d\n",item);
		pthread_mutex_unlock(&m2);
	}
}
void *consumer(void *param)
{
	//consumer thread�� ������ �Լ�, �迭���� �ϳ��� ���� �����Ѵ�.
	buffer_item item;
	while(1){
		pthread_mutex_lock(&m1);
		int r=rand()%3;			//0~5�� ������ ���� �ð����� thread�� ����Ű��.
		sleep(r);
		remove_item(&item);
		//if(remove_item(&item))	//�迭���� �ϳ��� ���Ҹ� �����ϰ�, �� ������ ���� item�� �����Ѵ�. �׸��� �������� ���� �޼����� ����Ѵ�.
		//	fprintf(stderr,"report error condition");
		//else
		//	printf("consumer consumed %d\n",item);
		pthread_mutex_unlock(&m2);
	}
}
	
int main(int argc,char *argv[]){
	int iSleep=atoi(argv[1]);			//command line���� �Ű������� �������ڵ��� ������ȯ�Ѵ�.
	int iNpro=atoi(argv[2]);
	int iNcons=atoi(argv[3]);
	buffer=(buffer_item*)malloc(sizeof(buffer_item)*BUFFER_SIZE); //buffer�� �־��� ũ�⸸ŭ �����Ҵ��Ѵ�.
	int i=0;							//for��
	for(i=0;i<BUFFER_SIZE;i++)			//ó���� buffer�� ��� ���Ҹ� �� ���·� �����.
		buffer[i]=-1;			

	sem_init(&empty,0,BUFFER_SIZE);		//empty������ ó���� buffer�� ũ��� �ʱ�ȭ�ȴ�. �ֳ��ϸ� ó������ �迭�� ��� ����ֱ� �����̴�.
	sem_init(&full,0,0);				//full������ ó���� 0���� �ʱ�ȭ�ȴ�.

	pthread_mutex_init(&m1,NULL);
	pthread_mutex_init(&m2,NULL);
	
	pthread_mutex_init(&mutex,NULL);
	pthread_t idP,idC,idM;					//producer, consumer �������� ���̵� ���Ϲ��� ����
	
	for(i=0;i<iNpro;i++)				//�Է¹��� ������ŭ�� producer�����带 ���� �����Ѵ�.
	{
		pthread_create(&idP,NULL,producer,NULL);
	}
	for(i=0;i<iNcons;i++)				//�Է¹��� ������ŭ�� consumer�����带 �����Ѵ�.
	{
		pthread_create(&idC,NULL,consumer,NULL);
	}
	pthread_mutex_lock(&m2);
	pthread_create(&idM, NULL,monitoring,NULL);
	sleep(iSleep);

	return 0;
}