#include "../include/HashBucketMemoryPool/LockFreeMemoryPool.h"
#include <cstdlib>//malloc,free,exit
#include <iostream>//cerr
#include <cassert>//assert
#include <mutex>//once_flag,call_once

namespace LockFreeMemoryPool
{
	SimpleMemoryPool* HashBucket::_s_pool_instances[static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM)] = {};

	static std::once_flag hashbucket_init_flag;

	SimpleMemoryPool& HashBucket::getPoolInstance(size_t index)
	{
		assert(_s_pool_instances[index] != nullptr && "Error: HashBucket::initMemoryPools() not called or pool not initialized for this index!");
		return *_s_pool_instances[index];
	}

	size_t HashBucket::getPoolIndex(size_t size)
	{
		return (size + static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL) / static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE) - 1ULL;
	}

	void HashBucket::initMemoryPools()
	{
		std::call_once(hashbucket_init_flag, []() {
			for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); ++i)
			{
				size_t current_slot_size = (i + 1) * static_cast<size_t>(HashBucketConstants::SLOT_BASE_SIZE);
				_s_pool_instances[i] = new SimpleMemoryPool(static_cast<size_t>(HashBucketConstants::BLOCK_SIZE_DEFAULT));

				_s_pool_instances[i]->init(current_slot_size);
			}
			std::cout << "HashBucket: All SimpleMemoryPools initialized." << std::endl;
			});
	}

	void* HashBucket::allocate(size_t size)
	{
		if (_s_pool_instances[0] == nullptr) {
			//std::cerr << "Error: HashBucket::initMemoryPools() not called before allocation! Exiting." << std::endl;
			throw std::bad_alloc();
		}

		if (size <= 0)
			return nullptr;

		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			return malloc(size);
		}

		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		return getPoolInstance(static_cast<int>(index)).allocate();
	}

	void HashBucket::deallocate(void* ptr, size_t size)
	{
		if (_s_pool_instances[0] == nullptr) {
			//std::cerr << "Error: HashBucket::initMemoryPools() not called before deallocation!" << std::endl;
			return;
		}

		if (ptr == nullptr)
			return;

		if (size > static_cast<size_t>(HashBucketConstants::MAX_SLOT_SIZE))
		{
			free(ptr);
			return;
		}

		size_t index = getPoolIndex(size);
		assert(index < static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM));

		getPoolInstance(static_cast<int>(index)).deallocate(ptr);
	}

	HashBucket::PoolCleaner HashBucket::_s_pool_cleaner;

	HashBucket::PoolCleaner::~PoolCleaner()
	{
		for (int i = 0; i < static_cast<int>(HashBucketConstants::MEMORY_POOL_NUM); i++)
		{
			if (_s_pool_instances[i] != nullptr)
			{
				delete _s_pool_instances[i];
				_s_pool_instances[i] = nullptr;
			}
		}
	}


	SimpleMemoryPool::SimpleMemoryPool(size_t blockSize)
		:BlockSize_(blockSize),
		SlotSize_(0),
		firstBlock_(nullptr),
		curSlot_(nullptr),
		freeList_({ nullptr, 0 }),
		lastSlot_(nullptr),
		mutexForBlock_()
	{
	}


	SimpleMemoryPool::~SimpleMemoryPool()
	{
		SimpleSlot* currentBlock = firstBlock_;
		while (currentBlock)
		{
			SimpleSlot* nextBlock = currentBlock->next;
			free(static_cast<void*>(currentBlock));
			currentBlock = nextBlock;
		}
	}


	void SimpleMemoryPool::init(size_t slotSize)
	{
		assert(slotSize > 0);
		SlotSize_ = (slotSize < sizeof(SimpleSlot)) ? sizeof(SimpleSlot) : slotSize;


		assert(SlotSize_ % sizeof(SimpleSlot*) == 0);
		assert(BlockSize_ >= SlotSize_);


		firstBlock_ = nullptr;
		curSlot_ = nullptr;
		freeList_.store({nullptr,0},std::memory_order_relaxed);
		lastSlot_ = nullptr;
	}

	size_t SimpleMemoryPool::padPointer(char* p, size_t align)
	{
		if (align == 0 || align == 1)
			return 0;

		size_t remainder = reinterpret_cast<size_t>(p) % align;

		if (remainder == 0)
			return 0;

		return align - remainder;
	}


	void SimpleMemoryPool::allocateNewBlock()
	{
		void* newBlockRaw = malloc(BlockSize_);
		if (!newBlockRaw)
		{
			//std::cerr << "Error: Failed to allocate new block of size" << BlockSize_ << std::endl;
			throw std::bad_alloc();
		}

		SimpleSlot* blockHeader = static_cast<SimpleSlot*>(newBlockRaw);
		blockHeader->next = firstBlock_;
		firstBlock_ = blockHeader;


		char* blockBody = static_cast<char*>(newBlockRaw) + sizeof(SimpleSlot);

		size_t paddingSize = padPointer(blockBody, SlotSize_);

		curSlot_ = blockBody + paddingSize;

		lastSlot_ = static_cast<char*>(newBlockRaw) + BlockSize_;
	}


	void* SimpleMemoryPool::allocate()
	{
		if (SlotSize_ == 0) {
			//std::cerr << "Error: SimpleMemoryPool::init() not called or slotSize is 0! Exiting." << std::endl;
			throw std::bad_alloc();
		}

		if (freeList_.load(std::memory_order_relaxed).ptr != nullptr)
		{
			SimpleSlot* slot = popFreeList();
			return slot;
		}


		std::lock_guard<std::mutex> lock(mutexForBlock_);
		if (curSlot_ == nullptr || (curSlot_ + SlotSize_) > lastSlot_)
		{
			allocateNewBlock();
		}


		if ((curSlot_ + SlotSize_) <= lastSlot_)
		{
			void* slot = curSlot_;
			curSlot_ += SlotSize_;
			return slot;
		}
		else
		{
			//std::cerr << "Error: Fresh block allocated but cannot fit a slot! Check BlockSize/SlotSize. Exiting." << std::endl;
			throw std::bad_alloc();
		}
	}


	void SimpleMemoryPool::deallocate(void* ptr) {
		if (ptr == nullptr) return;

		SimpleSlot* slot = static_cast<SimpleSlot*>(ptr);

		pushFreeList(slot);
	}


	//实现无锁入队操作
	bool SimpleMemoryPool::pushFreeList(SimpleSlot* slot) {
		TagAtomicSlot oldHead;
		while (true)
		{
			//获取当前头节点
			oldHead = freeList_.load(std::memory_order_relaxed);
			//将新节点的next指向当前头节点
			slot->next=oldHead.ptr;

			//尝试将新节点设置为头节点
			if (freeList_.compare_exchange_weak(oldHead,{slot,oldHead.tag+1}, std::memory_order_release, std::memory_order_relaxed))
			{
				return true;
			}
			//CAS失败则重试
		}
	}

	//实现无锁出队操作
	SimpleSlot* SimpleMemoryPool::popFreeList() {
		TagAtomicSlot oldHead;
		SimpleSlot* result_slot = nullptr;
		while (true)
		{
			oldHead = freeList_.load(std::memory_order_relaxed);
			if (oldHead.ptr== nullptr)
			{
				return nullptr;//队列为空
			}

			// 获取下一个节点 (普通指针)
			SimpleSlot* newHead = oldHead.ptr->next;

			if (freeList_.compare_exchange_weak(oldHead, { newHead, oldHead.tag + 1 },std::memory_order_release, std::memory_order_relaxed)) 
			{
				result_slot = oldHead.ptr;
				break;
			}
		}
		return result_slot;
	}
}