#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<stdio.h>

typedef int buffer_item;			//buffer에 들어갈 item의 자료형은 정수형이다.
#define BUFFER_SIZE 10				//동적할당할 버퍼의 크기를 정한다.

buffer_item *buffer;
int rear=0;							//circular 배열의 뒤 꼬리 부분을 의미한다.
int front=0;						//앞부분을 의미한다.
pthread_mutex_t mutex;				//binary semaphore로 0과 1의 값만을 가지면서 mutual exlusion을 만족시킨다.	
sem_t empty;						//배열의 비어있는 공간의 수를 나타내는 semaphore
sem_t full;							//배열의 점유된 공간의 수를 나타내는 semaphore
sem_t m1, m2;

int cnt, ack; // cnt : size of buffer ack : buffer counter

int insert_item(buffer_item *item)
{

	sem_wait(&empty);			//flag 변수에 empty semaphore에대한 wait함수 호출값을 저장한다. wait함수는 empty의 값을 1 감소시키고, 만약 empty가 0이면 empty가 증가할때까지 코드 실행을 block한다.
	pthread_mutex_lock(&mutex);	//flag 변수에 mutex semaphore에 대한 lock함수를 호출하고 그 리턴값을 저장한다. lock함수는 wait함수와 같이 mutex를 1 감소, 0일시 block하는 역할을한다.
	
	buffer[rear]=*item;					//배열의 맨 뒤에 item추가
	rear=(rear+1)%BUFFER_SIZE;
	printf("insert : %d\n", rear);
	cnt++;
	
	sem_post(&m1);	//순서 강제를 위한 세마포어
	sem_wait(&m2); //순서 강제를 위한  세마포어
	
	pthread_mutex_unlock(&mutex);	//unlock함수를 호출한다. lock함수와 반대로 mutex를 1 증가시킨다.
	sem_post(&full);			//BUFFER에 데이터가 있음을 신호한다.
	return 0;								//정상으로 종료될 시에는 0을 리턴한다.
}

int remove_item(buffer_item *item){

	sem_wait(&full);		//BUFFER에 데이터가 있을 때 까지 기다린다.
	pthread_mutex_lock(&mutex);	//임계구역에 접근이 없을 때까지 기다린다.
	*item=buffer[front];				//배열 맨 앞의 원소를 item에 넣고
	buffer[front]=-1;					//그 index는 빈 공간으로 여긴다.
	front=(front+1)%BUFFER_SIZE;
	printf("remove : %d\n", front);
	cnt--;
	

	sem_post(&m1); //순서강제
	sem_wait(&m2);	//순서강제
	
	pthread_mutex_unlock(&mutex);	//unlock함수를 호출한다. lock함수와 반대로 mutex를 1 증가시킨다.
	sem_post(&empty); //BUFFER에 아이템이 감소했음을 신호한다.
	return 0;
}

void *monitoring()
{
	while(1){
	sem_wait(&m1); //순서강제
	printf("Ack num : %d, size of buffer : %d\n", ack, cnt);
	ack++;
	sem_post(&m2); //순서강제
	}
}

void *producer(void *param)
{	
	buffer_item item;
	while(1){
		int r=rand()%5; //난수를 생성하여
		sleep(r); //그 수만큼 sleep(대기)
		item=rand();
		insert_item(&item); //insert_item의 호출
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
	int iSleep=atoi(argv[1]);			//command line에서 매개변수로 받은인자들을 정수변환한다.
	int iNpro=atoi(argv[2]);			//isleep = 프로세스가 얼만큼 sleep할 것인가? iNpro = producer thread를 몇 개를 만들것인가?
	int iNcons=atoi(argv[3]);			//iNcons = consumer thread를 몇 개를 만들것인가?
	buffer=(buffer_item*)malloc(sizeof(buffer_item)*BUFFER_SIZE); //buffer를 주어진 크기만큼 동적할당한다.
	int i=0;							//for문
	for(i=0;i<BUFFER_SIZE;i++)			//처음엔 buffer의 모든 원소를 빈 상태로 만든다.
		buffer[i]=-1;			

	sem_init(&empty, 0, BUFFER_SIZE);		//empty변수는 처음에 buffer의 크기로 초기화된다. 왜냐하면 처음에는 배열이 모두 비어있기 때문이다.
	sem_init(&full, 0, 0);				    //full변수는 처음에 0으로 초기화된다.
	sem_init(&m1, 0, 0);
	sem_init(&m2, 0, 0);
	
	pthread_mutex_init(&mutex,NULL);		//mutex는 1로 초기화한다.
	pthread_t idP,idC,idM;					//producer, consumer, monitor 쓰레드의 아이디를 저장할 객체
	
	for(i=0;i<iNpro;i++)				//입력받은 개수만큼의 producer쓰레드를 먼저 생성한다.
	{
		pthread_create(&idP,NULL,producer,NULL);
	}
	for(i=0;i<iNcons;i++)				//입력받은 개수만큼의 consumer쓰레드를 생성한다.
	{
		pthread_create(&idC,NULL,consumer,NULL);
	}

	pthread_create(&idM, NULL,monitoring,NULL);
	sleep(iSleep);						//입력받은 시간동안만 메인함수를 실행하고 그후 메인함수를 종료시킨다. 따라서 각 thread들도 종료된다.

	return 0;
}