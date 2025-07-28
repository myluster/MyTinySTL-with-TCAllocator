#pragma once
#include "../span.h"
#include "../bitset.h"
#include "../byte.h"
namespace mystl
{
	//��̬�ṩ���ֳ������ڴ���㷽��
	class size_utils
	{
	public:
		//�ڴ����С���뵥λ
		static constexpr size_t ALIGNMENT = sizeof(void*);//�����ֳ�
		//һ���ڴ�ҳ�ı�׼��С4K
		static constexpr size_t PAGE_SIZE = 4096;
		//���ִ�С�ڴ����ֵ
		static constexpr size_t MAX_CACHED_UNIT_SIZE = 16 * 1024;
		//С������������ɶ��ٸ����뵥Ԫ
		static constexpr size_t CACHE_LINE_SIZE = MAX_CACHED_UNIT_SIZE / ALIGNMENT;

		//�ֽ������϶���
		static size_t align(const size_t memory_size, const size_t alignment=ALIGNMENT)
		{
			return (memory_size + alignment - 1) & ~(alignment - 1);
		}

		//0-base����
		static size_t get_index(const size_t memory_size)
		{
			return align(memory_size) / ALIGNMENT - 1;
		}
	};


	//�����PageCache�з����������ڴ棨���ٵ�TreadCache��CentralCache��
	class page_span
	{
	public:
		//һ���ڴ�ҳ������ж��ٿ�
		static constexpr size_t MAX_UNIT_COUNT = size_utils::PAGE_SIZE / size_utils::ALIGNMENT;
		//��ʼ��page_span
		page_span(const mystl::span<mystl::byte> span, const size_t unit_size) :m_memory(span), m_unit_size(unit_size) {};

		//��ǰҳ���ǲ���ȫ��û�б�����
		bool is_empty()
		{
			return m_allocated_map.none();
		}

		//��ҳ�������ڴ�
		void allocate(mystl::span<mystl::byte> memory);

		//����һ���ڴ�黹��ҳ��
		void deallocate(mystl::span<mystl::byte> memory);

		//�ж�ĳһ�οռ��ǲ��Ǳ����page_span�������
		bool is_valid_unit_span(mystl::span<mystl::byte> memory);

		//������ڴ泤��
		inline size_t size()
		{
			return m_memory.size();
		}

		//��ʼ��ַ
		mystl::byte* data()
		{
			return m_memory.data();
		}

		//ά���ĳ���
		size_t unit_size()
		{
			return m_unit_size;
		}

		//��������ά���ĵ�ַ
		mystl::span<mystl::byte> get_memory_span()
		{
			return m_memory;
		}

	private:
		//���page_span����Ŀռ��С
		const mystl::span<mystl::byte> m_memory;
		//һ�����䵥λ�Ĵ�С
		const size_t m_unit_size;
		//���ڹ���Ŀǰҳ��ķ������
		mystl::bitset<MAX_UNIT_COUNT> m_allocated_map;
	};
}