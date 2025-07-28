#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace LockFreeMemoryPool
{
	enum class HashBucketConstants
	{
		SLOT_BASE_SIZE = 8,
		MAX_SLOT_SIZE = 512,
		MEMORY_POOL_NUM = MAX_SLOT_SIZE / SLOT_BASE_SIZE,
		BLOCK_SIZE_DEFAULT = 4096//Ĭ�ϵĿ��С
	};

	struct SimpleSlot
	{
		SimpleSlot* next;
	};

	struct TagAtomicSlot
	{
		SimpleSlot* ptr;
		unsigned int tag;
		// ����ֽڣ�ȷ�� TagAtomicSlot �ܴ�С�� 16 �ֽ� (�� 64 λϵͳ��)
		// 64λϵͳ: ptr (8�ֽ�) + tag (4�ֽ�) + 4�ֽ���� = 16�ֽ�
		char padding[4];

		bool operator==(const TagAtomicSlot& other)const
		{
			return ptr == other.ptr && tag == other.tag;
		}
		bool operator!=(const TagAtomicSlot& other)const
		{
			return !(*this == other);
		}
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

		//ʹ��CAS��������������Ӻͳ���
		bool pushFreeList(SimpleSlot* slot);

		SimpleSlot* popFreeList();

	private:
		size_t						BlockSize_;
		size_t						SlotSize_;
		SimpleSlot*					firstBlock_;
		char* curSlot_;
		std::atomic<TagAtomicSlot>	freeList_;//�ڴ�ԭ�ӻ�
		char* lastSlot_;
		std::mutex					mutexForBlock_;
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
			//std::cerr << "Error: newElement failed to allocate memory: " << e.what() << std::endl;
			return nullptr;
		}
		catch (const std::runtime_error& e) {
			//std::cerr << "Error: newElement runtime error: " << e.what() << std::endl;
			return nullptr;
		}
		catch (...) {
			//std::cerr << "Error: newElement an unknown error occurred during allocation or construction." << std::endl;
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
