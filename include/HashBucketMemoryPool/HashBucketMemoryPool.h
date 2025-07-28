#pragma once

#include <cstddef>//size_t
#include <new>//new bad_alloc
#include <utility>//forward
#include <iostream>//cerr,cout
namespace HashBucketMemoryPool
{
	enum class HashBucketConstants
	{
		SLOT_BASE_SIZE = 8,
		MAX_SLOT_SIZE = 512,
		MEMORY_POOL_NUM = MAX_SLOT_SIZE / SLOT_BASE_SIZE,
		BLOCK_SIZE_DEFAULT = 4096//Ĭ�ϵĿ��С
	};

	class SimpleMemoryPool;//ǰ������

	class HashBucket
	{
	public:
		static void initMemoryPools();
		static void* allocate(size_t size);
		static void deallocate(void* ptr, size_t size);

		template<typename T,typename... Args>
		friend T* newElement(Args&&... args);

		template<typename T>
		friend void deleteElement(T* p);

	private:
		//���鱾���Ǿ�̬�ģ�ָ��Ĭ�ϳ�ʼ��Ϊ nullptr
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


	//������֮ǰ��SimpleMemoryPoool�����������и��ģ���init�������
	//�۽ṹ�壺����ڵ�
	//Ϊʲôѡ��struct������union����Ϊ�˼��ݺ����Ĳ�����̣��ڵ��߳�����union���ã������ڴ渴��
	struct SimpleSlot
	{
		SimpleSlot* next;
	};

	class SimpleMemoryPool {
	public:

		// ���캯����ָ��ÿ����ϵͳ������ڴ���С
		SimpleMemoryPool(size_t blockSize = 4096);
		// �����������ͷ������ѷ���Ĵ��ڴ��
		~SimpleMemoryPool();

		//�������׶γ�ʼ������ƣ����ò۴�С��׼���ڴ��
		void init(size_t slotSize);

		// ���ڴ�ط���һ���ڴ�ۣ����� void* ָ��
		void* allocate();

		// ��һ���ڴ�۹黹���ڴ��
		void deallocate(void*);

	private:
		//��ϵͳ����һ���µĴ��ڴ�飬��׼�������и�ɲ�
		void allocateNewBlock();

		// p ���ڴ����ʼ��ַ��align ��������һ����ַ����С���루������ֵ��
		// ȷ�����¿����и���Ĳ۵�ַ�Ƕ����
		size_t padPointer(char* p, size_t align);

		// ��һ���������������ͷ��
		void pushFreeList(SimpleSlot* slot);
		// �ӿ�������ͷ��ȡ��һ����
		SimpleSlot* popFreeList();

	private:
		size_t                 BlockSize_;		// �ڴ���С
		size_t                 SlotSize_;		// �۴�С

		SimpleSlot* firstBlock_;				// ָ���ڴ�ع�����׸�ʵ���ڴ��
		char* curSlot_;							// ָ��ǰ����ʹ�õĴ��ڴ���У���һ����ֱ�ӷ����ȥ���²۵���ʼλ��
		SimpleSlot* freeList_;					// ָ����в������ͷ������ű�ʹ�ù����ֱ��ͷŵĲۣ�
		char* lastSlot_;						// ��Ϊ��ǰ�ڴ��������ܹ����Ԫ�ص�λ�ñ�ʶ(������λ���������µ��ڴ��)
	};

}