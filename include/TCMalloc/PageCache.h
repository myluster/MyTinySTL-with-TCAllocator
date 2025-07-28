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
	//PIMPL实现类的前向声明
	class PageCacheImpl;

	class page_cache
	{
	public:
		static constexpr size_t PAGE_ALLOCATE_COUNT = 2048;
		static page_cache& get_instance() {
			static page_cache instance;
			return instance;
		}

		// 申请指定页数的内存
		// 参数：申请的页数
		std::optional<span<byte>> allocate_page(size_t page_count);

		// 回收指定页数的内存
		void deallocate_page(span<byte> page);

		static std::optional<span<byte>> allocate_unit(size_t memory_size);

		// 回收一个单元的内存，用于回收超大块内存
		static void deallocate_unit(span<byte> memories);

		// 关闭内存池
		void stop();

		~page_cache();

	private:
		page_cache();

		// 只申请，不回收，只有在销毁时回收
		std::optional<span<byte>> system_allocate_memory(size_t page_count);

		/// 回收内存，只有在析构函数中调用
		void system_deallocate_memory(span<byte> page);

		/*
		mystl::map<size_t, mystl::set<span<byte>>> free_page_store;
		mystl::map<byte*, span<byte>> free_page_map;
		// 用于回收时 munmap
		mystl::vector<span<byte>> page_vector;
		*/

		PageCacheImpl* pimpl;

		// 表示当前的内存池是不是已经关闭了
		bool m_stop = false;
		// 并发控制
		std::mutex m_mutex;
	};

}