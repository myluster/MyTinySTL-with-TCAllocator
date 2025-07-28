#include "../include/HashBucketMemoryPool/SimpleMemoryPool.h"
#include <cstdlib>// malloc, free, exit
#include <iostream>//cerr,cout
#include <cassert>//assert
namespace SimpleMemoryPool
{
	SimpleMemoryPool::SimpleMemoryPool(size_t blockSize, size_t slotSize)
		:BlockSize_(blockSize),
		 // ȷ�� slotSize_ ���������� SimpleSlot �����䱶�� (���ڶ�����������)
		 // �������� slotSize С�� SimpleSlot �Ĵ�С��������ʹ�� sizeof(SimpleSlot)
		 SlotSize_((slotSize < sizeof(SimpleSlot)) ? sizeof(SimpleSlot) : slotSize),
		 firstBlock_(nullptr),
		 curSlot_(nullptr),
		 freeList_(nullptr),
		 lastSlot_(nullptr)
	{
		// ȷ���۴�С�� SimpleSlot ָ���С�ı��� (Ϊ����������Ķ���)
		// ����ȷ�� blockSize_ �㹻������������һ����
		assert(SlotSize_ % sizeof(SimpleSlot*) == 0); // �����������ʵĶ���Ҫ��
		assert(BlockSize_ >= SlotSize_); // ������װһ����

		// ��ʼʱ�����һ���ڴ��
		allocateNewBlock();
	}

	SimpleMemoryPool::~SimpleMemoryPool()
	{
		// �������ͷ����д�ϵͳ�������Ĵ��ڴ��
		SimpleSlot* currentBlock = firstBlock_;
		while (currentBlock)
		{
			SimpleSlot* nextBlock = currentBlock->next;//��һ�����ָ��
			free(static_cast<void*>(currentBlock));//��ʾת������ʽת��Ҳ�ǺϷ���
			currentBlock = nextBlock;
		}
	}

	//�����ָ�� p ��ʼ��ֱ����һ������align����ĵ�ַ���������ֽ���
	size_t SimpleMemoryPool::padPointer(char* p, size_t align)
	{
		if (align == 0 || align == 1)
			return 0;
		
		//���㵱ǰָ���ַ����������
		size_t remainder = reinterpret_cast<size_t>(p) % align;

		//�������Ϊ 0����ʾ�Ѷ��룬�������
		if (remainder == 0)
			return 0;

		//������Ҫ�����ֽ���
		return align - remainder;
	}

	// ��ϵͳ����һ���µ��ڴ��
	void SimpleMemoryPool::allocateNewBlock()
	{
		//��ϵͳ����һ��BlockSize_ ��С�����ڴ��
		void* newBlockRaw = malloc(BlockSize_);
		if (!newBlockRaw)
		{
			std::cerr << "Error: Failed to allocate new block of size" << BlockSize_ << std::endl;
			// ������������׳� std::bad_alloc�������Ϊ exit
			exit(1);
		}

		//���¿���ӵ� firstBlock_ �����ͷ��
		SimpleSlot* blockHeader = static_cast<SimpleSlot*>(newBlockRaw);
		blockHeader->next = firstBlock_;
		firstBlock_ = blockHeader;       // ��������ͷ��Ϊ�¿�

		//����ʵ�ʿ����ڴ����ʼλ�� (������ͷ����������ָ��Ŀռ�)
		char* blockBody = static_cast<char*>(newBlockRaw) + sizeof(SimpleSlot);

		// ���� blockBody ����һ�� slotSize_ ���������ֽ�
		size_t paddingSize = padPointer(blockBody, SlotSize_);

		// curSlot_ ָ�򾭹���������ĵ�һ���۵���ʼλ��
		curSlot_ = blockBody + paddingSize;

		// 4. ���㵱ǰ�������һ���۵Ľ����߽�
		lastSlot_ = static_cast<char*>(newBlockRaw) + BlockSize_;
	}

	void* SimpleMemoryPool::allocate()
	{
		//���ȴӿ��������ȡ�ѻ��յĲ�
		if (freeList_ != nullptr)
		{
			SimpleSlot* slot = popFreeList();
			return slot;
		}

		//��������Ϊ�գ��ӵ�ǰ���ڴ�����и��²�
		//��鵱ǰ���Ƿ����㹻�Ŀռ�������һ����
		if ((curSlot_ + SlotSize_) <= lastSlot_)
		{
			void* slot = curSlot_;
			curSlot_ += SlotSize_;
			return slot;
		}

		//��ǰ�ڴ��Ҳ�þ�����Ҫ��ϵͳ�����µ��ڴ��
		allocateNewBlock();
		//�ݹ���� allocate() �������·���Ŀ��л�ȡ��һ����
		return allocate();
	}


	// �ͷ�һ���ڴ�ۣ�����黹���ڴ��
	void SimpleMemoryPool::deallocate(void* ptr) {
		if (ptr == nullptr) return;

		// ���ͷŵ��ڴ��ת���� SimpleSlot* ����
		SimpleSlot* slot = static_cast<SimpleSlot*>(ptr);

		// ������ӵ����������ͷ��
		pushFreeList(slot);
	}

	// ��һ���������������ͷ��
	void SimpleMemoryPool::pushFreeList(SimpleSlot* slot) {
		slot->next = freeList_;
		freeList_ = slot;
	}

	// �ӿ�������ͷ��ȡ��һ����
	SimpleSlot* SimpleMemoryPool::popFreeList() {
		SimpleSlot* head = freeList_;
		if (head != nullptr) {
			freeList_ = head->next;
		}
		return head;
	}
}