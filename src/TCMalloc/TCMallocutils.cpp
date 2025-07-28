#include "../../include/TCMalloc/TCMallocutils.h"
#include "../../include/byte.h"
#include <cassert>
namespace mystl
{
	void page_span::allocate(mystl::span<mystl::byte> memory)
	{
		//判断这个空间是不是被管理的
		assert(is_valid_unit_span(memory));
		uint64_t address_offset = memory.data() - m_memory.data();
		uint64_t index = address_offset / m_unit_size;
		assert(m_allocated_map[index] == 0);
		m_allocated_map[index] = true;
	}

	void page_span::deallocate(mystl::span<mystl::byte> memory)
	{
		assert(is_valid_unit_span(memory));
		uint64_t address_offset = memory.data() - m_memory.data();
		uint64_t index = address_offset / m_unit_size;
		assert(m_allocated_map[index] == 1);
		m_allocated_map[index] = false;
	}

	bool page_span::is_valid_unit_span(mystl::span<mystl::byte> memory)
	{
		//归还的空间大小与这个页面管理的大小不一样
		if (memory.size() != m_unit_size)
			return false;
		//起始地址小于了管理的地址
		if (memory.data() < m_memory.data())
			return false;
		//计算地址偏移量
		uint64_t address_offset = memory.data() - m_memory.data();
		//内存没对齐
		if (address_offset % m_unit_size != 0)
			return false;
		//对比结束的地址是不是小于或等于管理区域的结束地址
		return address_offset + m_unit_size <= m_memory.size();
	}
}