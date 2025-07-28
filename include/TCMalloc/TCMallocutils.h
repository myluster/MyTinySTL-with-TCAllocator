#pragma once
#include "../span.h"
#include "../bitset.h"
#include "../byte.h"
namespace mystl
{
	//静态提供各种常量和内存计算方法
	class size_utils
	{
	public:
		//内存的最小对齐单位
		static constexpr size_t ALIGNMENT = sizeof(void*);//机器字长
		//一个内存页的标准大小4K
		static constexpr size_t PAGE_SIZE = 4096;
		//区分大小内存的阈值
		static constexpr size_t MAX_CACHED_UNIT_SIZE = 16 * 1024;
		//小对象最多能容纳多少个对齐单元
		static constexpr size_t CACHE_LINE_SIZE = MAX_CACHED_UNIT_SIZE / ALIGNMENT;

		//字节数向上对齐
		static size_t align(const size_t memory_size, const size_t alignment=ALIGNMENT)
		{
			return (memory_size + alignment - 1) & ~(alignment - 1);
		}

		//0-base索引
		static size_t get_index(const size_t memory_size)
		{
			return align(memory_size) / ALIGNMENT - 1;
		}
	};


	//管理从PageCache中分配下来的内存（跟踪到TreadCache和CentralCache）
	class page_span
	{
	public:
		//一个内存页最多能切多少块
		static constexpr size_t MAX_UNIT_COUNT = size_utils::PAGE_SIZE / size_utils::ALIGNMENT;
		//初始化page_span
		page_span(const mystl::span<mystl::byte> span, const size_t unit_size) :m_memory(span), m_unit_size(unit_size) {};

		//当前页面是不是全部没有被分配
		bool is_empty()
		{
			return m_allocated_map.none();
		}

		//从页面申请内存
		void allocate(mystl::span<mystl::byte> memory);

		//把这一块内存归还给页面
		void deallocate(mystl::span<mystl::byte> memory);

		//判断某一段空间是不是被这个page_span所管理的
		bool is_valid_unit_span(mystl::span<mystl::byte> memory);

		//管理的内存长度
		inline size_t size()
		{
			return m_memory.size();
		}

		//起始地址
		mystl::byte* data()
		{
			return m_memory.data();
		}

		//维护的长度
		size_t unit_size()
		{
			return m_unit_size;
		}

		//获得这个所维护的地址
		mystl::span<mystl::byte> get_memory_span()
		{
			return m_memory;
		}

	private:
		//这个page_span管理的空间大小
		const mystl::span<mystl::byte> m_memory;
		//一个分配单位的大小
		const size_t m_unit_size;
		//用于管理目前页面的分配情况
		mystl::bitset<MAX_UNIT_COUNT> m_allocated_map;
	};
}