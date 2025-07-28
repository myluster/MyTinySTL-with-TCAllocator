#include "../../include/TCMalloc/TCMallocutils.h"
#include "../../include/byte.h"
#include <cassert>
namespace mystl
{
	void page_span::allocate(mystl::span<mystl::byte> memory)
	{
		//�ж�����ռ��ǲ��Ǳ������
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
		//�黹�Ŀռ��С�����ҳ�����Ĵ�С��һ��
		if (memory.size() != m_unit_size)
			return false;
		//��ʼ��ַС���˹���ĵ�ַ
		if (memory.data() < m_memory.data())
			return false;
		//�����ַƫ����
		uint64_t address_offset = memory.data() - m_memory.data();
		//�ڴ�û����
		if (address_offset % m_unit_size != 0)
			return false;
		//�ԱȽ����ĵ�ַ�ǲ���С�ڻ���ڹ�������Ľ�����ַ
		return address_offset + m_unit_size <= m_memory.size();
	}
}