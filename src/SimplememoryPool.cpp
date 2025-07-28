#include "../include/HashBucketMemoryPool/SimpleMemoryPool.h"
#include <cstdlib>// malloc, free, exit
#include <iostream>//cerr,cout
#include <cassert>//assert
namespace SimpleMemoryPool
{
	SimpleMemoryPool::SimpleMemoryPool(size_t blockSize, size_t slotSize)
		:BlockSize_(blockSize),
		 // 确保 slotSize_ 至少能容纳 SimpleSlot 且是其倍数 (对于对齐和链表操作)
		 // 如果传入的 slotSize 小于 SimpleSlot 的大小，则至少使用 sizeof(SimpleSlot)
		 SlotSize_((slotSize < sizeof(SimpleSlot)) ? sizeof(SimpleSlot) : slotSize),
		 firstBlock_(nullptr),
		 curSlot_(nullptr),
		 freeList_(nullptr),
		 lastSlot_(nullptr)
	{
		// 确保槽大小是 SimpleSlot 指针大小的倍数 (为了链表操作的对齐)
		// 并且确保 blockSize_ 足够大，能容纳至少一个槽
		assert(SlotSize_ % sizeof(SimpleSlot*) == 0); // 或者其他合适的对齐要求
		assert(BlockSize_ >= SlotSize_); // 至少能装一个槽

		// 初始时分配第一个内存块
		allocateNewBlock();
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
		firstBlock_ = blockHeader;       // 更新链表头部为新块

		//计算实际可用内存的起始位置 (跳过块头部用于链表指针的空间)
		char* blockBody = static_cast<char*>(newBlockRaw) + sizeof(SimpleSlot);

		// 计算 blockBody 到下一个 slotSize_ 对齐的填充字节
		size_t paddingSize = padPointer(blockBody, SlotSize_);

		// curSlot_ 指向经过对齐填充后的第一个槽的起始位置
		curSlot_ = blockBody + paddingSize;

		// 4. 计算当前块中最后一个槽的结束边界
		lastSlot_ = static_cast<char*>(newBlockRaw) + BlockSize_;
	}

	void* SimpleMemoryPool::allocate()
	{
		//优先从空闲链表获取已回收的槽
		if (freeList_ != nullptr)
		{
			SimpleSlot* slot = popFreeList();
			return slot;
		}

		//空闲链表为空，从当前大内存块中切割新槽
		//检查当前块是否还有足够的空间来分配一个槽
		if ((curSlot_ + SlotSize_) <= lastSlot_)
		{
			void* slot = curSlot_;
			curSlot_ += SlotSize_;
			return slot;
		}

		//当前内存块也用尽，需要向系统申请新的内存块
		allocateNewBlock();
		//递归调用 allocate() 自身，在新分配的块中获取第一个槽
		return allocate();
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