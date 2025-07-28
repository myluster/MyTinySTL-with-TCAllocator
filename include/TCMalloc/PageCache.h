#pragma once
#include <atomic>
//#include "../map.h"
#include "../span.h"
//#include "../set.h"
//#include "../vector.h"
#include "TCMallocutils.h"
#include <optional>
#include <mutex>
namespace mystl
{
	//PIMPLʵ�����ǰ������
	class PageCacheImpl;

	class page_cache
	{
	public:
		static constexpr size_t PAGE_ALLOCATE_COUNT = 2048;
		static page_cache& get_instance() {
			static page_cache instance;
			return instance;
		}

		// ����ָ��ҳ�����ڴ�
		// �����������ҳ��
		std::optional<span<byte>> allocate_page(size_t page_count);

		// ����ָ��ҳ�����ڴ�
		void deallocate_page(span<byte> page);

		static std::optional<span<byte>> allocate_unit(size_t memory_size);

		// ����һ����Ԫ���ڴ棬���ڻ��ճ�����ڴ�
		static void deallocate_unit(span<byte> memories);

		// �ر��ڴ��
		void stop();

		~page_cache();

	private:
		page_cache();

		// ֻ���룬�����գ�ֻ��������ʱ����
		std::optional<span<byte>> system_allocate_memory(size_t page_count);

		/// �����ڴ棬ֻ�������������е���
		void system_deallocate_memory(span<byte> page);

		/*
		mystl::map<size_t, mystl::set<span<byte>>> free_page_store;
		mystl::map<byte*, span<byte>> free_page_map;
		// ���ڻ���ʱ munmap
		mystl::vector<span<byte>> page_vector;
		*/

		PageCacheImpl* pimpl;

		// ��ʾ��ǰ���ڴ���ǲ����Ѿ��ر���
		bool m_stop = false;
		// ��������
		std::mutex m_mutex;
	};

}