#include "../include/HashBucketMemoryPool/HashBucketMemoryPool.h"
#include <cstdlib>//malloc,free,exit
#include <iostream>//cerr
#include <cassert>//assert
#include <mutex>//once_flag,call_once

namespace HashBucketMemoryPool
{
	//_s_pool_instances 数组的指针默认初始化为 nullptr
	SimpleMemoryPool* HashBucket::_s_pool_instances[static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM)] = {};
	
	//用于确保 HashBucket::initMemoryPools 只被执行一次的标志
	static std::once_flag hashbucket_init_flag;


    SimpleMemoryPool& HashBucket::getPoolInstance(size_t index)
    {
		assert(_s_pool_instances[index] != nullptr && "Error: HashBucket::initMemoryPools() not called or pool not initialized for this index!");
		return *_s_pool_instances[index];
    }

	size_t HashBucket::getPoolIndex(size_t size)
	{
		// 计算索引的公式：(向上取整(size / SLOT_BASE_SIZE)) - 1
		return (size + static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL) / static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL;
	}

	void HashBucket::initMemoryPools()
	{
		// 使用 std::call_once 保证初始化逻辑只执行一次，即使多线程并发调用
		std::call_once(hashbucket_init_flag, []() {
			for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); ++i)
			{
				// 在这里 new SimpleMemoryPool 实例，并传入正确的 slotSize
				// 使用 HashBucketConstants::BLOCK_SIZE_DEFAULT 作为大块内存大小
				// 计算每个池对应的 slotSize，直接在构造函数中设置
				size_t current_slot_size = (i + 1) * static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE);
				_s_pool_instances[i] = new SimpleMemoryPool(static_cast<size_t>(HashBucketConstants::BLOCK_SIZE_DEFAULT)); // 构造函数只接受 blockSize

				// 确保每个新创建的池都被正确初始化 (调用其 init 方法)
				_s_pool_instances[i]->init(current_slot_size); // init 方法设置 slotSize 和重置状态
			}
			std::cout << "HashBucket: All SimpleMemoryPools initialized." << std::endl;
			});
	}

	void* HashBucket::allocate(size_t size)
	{
		// 在这里进行初始化检查：如果 HashBucket::initMemoryPools() 未被调用，池则未准备好
		// _s_pool_instances[0] 应该在 initMemoryPools 被调用后是非 nullptr。
		// 检查第一个池是否已创建作为初始化标志。
		if (_s_pool_instances[0] == nullptr) {
			std::cerr << "Error: HashBucket::initMemoryPools() not called before allocation! Exiting." << std::endl;
			exit(1); // 简化处理，实际库中应抛出异常
		}

		if (size <= 0)
			return nullptr;

		// 如果请求大小超过最大槽大小，则回退到系统 malloc
		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			return malloc(size); // 使用 malloc
		}

		// 获取对应大小的内存池实例，并从中分配内存
		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		return getPoolInstance(static_cast<int>(index)).allocate();
	}

	void HashBucket::deallocate(void* ptr, size_t size)
	{
		// 在这里进行初始化检查
		if (_s_pool_instances[0] == nullptr) {
			std::cerr << "Error: HashBucket::initMemoryPools() not called before deallocation!" << std::endl;
			return; // 简化处理，直接返回
		}

		if (ptr == nullptr)
			return;

		// 如果是大于最大槽大小的内存，则回退到系统 free
		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			free(ptr); // 使用 free
			return;
		}

		// 获取对应大小的内存池实例，并向其归还内存
		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		getPoolInstance(static_cast<int>(index)).deallocate(ptr);
	}


	//静态清理对象的定义，确保析构函数在程序退出时运行
	HashBucket::PoolCleaner HashBucket::_s_pool_cleaner;

	HashBucket::PoolCleaner::~PoolCleaner()
	{
		for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); i++)
		{
			if (_s_pool_instances[i] != nullptr)
			{
				delete _s_pool_instances[i];//释放内存池实例
				_s_pool_instances[i] = nullptr;//防止野指针
			}
		}
	}


	//SimpleMemoryPool的实现，但是将init分离出来了
	SimpleMemoryPool::SimpleMemoryPool(size_t blockSize)
		:BlockSize_(blockSize),
		SlotSize_(0),// 初始时设置为 0，等待 init 方法设置
		firstBlock_(nullptr),
		curSlot_(nullptr),
		freeList_(nullptr),
		lastSlot_(nullptr)
	{
		// 构造函数只负责成员初始化，不再进行断言或内存分配
	}


	SimpleMemoryPool::~SimpleMemoryPool()
	{
		// 遍历并释放所有从系统申请来的大内存块
		SimpleSlot* currentBlock = firstBlock_;
		while (currentBlock)
		{
			SimpleSlot* nextBlock = currentBlock->next;//下一个块的指针
			free(static_cast<void*>(currentBlock));//显示转换，隐式转换也是合法的
			currentBlock = nextBlock;
		}
	}


	void SimpleMemoryPool::init(size_t slotSize)
	{
		assert(slotSize > 0);// 确保槽大小合法
		// 确保 slotSize_ 至少能容纳 SimpleSlot
		SlotSize_ = (slotSize < sizeof(SimpleSlot)) ? sizeof(SimpleSlot) : slotSize;


		//保证内存对齐
		assert(SlotSize_ % sizeof(SimpleSlot*) == 0);/// 确保槽大小是指针大小的倍数
		assert(BlockSize_ >= SlotSize_);//确保槽大小是指针大小的倍数


		//重置所有状态，确保池处于干净的初始状态
		firstBlock_ = nullptr;
		curSlot_ = nullptr;
		freeList_ = nullptr;
		lastSlot_ = nullptr;
	}


	//计算从指针 p 开始，直到下一个符合align对齐的地址所需的填充字节数
	size_t SimpleMemoryPool::padPointer(char* p, size_t align)
	{
		if (align == 0 || align == 1)
			return 0;

		//计算当前指针地址对齐后的余数
		size_t remainder = reinterpret_cast<size_t>(p) % align;

		//如果余数为 0，表示已对齐，无需填充
		if (remainder == 0)
			return 0;

		//返回需要填充的字节数
		return align - remainder;
	}


	// 向系统申请一个新的内存块
	void SimpleMemoryPool::allocateNewBlock()
	{
		//像系统申请一个BlockSize_ 大小的新内存块
		void* newBlockRaw = malloc(BlockSize_);
		if (!newBlockRaw)
		{
			std::cerr << "Error: Failed to allocate new block of size" << BlockSize_ << std::endl;
			// 生产级代码会抛出 std::bad_alloc，这里简化为 exit
			exit(1);
		}


		//将新块添加到 firstBlock_ 链表的头部
		SimpleSlot* blockHeader = static_cast<SimpleSlot*>(newBlockRaw);
		blockHeader->next = firstBlock_;
		firstBlock_ = blockHeader;// 更新链表头部为新块


		//计算实际可用内存的起始位置 (跳过块头部用于链表指针的空间)
		char* blockBody = static_cast<char*>(newBlockRaw) + sizeof(SimpleSlot);

		//计算 blockBody 到下一个 slotSize_ 对齐的填充字节
		size_t paddingSize = padPointer(blockBody, SlotSize_);

		//curSlot_ 指向经过对齐填充后的第一个槽的起始位置
		curSlot_ = blockBody + paddingSize;

		//计算当前块中最后一个槽的结束边界
		lastSlot_ = static_cast<char*>(newBlockRaw) + BlockSize_;
	}


	void* SimpleMemoryPool::allocate()
	{
		// 确保 init 方法已被调用，SlotSize_ 已设置
		if (SlotSize_ == 0) {
			std::cerr << "Error: SimpleMemoryPool::init() not called or slotSize is 0! Exiting." << std::endl;
			exit(1);
		}


		//优先从空闲链表获取已回收的槽
		if (freeList_ != nullptr)
		{
			SimpleSlot* slot = popFreeList();
			return slot;
		}


		//空闲链表为空，从当前大内存块中切割新槽
		//检查 curSlot_ 是否已初始化或当前块是否还有空间
		// 只有当 curSlot_ 为 nullptr (首次分配) 或当前块已用尽时，才需要 allocateNewBlock
		if (curSlot_ == nullptr || (curSlot_ + SlotSize_) > lastSlot_)
		{
			allocateNewBlock();
		}


		//此时 curSlot_ 肯定指向新块的起始，或者之前有剩余空间
		//这行判断 (curSlot_ + SlotSize_) <= lastSlot_ 仍然是必要的，以处理 allocateNewBlock() 刚返回的情况
		if ((curSlot_ + SlotSize_) <= lastSlot_)
		{
			void* slot = curSlot_;
			curSlot_ += SlotSize_;
			return slot;
		}
		else
		{
			// 这表示 allocateNewBlock() 分配的块即使是新的，也无法容纳一个槽 (BlockSize_ < SlotSize_)
			std::cerr << "Error: Fresh block allocated but cannot fit a slot! Check BlockSize/SlotSize. Exiting." << std::endl;
			exit(1);
		}

	}


	// 释放一个内存槽，将其归还给内存池
	void SimpleMemoryPool::deallocate(void* ptr) {
		if (ptr == nullptr) return;

		// 将释放的内存槽转换回 SimpleSlot* 类型
		SimpleSlot* slot = static_cast<SimpleSlot*>(ptr);

		// 将其添加到空闲链表的头部
		pushFreeList(slot);
	}


	// 将一个槽推入空闲链表头部
	void SimpleMemoryPool::pushFreeList(SimpleSlot* slot) {
		slot->next = freeList_;
		freeList_ = slot;
	}


	// 从空闲链表头部取出一个槽
	SimpleSlot* SimpleMemoryPool::popFreeList() {
		SimpleSlot* head = freeList_;
		if (head != nullptr) {
			freeList_ = head->next;
		}
		return head;
	}
}