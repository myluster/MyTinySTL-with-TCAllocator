#pragma once

#include <cstddef>//size_t
#include <new>//new delete 

//简单的单级内存池
namespace SimpleMemoryPool
{
	//槽结构体：链表节点
	//为什么选择struct而不是union：是为了兼容后续的并发编程，在但线程中用union更好，可以内存复用
	struct SimpleSlot
	{
		SimpleSlot* next;
	};

	class SimpleMemoryPool {
	public:
		// 构造函数：指定每次向系统申请的内存块大小
		SimpleMemoryPool(size_t blockSize = 4096, size_t slotSize = sizeof(SimpleSlot));
		// 析构函数：释放所有已分配的大内存块
		~SimpleMemoryPool();

		// 从内存池分配一个内存槽，返回 void* 指针
		void* allocate();

		// 将一个内存槽归还给内存池
		void deallocate(void*);

	private:
		//向系统申请一个新的大内存块，并准备将其切割成槽
		void allocateNewBlock();

		// p 是内存的起始地址，align 是其与下一个地址的最小距离（即对齐值）
		// 确保从新块中切割出的槽地址是对齐的
		size_t padPointer(char* p, size_t align);

		// 将一个槽推入空闲链表头部
		void pushFreeList(SimpleSlot* slot);
		// 从空闲链表头部取出一个槽
		SimpleSlot* popFreeList();

	private:
		size_t                 BlockSize_;		// 内存块大小
		size_t                 SlotSize_;		// 槽大小

		SimpleSlot* firstBlock_;				// 指向内存池管理的首个实际内存块
		char* curSlot_;							// 指向当前正在使用的大内存块中，下一个可直接分配出去的新槽的起始位置
		SimpleSlot* freeList_;					// 指向空闲槽链表的头部（存放被使用过后又被释放的槽）
		char* lastSlot_;						// 作为当前内存块中最后能够存放元素的位置标识(超过该位置需申请新的内存块)
	};
}