#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<stdio.h>

typedef int buffer_item;			//buffer에 들어갈 item의 자료형은 정수형이다.
#define BUFFER_SIZE 10				//동적할당할 버퍼의 크기를 정한다.

buffer_item *buffer;
int rear=0;							//circular4 배열의 뒤 꼬리 부분을 의미한다.
int front=0;						//앞부분을 의미한다.
pthread_mutex_t mutex;				//binary semaphore로 0과 1의 값만을 가지면서 mutual exlusion을 만족시킨다.	
sem_t empty;						//배열의 비어있는 공간의 수를 나타내는 semaphore
sem_t full;							//배열의 점유된 공간의 수를 나타내는 semaphore

pthread_mutex_t m1,m2;

int cnt, ack; // cnt : size of buffer ack : buffer counter

// error check
int error_semWait;
int error_mutexLock;
int error_mutexUnlock;
int error_semPost;

int insert_item(buffer_item *item){

	error_semWait=sem_wait(&empty);			//flag 변수에 empty semaphore에대한 wait함수 호출값을 저장한다. wait함수는 empty의 값을 1 감소시키고, 만약 empty가 0이면 empty가 증가할때까지 코드 실행을 block한다.
	error_mutexLock=pthread_mutex_lock(&mutex);	//flag 변수에 mutex semaphore에 대한 lock함수를 호출하고 그 리턴값을 저장한다. lock함수는 wait함수와 같이 mutex를 1 감소, 0일시 block하는 역할을한다.
	if(error_semWait!=0 || error_mutexLock!=0)			//flag 중 하나라도 0이 아닌 값을 리턴했다면 이는 semaphore를 제대로 작동시키지 못 한 것이므로 -1을 리턴하고 함수를 종료한다.
		return -1;

	buffer[rear]=*item;					//배열의 맨 뒤에 item추가
	rear=(rear+1)%BUFFER_SIZE;
	printf("insert : %d\n", rear);
	cnt++;
	
	error_mutexUnlock=pthread_mutex_unlock(&mutex);	//unlock함수를 호출한다. lock함수와 반대로 mutex를 1 증가시킨다.
	error_semPost=sem_post(&full);
	if(error_mutexUnlock!=0 || error_semPost!=0)				//위와 같이 예외처리한다. 
		return -1;
	return 0;								//정상으로 종료될 시에는 0을 리턴한다.
}

int remove_item(buffer_item *item){

	error_semWait=sem_wait(&full);			//flag 변수에 empty semaphore에대한 wait함수 호출값을 저장한다. wait함수는 empty의 값을 1 감소시키고, 만약 empty가 0이면 empty가 증가할때까지 코드 실행을 block한다.
	error_mutexLock=pthread_mutex_lock(&mutex);	//flag 변수에 mutex semaphore에 대한 lock함수를 호출하고 그 리턴값을 저장한다. lock함수는 wait함수와 같이 mutex를 1 감소, 0일시 block하는 역할을한다.

	if(error_semWait!=0 || error_mutexLock!=0)			//flag 중 하나라도 0이 아닌 값을 리턴했다면 이는 semaphore를 제대로 작동시키지 못 한 것이므로 -1을 리턴하고 함수를 종료한다.
		return -1;

	*item=buffer[front];				//배열 맨 앞의 원소를 item에 넣고
	buffer[front]=-1;					//그 index는 빈 공간으로 여긴다.
	front=(front+1)%BUFFER_SIZE;
	printf("remove : %d\n", front);
	cnt--;

	error_mutexUnlock=pthread_mutex_unlock(&mutex);	//unlock함수를 호출한다. lock함수와 반대로 mutex를 1 증가시킨다.
	error_semPost=sem_post(&empty);

	if(error_mutexUnlock!=0 || error_semPost!=0)				//위와 같이 예외처리한다. 
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
		int r=rand()%3;			//0~5초 사이의 임의 시간동안 thread를 대기시키다.
		sleep(r);
		item=rand();
		insert_item(&item);
		//if(insert_item(item))	//임의의수를 배열에 추가한다. 그리고 그 결과에 따라 error나 success메세지를 띄운다.
		//	fprintf(stderr,"report error condition");
		//else
		//	printf("producer produced %d\n",item);
		pthread_mutex_unlock(&m2);
	}
}
void *consumer(void *param)
{
	//consumer thread가 실행할 함수, 배열에서 하나의 값을 제거한다.
	buffer_item item;
	while(1){
		pthread_mutex_lock(&m1);
		int r=rand()%3;			//0~5초 사이의 임의 시간동안 thread를 대기시키다.
		sleep(r);
		remove_item(&item);
		//if(remove_item(&item))	//배열에서 하나의 원소를 제거하고, 그 제거한 값을 item에 저장한다. 그리고 실행결과에 따라 메세지를 출력한다.
		//	fprintf(stderr,"report error condition");
		//else
		//	printf("consumer consumed %d\n",item);
		pthread_mutex_unlock(&m2);
	}
}
	
int main(int argc,char *argv[]){
	int iSleep=atoi(argv[1]);			//command line에서 매개변수로 받은인자들을 정수변환한다.
	int iNpro=atoi(argv[2]);
	int iNcons=atoi(argv[3]);
	buffer=(buffer_item*)malloc(sizeof(buffer_item)*BUFFER_SIZE); //buffer를 주어진 크기만큼 동적할당한다.
	int i=0;							//for문
	for(i=0;i<BUFFER_SIZE;i++)			//처음엔 buffer의 모든 원소를 빈 상태로 만든다.
		buffer[i]=-1;			

	sem_init(&empty,0,BUFFER_SIZE);		//empty변수는 처음에 buffer의 크기로 초기화된다. 왜냐하면 처음에는 배열이 모두 비어있기 때문이다.
	sem_init(&full,0,0);				//full변수는 처음에 0으로 초기화된다.

	pthread_mutex_init(&m1,NULL);
	pthread_mutex_init(&m2,NULL);
	
	pthread_mutex_init(&mutex,NULL);
	pthread_t idP,idC,idM;					//producer, consumer 쓰레드의 아이디를 리턴받을 변수
	
	for(i=0;i<iNpro;i++)				//입력받은 개수만큼의 producer쓰레드를 먼저 생성한다.
	{
		pthread_create(&idP,NULL,producer,NULL);
	}
	for(i=0;i<iNcons;i++)				//입력받은 개수만큼의 consumer쓰레드를 생성한다.
	{
		pthread_create(&idC,NULL,consumer,NULL);
	}
	pthread_mutex_lock(&m2);
	pthread_create(&idM, NULL,monitoring,NULL);
	sleep(iSleep);

	return 0;
}