#pragma once
#include <atomic>
#include <optional>
//#include "../list.h"
#include "../span.h"
#include "../byte.h"
//#include "../map.h"
#include "TCMallocutils.h"
namespace mystl
{
	//ǰ������ list����Ϊ allocate() �ķ���ֵ��Ҫ��
	template<typename T> class list;

	//PIMPLʵ�����ǰ������
	class CentralCacheImpl;

	class central_cache
	{
	public:
		//һ��������8ҳ�Ŀռ�
		static constexpr size_t PAGE_SPAN = 8;
		static central_cache& get_instance()
		{
			static central_cache instance;
			return instance;
		}

		//���ڷ���ָ�������ָ���С�Ŀռ�
		//memory_size:Ҫ����Ĵ�С��block_count:����ĸ���
		//����һ����ͬ��С��ָ���������ڴ��
		std::optional<list<span<byte>>> allocate(size_t memory_size, size_t block_count);

		// �����ĺ�����ֻ����һ���ڴ�飬���ڴ��Ƴ�ʼ������
		std::optional<span<byte>> allocate_single(size_t memory_size);

		//�����ڴ��
		//memories:���̻߳�����л��յ��ڴ���Ƭ
		void deallocate(list<span<byte>> memories);

		~central_cache();

	private:
		central_cache();

		//��¼���䵽TreadCache���ڴ��
		void record_allocate_memory_span(span<byte> memory);

		std::optional<span<byte>> get_page_from_page_cache(size_t page_allocate_count);

		/*
		list<span<byte>> m_free_array[size_utils::CACHE_LINE_SIZE];
		std::atomic_flag m_status[size_utils::CACHE_LINE_SIZE];

		map<byte*, page_span> m_page_set[size_utils::CACHE_LINE_SIZE];
		*/

		//ָ���Ա
		CentralCacheImpl* pimpl;

	};
}