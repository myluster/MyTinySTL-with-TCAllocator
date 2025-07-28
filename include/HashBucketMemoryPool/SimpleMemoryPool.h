#pragma once

#include <cstddef>//size_t
#include <new>//new delete 

//�򵥵ĵ����ڴ��
namespace SimpleMemoryPool
{
	//�۽ṹ�壺����ڵ�
	//Ϊʲôѡ��struct������union����Ϊ�˼��ݺ����Ĳ�����̣��ڵ��߳�����union���ã������ڴ渴��
	struct SimpleSlot
	{
		SimpleSlot* next;
	};

	class SimpleMemoryPool {
	public:
		// ���캯����ָ��ÿ����ϵͳ������ڴ���С
		SimpleMemoryPool(size_t blockSize = 4096, size_t slotSize = sizeof(SimpleSlot));
		// �����������ͷ������ѷ���Ĵ��ڴ��
		~SimpleMemoryPool();

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