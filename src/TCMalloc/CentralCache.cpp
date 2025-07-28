#include "../../include/TCMalloc/CentralCache.h"
#include "../../include/TCMalloc/PageCache.h"
#include "../../include/TCMalloc/TCMallocBootstrap.h"
#include "../../include/list.h"
#include "../../include/map.h"
#include <iostream>

namespace mystl
{
	class CentralCacheImpl
	{
	public:
		// 把所有私有数据成员都移到这里
		list<span<byte>> m_free_array[size_utils::CACHE_LINE_SIZE];
		std::atomic_flag m_status[size_utils::CACHE_LINE_SIZE];
		map<byte*, page_span> m_page_set[size_utils::CACHE_LINE_SIZE];
	};
	//以后访问成员时，通过 pimpl->

	central_cache::central_cache(): pimpl(nullptr)
	{
		// 初始化 m_status
		g_is_allocator_constructing = true;
		pimpl = new CentralCacheImpl();
		for (size_t i = 0; i < size_utils::CACHE_LINE_SIZE; ++i) {
			pimpl->m_status[i].clear(std::memory_order_release);
		}
		g_is_allocator_constructing = false;
	}

	central_cache::~central_cache()
	{
		delete pimpl;
	}



	std::optional<list<span<byte>>> central_cache::allocate(const size_t memory_size, const size_t block_count)
	{
		//对齐检测
		assert(memory_size % 8 == 0);
		//一次性申请的空间值可以小于512
		assert(block_count <= page_span::MAX_UNIT_COUNT);

		if (memory_size == 0 || block_count == 0)
		{
			return std::nullopt;
		}

		if (memory_size > size_utils::MAX_CACHED_UNIT_SIZE)
		{
			auto memory_opt = page_cache::allocate_unit(memory_size);
			if (memory_opt) {
				return list<span<byte>>{ *memory_opt };
			}
			else {
				return std::nullopt;
			}
		}

		const size_t index = size_utils::get_index(memory_size);

		//自旋锁
		auto& flag = pimpl->m_status[index];
		while (flag.test_and_set(std::memory_order_acquire))
		{
			std::this_thread::yield();
		}

		try
		{
			if (pimpl->m_free_array[index].size() < block_count)
			{
				//如果当前缓存的个数小于申请的块数，则向页分配器申请
				size_t allocate_unit_count = page_span::MAX_UNIT_COUNT;
				size_t allocate_page_count = size_utils::align(memory_size * allocate_unit_count, size_utils::PAGE_SIZE) / size_utils::PAGE_SIZE;
				auto ret = get_page_from_page_cache(allocate_page_count);
				if (!ret.has_value())
				{
					pimpl->m_status[index].clear(std::memory_order_release);
					return std::nullopt;
				}
				span<byte> memory = ret.value();

				//用于管理这个页面
				page_span page_span(memory, memory_size);
				for (size_t i = 0; i < allocate_unit_count; i++)
				{
					span<byte> split_memory = memory.subspan(0, memory_size);
					memory = memory.subspan(memory_size);
					pimpl->m_free_array[index].push_back(split_memory);
				}

				auto start_addr = page_span.data();
				auto [_, succeed] = pimpl->m_page_set[index].emplace(start_addr, move(page_span));
				assert(succeed == true);

			}
		}
		catch(...)
		{
			std::cout << "\n>>> DEBUG: Exception caught in central_cache::allocate! Size: " << memory_size << ", Count: " << block_count << std::endl;
			pimpl->m_status[index].clear(std::memory_order_release);
			throw std::runtime_error("Memory allocation failed");
		}

		list<span<byte>> result;
		for (size_t i = 0; i < block_count; i++) {
			span<byte> memory = pimpl->m_free_array[index].front();
			pimpl->m_free_array[index].pop_front();
			record_allocate_memory_span(memory); // 记录分配
			result.push_back(memory);
		}

		pimpl->m_status[index].clear(std::memory_order_release);
		return result;
	}

