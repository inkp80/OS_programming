#include<stdio.h>
#include<cstring>
#include<cmath>
struct node
{
	node* prev;
	node* next;
	int fnum;
	int pnum;
};

struct tlb
{
	tlb* next;
	tlb* prev;
	int fr;
	int pa;
};

tlb* table;

int pfault_cnt;
int frame[129];
int phy_add;
int flag;
int cnt;
int vir_add;
int page, offset;

class doubleLinked
{
private:
	tlb* Thead;
	tlb* Ttail;

	node* head;
	node* tail;

	int tbsize;
	int size;
public:
	doubleLinked()
	{
		head = NULL;
		tail = NULL;
		size = 0;
	}
	bool isEmpty()
	{
		return (size == 0);
	}
	//void TLB()
	void findPage(int num) //같은 페이지가 존재하는지의 여부
	{
		//TLB
		node* scan = head;
		while (scan != NULL)
		{
			if (scan->pnum == num) //동일한 페이지가 존재한다
			{
				phy_add = 256 * scan->fnum;

				flag = 1; //페이지를 찾았음을 표시한다

				if (scan == head) //그 페이지가 head이다
				{

					if (head->next == NULL) //head가 유일한 노드
						return;
					if (head->next == tail) //head와 tail만 존재 (즉, 2개의 노드만 존재)
					{
						head = head->next;
						head->next = scan;
						scan->prev = head;
						scan->next = NULL;
						tail = scan;
						return;
					}

					else //노드가 3개 이상 있을 때
					{
						head = head->next;
						head->prev = NULL;
						tail->next = scan;
						scan->prev = tail;
						tail = scan;
						tail->next = NULL;
						return;
					}
				}
				if (scan == tail) //tail과 같음
				{
					phy_add = 256 * scan->fnum;
					return;
				}

				//head - mid - tail 중 mid에 위치
				//즉, 노드가 3개 이상이고, head도 tail도 아닐 때
				scan->prev->next = scan->next;
				scan->next->prev = scan->prev;
				tail->next = scan;
				scan->prev = tail;
				scan->next = NULL;
				tail = scan;

				phy_add = 256 * scan->fnum;
				return;
			}
			scan = scan->next;
		}
	}

	void page_fault(node* pos) //memory가 가득차 demand된 page를 올리기 위해서 replacement가 필요함
	{

		//LRU stack implementation
		//stack implementation에서 head가 victim이다.
		pos->fnum = head->fnum; //head에 할당되어 있는 frame num를 할당함
		frame[pos->fnum] = pos->pnum;
		head = head->next;
		head->prev = NULL;
		tail->next = pos;
		pos->prev = tail;
		tail = pos;

		phy_add = 256 * pos->fnum;
		return;

	}

	void demanding(int num)
	{
		flag = 0;
		node* temp = new node();
		temp->pnum = num;
		tlb* tb;
		if (isEmpty()) //memory가 비어있다.
		{
			pfault_cnt++;
			size++;
			head = temp;
			tail = head;
			head->fnum = 0;
			frame[0] = num;
			phy_add = 256 * temp->fnum;
		}

		findPage(temp->pnum);
		if (flag == 1) //동일한 페이지가 존재함
			return;
		pfault_cnt++;
		if (size == 128) //page fault 발생
		{
			page_fault(temp);
			return;
		}

		//page fault도, 동일한 page도 모두 존재하지 않는다 (memory에 free space 존재)
		size++;
		for (int i = 0; i < 128; i++)
		{
			if (frame[i] == -1)
			{
				temp->fnum = i;
				frame[i] = temp->pnum;
				break;
			}
		}
		tail->next = temp;
		temp->prev = tail;
		tail = temp;
		phy_add = 256 * temp->fnum;
	}

};


int main()
{
	freopen("input.txt", "r", stdin);
	freopen("physical.txt", "w", stdout);
	memset(frame, -1, sizeof(frame));
	doubleLinked OS;
	while (scanf("%d", &vir_add) != EOF)
	{
		printf("Logical address : %d\n", vir_add);
		cnt++;
		phy_add = 0;
		vir_add = vir_add & (1 << 16) - 1;
		page = vir_add / 256;
		offset = vir_add % 256;
		OS.demanding(page);
		phy_add += offset;
		printf("Physical address : %d\n", phy_add);
	}
	printf("TLB hit rate : 총 (%d) 중 (%d) hit 했음\n", cnt, cnt-pfault_cnt);
	printf("LRU hit rate : 총 (%d) 중 (%d) hit 했음\n", cnt, pfault_cnt);
}