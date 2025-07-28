#include "../include/HashBucketMemoryPool/HashBucketMemoryPool.h"
#include <cstdlib>//malloc,free,exit
#include <iostream>//cerr
#include <cassert>//assert
#include <mutex>//once_flag,call_once

namespace HashBucketMemoryPool
{
	//_s_pool_instances �����ָ��Ĭ�ϳ�ʼ��Ϊ nullptr
	SimpleMemoryPool* HashBucket::_s_pool_instances[static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM)] = {};
	
	//����ȷ�� HashBucket::initMemoryPools ֻ��ִ��һ�εı�־
	static std::once_flag hashbucket_init_flag;


    SimpleMemoryPool& HashBucket::getPoolInstance(size_t index)
    {
		assert(_s_pool_instances[index] != nullptr && "Error: HashBucket::initMemoryPools() not called or pool not initialized for this index!");
		return *_s_pool_instances[index];
    }

	size_t HashBucket::getPoolIndex(size_t size)
	{
		// ���������Ĺ�ʽ��(����ȡ��(size / SLOT_BASE_SIZE)) - 1
		return (size + static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL) / static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL;
	}

	void HashBucket::initMemoryPools()
	{
		// ʹ�� std::call_once ��֤��ʼ���߼�ִֻ��һ�Σ���ʹ���̲߳�������
		std::call_once(hashbucket_init_flag, []() {
			for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); ++i)
			{
				// ������ new SimpleMemoryPool ʵ������������ȷ�� slotSize
				// ʹ�� HashBucketConstants::BLOCK_SIZE_DEFAULT ��Ϊ����ڴ��С
				// ����ÿ���ض�Ӧ�� slotSize��ֱ���ڹ��캯��������
				size_t current_slot_size = (i + 1) * static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE);
				_s_pool_instances[i] = new SimpleMemoryPool(static_cast<size_t>(HashBucketConstants::BLOCK_SIZE_DEFAULT)); // ���캯��ֻ���� blockSize

				// ȷ��ÿ���´����ĳض�����ȷ��ʼ�� (������ init ����)
				_s_pool_instances[i]->init(current_slot_size); // init �������� slotSize ������״̬
			}
			std::cout << "HashBucket: All SimpleMemoryPools initialized." << std::endl;
			});
	}

	void* HashBucket::allocate(size_t size)
	{
		// ��������г�ʼ����飺��� HashBucket::initMemoryPools() δ�����ã�����δ׼����
		// _s_pool_instances[0] Ӧ���� initMemoryPools �����ú��Ƿ� nullptr��
		// ����һ�����Ƿ��Ѵ�����Ϊ��ʼ����־��
		if (_s_pool_instances[0] == nullptr) {
			std::cerr << "Error: HashBucket::initMemoryPools() not called before allocation! Exiting." << std::endl;
			exit(1); // �򻯴���ʵ�ʿ���Ӧ�׳��쳣
		}

		if (size <= 0)
			return nullptr;

		// ��������С�������۴�С������˵�ϵͳ malloc
		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			return malloc(size); // ʹ�� malloc
		}

		// ��ȡ��Ӧ��С���ڴ��ʵ���������з����ڴ�
		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		return getPoolInstance(static_cast<int>(index)).allocate();
	}

	void HashBucket::deallocate(void* ptr, size_t size)
	{
		// ��������г�ʼ�����
		if (_s_pool_instances[0] == nullptr) {
			std::cerr << "Error: HashBucket::initMemoryPools() not called before deallocation!" << std::endl;
			return; // �򻯴���ֱ�ӷ���
		}

		if (ptr == nullptr)
			return;

		// ����Ǵ������۴�С���ڴ棬����˵�ϵͳ free
		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			free(ptr); // ʹ�� free
			return;
		}

		// ��ȡ��Ӧ��С���ڴ��ʵ����������黹�ڴ�
		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		getPoolInstance(static_cast<int>(index)).deallocate(ptr);
	}


	//��̬�������Ķ��壬ȷ�����������ڳ����˳�ʱ����
	HashBucket::PoolCleaner HashBucket::_s_pool_cleaner;

	HashBucket::PoolCleaner::~PoolCleaner()
	{
		for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); i++)
		{
			if (_s_pool_instances[i] != nullptr)
			{
				delete _s_pool_instances[i];//�ͷ��ڴ��ʵ��
				_s_pool_instances[i] = nullptr;//��ֹҰָ��
			}
		}
	}


	//SimpleMemoryPool��ʵ�֣����ǽ�init���������
	SimpleMemoryPool::SimpleMemoryPool(size_t blockSize)
		:BlockSize_(blockSize),
		SlotSize_(0),// ��ʼʱ����Ϊ 0���ȴ� init ��������
		firstBlock_(nullptr),
		curSlot_(nullptr),
		freeList_(nullptr),
		lastSlot_(nullptr)
	{
		// ���캯��ֻ�����Ա��ʼ�������ٽ��ж��Ի��ڴ����
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


	void SimpleMemoryPool::init(size_t slotSize)
	{
		assert(slotSize > 0);// ȷ���۴�С�Ϸ�
		// ȷ�� slotSize_ ���������� SimpleSlot
		SlotSize_ = (slotSize < sizeof(SimpleSlot)) ? sizeof(SimpleSlot) : slotSize;


		//��֤�ڴ����
		assert(SlotSize_ % sizeof(SimpleSlot*) == 0);/// ȷ���۴�С��ָ���С�ı���
		assert(BlockSize_ >= SlotSize_);//ȷ���۴�С��ָ���С�ı���


		//��������״̬��ȷ���ش��ڸɾ��ĳ�ʼ״̬
		firstBlock_ = nullptr;
		curSlot_ = nullptr;
		freeList_ = nullptr;
		lastSlot_ = nullptr;
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
		firstBlock_ = blockHeader;// ��������ͷ��Ϊ�¿�


		//����ʵ�ʿ����ڴ����ʼλ�� (������ͷ����������ָ��Ŀռ�)
		char* blockBody = static_cast<char*>(newBlockRaw) + sizeof(SimpleSlot);

		//���� blockBody ����һ�� slotSize_ ���������ֽ�
		size_t paddingSize = padPointer(blockBody, SlotSize_);

		//curSlot_ ָ�򾭹���������ĵ�һ���۵���ʼλ��
		curSlot_ = blockBody + paddingSize;

		//���㵱ǰ�������һ���۵Ľ����߽�
		lastSlot_ = static_cast<char*>(newBlockRaw) + BlockSize_;
	}


	void* SimpleMemoryPool::allocate()
	{
		// ȷ�� init �����ѱ����ã�SlotSize_ ������
		if (SlotSize_ == 0) {
			std::cerr << "Error: SimpleMemoryPool::init() not called or slotSize is 0! Exiting." << std::endl;
			exit(1);
		}


		//���ȴӿ��������ȡ�ѻ��յĲ�
		if (freeList_ != nullptr)
		{
			SimpleSlot* slot = popFreeList();
			return slot;
		}


		//��������Ϊ�գ��ӵ�ǰ���ڴ�����и��²�
		//��� curSlot_ �Ƿ��ѳ�ʼ����ǰ���Ƿ��пռ�
		// ֻ�е� curSlot_ Ϊ nullptr (�״η���) ��ǰ�����þ�ʱ������Ҫ allocateNewBlock
		if (curSlot_ == nullptr || (curSlot_ + SlotSize_) > lastSlot_)
		{
			allocateNewBlock();
		}


		//��ʱ curSlot_ �϶�ָ���¿����ʼ������֮ǰ��ʣ��ռ�
		//�����ж� (curSlot_ + SlotSize_) <= lastSlot_ ��Ȼ�Ǳ�Ҫ�ģ��Դ��� allocateNewBlock() �շ��ص����
		if ((curSlot_ + SlotSize_) <= lastSlot_)
		{
			void* slot = curSlot_;
			curSlot_ += SlotSize_;
			return slot;
		}
		else
		{
			// ���ʾ allocateNewBlock() ����Ŀ鼴ʹ���µģ�Ҳ�޷�����һ���� (BlockSize_ < SlotSize_)
			std::cerr << "Error: Fresh block allocated but cannot fit a slot! Check BlockSize/SlotSize. Exiting." << std::endl;
			exit(1);
		}

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