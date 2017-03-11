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
	void findPage(int num) //���� �������� �����ϴ����� ����
	{
		//TLB
		node* scan = head;
		while (scan != NULL)
		{
			if (scan->pnum == num) //������ �������� �����Ѵ�
			{
				phy_add = 256 * scan->fnum;

				flag = 1; //�������� ã������ ǥ���Ѵ�

				if (scan == head) //�� �������� head�̴�
				{

					if (head->next == NULL) //head�� ������ ���
						return;
					if (head->next == tail) //head�� tail�� ���� (��, 2���� ��常 ����)
					{
						head = head->next;
						head->next = scan;
						scan->prev = head;
						scan->next = NULL;
						tail = scan;
						return;
					}

					else //��尡 3�� �̻� ���� ��
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
				if (scan == tail) //tail�� ����
				{
					phy_add = 256 * scan->fnum;
					return;
				}

				//head - mid - tail �� mid�� ��ġ
				//��, ��尡 3�� �̻��̰�, head�� tail�� �ƴ� ��
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

	void page_fault(node* pos) //memory�� ������ demand�� page�� �ø��� ���ؼ� replacement�� �ʿ���
	{

		//LRU stack implementation
		//stack implementation���� head�� victim�̴�.
		pos->fnum = head->fnum; //head�� �Ҵ�Ǿ� �ִ� frame num�� �Ҵ���
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
		if (isEmpty()) //memory�� ����ִ�.
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
		if (flag == 1) //������ �������� ������
			return;
		pfault_cnt++;
		if (size == 128) //page fault �߻�
		{
			page_fault(temp);
			return;
		}

		//page fault��, ������ page�� ��� �������� �ʴ´� (memory�� free space ����)
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
	printf("TLB hit rate : �� (%d) �� (%d) hit ����\n", cnt, cnt-pfault_cnt);
	printf("LRU hit rate : �� (%d) �� (%d) hit ����\n", cnt, pfault_cnt);
}