	void central_cache::deallocate(const list<span<byte>> memories)
	{
		if (memories.empty())
		{
			return;
		}
		if (memories.begin()->size() > size_utils::MAX_CACHED_UNIT_SIZE)
		{
			//超大内存块，直接返回给page_cache管理
			page_cache::get_instance().deallocate_unit(*memories.begin());
			return;
		}
		for (auto it = memories.begin(); it != memories.end(); it++)
		{
			assert(it->size() == memories.begin()->size());
		}

		const size_t index = size_utils::get_index(memories.begin()->size());
		auto& flag = pimpl->m_status[index];
		while (flag.test_and_set(std::memory_order_acquire)) {
			std::this_thread::yield();
		}

		for (const auto& memory : memories) {
			// 先归还到数组中
			assert((index + 1) * 8 == memory.size());
			pimpl->m_free_array[index].push_back(memory);
			// 然后再还给页面管理器中
			auto memory_data = memory.data();
			auto it = pimpl->m_page_set[index].upper_bound(memory_data);
			assert(it != pimpl->m_page_set[index].begin());
			--it;
			it->second.deallocate(memory);
			// 同时判断需不需要返回给页面管理器
			if (it->second.is_empty()) {
				// 如果已经还清内存了，则将这块内存还给页面管理器(page_cache)
				auto page_start_addr = it->second.data();
				auto page_end_addr = page_start_addr + it->second.size();
				assert(it->second.unit_size() == memory.size());
				auto mem_iter = pimpl->m_free_array[index].begin();
				// 遍历这个数组
				while (mem_iter != pimpl->m_free_array[index].end()) {
					auto memory_start_addr = mem_iter->data();
					auto memory_end_addr = memory_start_addr + mem_iter->size();
					if (memory_start_addr >= page_start_addr && memory_end_addr <= page_end_addr) {
						// 如果这个内存在这个范围内，则说明是正确的
						// 一定是满足要求的，如果不满足，则说明代码写错了
						assert(it->second.is_valid_unit_span(*mem_iter));
						// 指向下一个
						mem_iter = pimpl->m_free_array[index].erase(mem_iter);
					}
					else {
						++mem_iter;
					}
				}
				span<byte> page_memory = it->second.get_memory_span();
				pimpl->m_page_set[index].erase(it);
				page_cache::get_instance().deallocate_page(page_memory);
			}
		}
		pimpl->m_status[index].clear(std::memory_order_release);
	}

	void central_cache::record_allocate_memory_span(span<byte> memory)
	{
		const size_t index = size_utils::get_index(memory.size());
		auto it = pimpl->m_page_set[index].upper_bound(memory.data());
		assert(it != pimpl->m_page_set[index].begin());
		--it;
		it->second.allocate(memory);
	}


	std::optional<span<byte>> central_cache::get_page_from_page_cache(size_t page_allocate_count)
	{
		return page_cache::get_instance().allocate_page(page_allocate_count);
	}

	std::optional<span<byte>> central_cache::allocate_single(size_t memory_size)
	{
		// 对齐检测
		assert(memory_size % 8 == 0);
		if (memory_size == 0)
		{
			return std::nullopt;
		}

		// 对于超大内存，直接走 page_cache (虽然在当前场景不太可能)
		if (memory_size > size_utils::MAX_CACHED_UNIT_SIZE)
		{
			return page_cache::get_instance().allocate_unit(memory_size);
		}

		const size_t index = size_utils::get_index(memory_size);
		std::optional<span<byte>> result = std::nullopt;

		// --- 上锁 ---
		auto& flag = pimpl->m_status[index];
		while (flag.test_and_set(std::memory_order_acquire))
		{
			std::this_thread::yield();
		}

		try
		{
			// 1. 优先从现有自由链表获取
			if (!pimpl->m_free_array[index].empty())
			{
				result = pimpl->m_free_array[index].front();
				pimpl->m_free_array[index].pop_front();
				record_allocate_memory_span(*result);
			}
			// 2. 如果自由链表为空，则从 PageCache 获取新页
			else
			{
				size_t allocate_unit_count = page_span::MAX_UNIT_COUNT;
				size_t allocate_page_count = size_utils::align(memory_size * allocate_unit_count, size_utils::PAGE_SIZE) / size_utils::PAGE_SIZE;
				auto ret = get_page_from_page_cache(allocate_page_count);
				if (ret.has_value())
				{
					span<byte> memory = ret.value();

					// 创建 page_span 来管理新页
					page_span page_span(memory, memory_size);

					// 分割内存页：一个返回，其余的放入自由链表
					// 返回第一个块
					result = memory.subspan(0, memory_size);
					page_span.allocate(*result);
					memory = memory.subspan(memory_size);

					// 将 page_span 记录下来
					auto start_addr = page_span.data();
					pimpl->m_page_set[index].emplace(start_addr, move(page_span));

					// 剩余的块放入自由链表
					allocate_unit_count -= 1;
					for (size_t i = 0; i < allocate_unit_count; i++)
					{
						span<byte> split_memory = memory.subspan(0, memory_size);
						memory = memory.subspan(memory_size);
						pimpl->m_free_array[index].push_back(split_memory);
					}
				}
			}
		}
		catch (...)
		{
			std::cout << "\n>>> DEBUG: Exception caught in central_cache::allocate_single! Size: " << memory_size << std::endl;
			pimpl->m_status[index].clear(std::memory_order_release);
			throw std::runtime_error("Single memory allocation failed");
		}

		// --- 解锁 ---
		pimpl->m_status[index].clear(std::memory_order_release);
		return result;
	}
}