#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace MutexMemoryPool
{
	enum class HashBucketConstants
	{
		SLOT_BASE_SIZE = 8,
		MAX_SLOT_SIZE = 512,
		MEMORY_POOL_NUM = MAX_SLOT_SIZE / SLOT_BASE_SIZE,
		BLOCK_SIZE_DEFAULT = 4096//默认的块大小
	};

	struct SimpleSlot
	{
		SimpleSlot* next;
	};

	class SimpleMemoryPool {
	public:

		SimpleMemoryPool(size_t blockSize = 4096);

		~SimpleMemoryPool();

		void init(size_t slotSize);

		void* allocate();

		void deallocate(void*);

	private:

		void allocateNewBlock();

		size_t padPointer(char* p, size_t align);

		void pushFreeList(SimpleSlot* slot);

		SimpleSlot* popFreeList();

	private:
		size_t				BlockSize_;
		size_t				SlotSize_;
		SimpleSlot*			firstBlock_;
		char*				curSlot_;
		SimpleSlot*			freeList_;
		char*				lastSlot_;
		std::mutex			mutexForFreeList_;//保证freeList_在多线程中操作的原子性
		std::mutex			mutexForBlock_;//保证多线程情况下避免不必要的重复开辟内存导致的浪费行为
	};

	class HashBucket
	{
	public:
		static void initMemoryPools();
		static void* allocate(size_t size);
		static void deallocate(void* ptr, size_t size);

		template<typename T, typename... Args>
		friend T* newElement(Args&&... args);

		template<typename T>
		friend void deleteElement(T* p);

	private:
		//数组本身是静态的，指针默认初始化为 nullptr
		static SimpleMemoryPool* _s_pool_instances[static_cast<size_t>(HashBucketConstants::MEMORY_POOL_NUM)];

		static SimpleMemoryPool& getPoolInstance(size_t index);

		static size_t getPoolIndex(size_t size);

		struct PoolCleaner
		{
			~PoolCleaner();
		};

		static PoolCleaner _s_pool_cleaner;
	};


	template<typename T, typename... Args>
	T* newElement(Args&&... args) {
		T* p_object = nullptr;
		try {
			void* p_raw_memory = HashBucket::allocate(sizeof(T));
			if (p_raw_memory == nullptr) {
				throw std::bad_alloc();
			}
			p_object = new(p_raw_memory) T(std::forward<Args>(args)...);
		}
		catch (const std::bad_alloc& e) {
			std::cerr << "Error: newElement failed to allocate memory: " << e.what() << std::endl;
			return nullptr;
		}
		catch (const std::runtime_error& e) {
			std::cerr << "Error: newElement runtime error: " << e.what() << std::endl;
			return nullptr;
		}
		catch (...) {
			std::cerr << "Error: newElement an unknown error occurred during allocation or construction." << std::endl;
			return nullptr;
		}
		return p_object;
	}

	template<typename T>
	void deleteElement(T* p) {
		if (p == nullptr) return;
		p->~T();
		HashBucket::deallocate(static_cast<void*>(p), sizeof(T));
	}

}