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
		// ������˽�����ݳ�Ա���Ƶ�����
		list<span<byte>> m_free_array[size_utils::CACHE_LINE_SIZE];
		std::atomic_flag m_status[size_utils::CACHE_LINE_SIZE];
		map<byte*, page_span> m_page_set[size_utils::CACHE_LINE_SIZE];
	};
	//�Ժ���ʳ�Աʱ��ͨ�� pimpl->

	central_cache::central_cache(): pimpl(nullptr)
	{
		// ��ʼ�� m_status
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
		//������
		assert(memory_size % 8 == 0);
		//һ��������Ŀռ�ֵ����С��512
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

		//������
		auto& flag = pimpl->m_status[index];
		while (flag.test_and_set(std::memory_order_acquire))
		{
			std::this_thread::yield();
		}

		try
		{
			if (pimpl->m_free_array[index].size() < block_count)
			{
				//�����ǰ����ĸ���С������Ŀ���������ҳ����������
				size_t allocate_unit_count = page_span::MAX_UNIT_COUNT;
				size_t allocate_page_count = size_utils::align(memory_size * allocate_unit_count, size_utils::PAGE_SIZE) / size_utils::PAGE_SIZE;
				auto ret = get_page_from_page_cache(allocate_page_count);
				if (!ret.has_value())
				{
					pimpl->m_status[index].clear(std::memory_order_release);
					return std::nullopt;
				}
				span<byte> memory = ret.value();

				//���ڹ������ҳ��
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
			record_allocate_memory_span(memory); // ��¼����
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
			//�����ڴ�飬ֱ�ӷ��ظ�page_cache����
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
			// �ȹ黹��������
			assert((index + 1) * 8 == memory.size());
			pimpl->m_free_array[index].push_back(memory);
			// Ȼ���ٻ���ҳ���������
			auto memory_data = memory.data();
			auto it = pimpl->m_page_set[index].upper_bound(memory_data);
			assert(it != pimpl->m_page_set[index].begin());
			--it;
			it->second.deallocate(memory);
			// ͬʱ�ж��費��Ҫ���ظ�ҳ�������
			if (it->second.is_empty()) {
				// ����Ѿ������ڴ��ˣ�������ڴ滹��ҳ�������(page_cache)
				auto page_start_addr = it->second.data();
				auto page_end_addr = page_start_addr + it->second.size();
				assert(it->second.unit_size() == memory.size());
				auto mem_iter = pimpl->m_free_array[index].begin();
				// �����������
				while (mem_iter != pimpl->m_free_array[index].end()) {
					auto memory_start_addr = mem_iter->data();
					auto memory_end_addr = memory_start_addr + mem_iter->size();
					if (memory_start_addr >= page_start_addr && memory_end_addr <= page_end_addr) {
						// �������ڴ��������Χ�ڣ���˵������ȷ��
						// һ��������Ҫ��ģ���������㣬��˵������д����
						assert(it->second.is_valid_unit_span(*mem_iter));
						// ָ����һ��
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
		// ������
		assert(memory_size % 8 == 0);
		if (memory_size == 0)
		{
			return std::nullopt;
		}

		// ���ڳ����ڴ棬ֱ���� page_cache (��Ȼ�ڵ�ǰ������̫����)
		if (memory_size > size_utils::MAX_CACHED_UNIT_SIZE)
		{
			return page_cache::get_instance().allocate_unit(memory_size);
		}

		const size_t index = size_utils::get_index(memory_size);
		std::optional<span<byte>> result = std::nullopt;

		// --- ���� ---
		auto& flag = pimpl->m_status[index];
		while (flag.test_and_set(std::memory_order_acquire))
		{
			std::this_thread::yield();
		}

		try
		{
			// 1. ���ȴ��������������ȡ
			if (!pimpl->m_free_array[index].empty())
			{
				result = pimpl->m_free_array[index].front();
				pimpl->m_free_array[index].pop_front();
				record_allocate_memory_span(*result);
			}
			// 2. �����������Ϊ�գ���� PageCache ��ȡ��ҳ
			else
			{
				size_t allocate_unit_count = page_span::MAX_UNIT_COUNT;
				size_t allocate_page_count = size_utils::align(memory_size * allocate_unit_count, size_utils::PAGE_SIZE) / size_utils::PAGE_SIZE;
				auto ret = get_page_from_page_cache(allocate_page_count);
				if (ret.has_value())
				{
					span<byte> memory = ret.value();

					// ���� page_span ��������ҳ
					page_span page_span(memory, memory_size);

					// �ָ��ڴ�ҳ��һ�����أ�����ķ�����������
					// ���ص�һ����
					result = memory.subspan(0, memory_size);
					page_span.allocate(*result);
					memory = memory.subspan(memory_size);

					// �� page_span ��¼����
					auto start_addr = page_span.data();
					pimpl->m_page_set[index].emplace(start_addr, move(page_span));

					// ʣ��Ŀ������������
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

		// --- ���� ---
		pimpl->m_status[index].clear(std::memory_order_release);
		return result;
	}
}