#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<stdio.h>

typedef int buffer_item;			//buffer�� �� item�� �ڷ����� �������̴�.
#define BUFFER_SIZE 10				//�����Ҵ��� ������ ũ�⸦ ���Ѵ�.

buffer_item *buffer;
int rear=0;							//circular �迭�� �� ���� �κ��� �ǹ��Ѵ�.
int front=0;						//�պκ��� �ǹ��Ѵ�.
pthread_mutex_t mutex;				//binary semaphore�� 0�� 1�� ������ �����鼭 mutual exlusion�� ������Ų��.	
sem_t empty;						//�迭�� ����ִ� ������ ���� ��Ÿ���� semaphore
sem_t full;							//�迭�� ������ ������ ���� ��Ÿ���� semaphore
sem_t m1, m2;

int cnt, ack; // cnt : size of buffer ack : buffer counter

int insert_item(buffer_item *item)
{

	sem_wait(&empty);			//flag ������ empty semaphore������ wait�Լ� ȣ�Ⱚ�� �����Ѵ�. wait�Լ��� empty�� ���� 1 ���ҽ�Ű��, ���� empty�� 0�̸� empty�� �����Ҷ����� �ڵ� ������ block�Ѵ�.
	pthread_mutex_lock(&mutex);	//flag ������ mutex semaphore�� ���� lock�Լ��� ȣ���ϰ� �� ���ϰ��� �����Ѵ�. lock�Լ��� wait�Լ��� ���� mutex�� 1 ����, 0�Ͻ� block�ϴ� �������Ѵ�.
	
	buffer[rear]=*item;					//�迭�� �� �ڿ� item�߰�
	rear=(rear+1)%BUFFER_SIZE;
	printf("insert : %d\n", rear);
	cnt++;
	
	sem_post(&m1);	//���� ������ ���� ��������
	sem_wait(&m2); //���� ������ ����  ��������
	
	pthread_mutex_unlock(&mutex);	//unlock�Լ��� ȣ���Ѵ�. lock�Լ��� �ݴ�� mutex�� 1 ������Ų��.
	sem_post(&full);			//BUFFER�� �����Ͱ� ������ ��ȣ�Ѵ�.
	return 0;								//�������� ����� �ÿ��� 0�� �����Ѵ�.
}

int remove_item(buffer_item *item){

	sem_wait(&full);		//BUFFER�� �����Ͱ� ���� �� ���� ��ٸ���.
	pthread_mutex_lock(&mutex);	//�Ӱ豸���� ������ ���� ������ ��ٸ���.
	*item=buffer[front];				//�迭 �� ���� ���Ҹ� item�� �ְ�
	buffer[front]=-1;					//�� index�� �� �������� �����.
	front=(front+1)%BUFFER_SIZE;
	printf("remove : %d\n", front);
	cnt--;
	

	sem_post(&m1); //��������
	sem_wait(&m2);	//��������
	
	pthread_mutex_unlock(&mutex);	//unlock�Լ��� ȣ���Ѵ�. lock�Լ��� �ݴ�� mutex�� 1 ������Ų��.
	sem_post(&empty); //BUFFER�� �������� ���������� ��ȣ�Ѵ�.
	return 0;
}

void *monitoring()
{
	while(1){
	sem_wait(&m1); //��������
	printf("Ack num : %d, size of buffer : %d\n", ack, cnt);
	ack++;
	sem_post(&m2); //��������
	}
}

void *producer(void *param)
{	
	buffer_item item;
	while(1){
		int r=rand()%5; //������ �����Ͽ�
		sleep(r); //�� ����ŭ sleep(���)
		item=rand();
		insert_item(&item); //insert_item�� ȣ��
	}
}
void *consumer(void *param)
{
	buffer_item item;
	while(1){
		int r=rand()%5;
		sleep(r);
		remove_item(&item);
	}
}
	
int main(int argc,char *argv[]){
	int iSleep=atoi(argv[1]);			//command line���� �Ű������� �������ڵ��� ������ȯ�Ѵ�.
	int iNpro=atoi(argv[2]);			//isleep = ���μ����� ��ŭ sleep�� ���ΰ�? iNpro = producer thread�� �� ���� ������ΰ�?
	int iNcons=atoi(argv[3]);			//iNcons = consumer thread�� �� ���� ������ΰ�?
	buffer=(buffer_item*)malloc(sizeof(buffer_item)*BUFFER_SIZE); //buffer�� �־��� ũ�⸸ŭ �����Ҵ��Ѵ�.
	int i=0;							//for��
	for(i=0;i<BUFFER_SIZE;i++)			//ó���� buffer�� ��� ���Ҹ� �� ���·� �����.
		buffer[i]=-1;			

	sem_init(&empty, 0, BUFFER_SIZE);		//empty������ ó���� buffer�� ũ��� �ʱ�ȭ�ȴ�. �ֳ��ϸ� ó������ �迭�� ��� ����ֱ� �����̴�.
	sem_init(&full, 0, 0);				    //full������ ó���� 0���� �ʱ�ȭ�ȴ�.
	sem_init(&m1, 0, 0);
	sem_init(&m2, 0, 0);
	
	pthread_mutex_init(&mutex,NULL);		//mutex�� 1�� �ʱ�ȭ�Ѵ�.
	pthread_t idP,idC,idM;					//producer, consumer, monitor �������� ���̵� ������ ��ü
	
	for(i=0;i<iNpro;i++)				//�Է¹��� ������ŭ�� producer�����带 ���� �����Ѵ�.
	{
		pthread_create(&idP,NULL,producer,NULL);
	}
	for(i=0;i<iNcons;i++)				//�Է¹��� ������ŭ�� consumer�����带 �����Ѵ�.
	{
		pthread_create(&idC,NULL,consumer,NULL);
	}

	pthread_create(&idM, NULL,monitoring,NULL);
	sleep(iSleep);						//�Է¹��� �ð����ȸ� �����Լ��� �����ϰ� ���� �����Լ��� �����Ų��. ���� �� thread�鵵 ����ȴ�.

	return 0;
}