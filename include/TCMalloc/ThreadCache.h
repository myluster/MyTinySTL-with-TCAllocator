#pragma once
//#include "../list.h"
//#include "../set.h"
//#include "../unordered_map.h"
#include <optional>
#include "TCMallocutils.h"
#include "../span.h"

namespace mystl
{
	//PIMPLʵ�����ǰ������
	class ThreadCacheImpl;

	class thread_cache
	{
	public:
		static thread_cache& get_instance()
		{
			static thread_local thread_cache instance;
			return instance;
		}

		//���ڴ������һ���ڴ�
		std::optional<void*> allocate(size_t memory_size);

		//���ڴ�ع黹һƬ�ռ�
		void deallocate(void* start_p, size_t memory_size);

		~thread_cache();

	private:
		thread_cache();

		//��CentralCache������һ��ռ�
		std::optional<span<byte>> allocate_from_central_cache(size_t memory_size);
		
		//��̬�����ڴ�
		size_t compute_allocate_count(size_t memory_size);

		/*
		//��������
		list<span<byte>> m_free_cache[size_utils::CACHE_LINE_SIZE];

		//���ڱ�ʾ��һ��������ָ����С���ڴ�ʱ�����뼸���ڴ�
		size_t m_next_allocate_count[size_utils::CACHE_LINE_SIZE];
		*/

		//ָ���Ա
		ThreadCacheImpl* pimpl;

		// ����ÿ���б��������Ϊ256KB������16KB�Ķ���Ϊ���� 256KB / 16KB = 16����
		// �����ֵ��������Ҫ������������õķ�������Ƚ���
		// ����ֻ���뼸���̶���С�Ŀռ䣬�����ֵ�������õĴ�һЩ
		// ��������ڴ�ռ�Ĵ�С�ܸ��ӣ�����Ҫ���õ�СһЩ����Ȼ���ܻ��õ����̵߳Ŀռ�ռ�ù���
		static constexpr size_t MAX_FREE_BYTES_PER_LISTS = 256 * 1024;

	};
}