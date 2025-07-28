#include "../../include/TCMalloc/ThreadCache.h"
#include"../../include/TCMalloc/CentralCache.h"
#include "../../include/TCMalloc/TCMallocutils.h"
#include "../../include/TCMalloc/TCMallocBootstrap.h"
#include "../../include/list.h"
#include "../../include/set.h"
#include "../../include/unordered_map.h"
#include <assert.h>
namespace mystl
{
	class ThreadCacheImpl
	{
	public:
		// ��ԭ�е�˽�����ݳ�Ա�Ƶ�����
		list<span<byte>> m_free_cache[size_utils::CACHE_LINE_SIZE];
		size_t m_next_allocate_count[size_utils::CACHE_LINE_SIZE];
	};

	thread_cache::thread_cache() : pimpl(nullptr)
	{
		g_is_allocator_constructing = true;
		// ��ʼ������
		pimpl = new ThreadCacheImpl();

		for (size_t i = 0; i < size_utils::CACHE_LINE_SIZE; ++i)
		{
			pimpl->m_next_allocate_count[i] = 1;
		}

		g_is_allocator_constructing = false;
	}

	thread_cache::~thread_cache()
	{
		delete pimpl;
	}

	std::optional<void*> thread_cache::allocate(size_t memory_size)
	{
		if (memory_size == 0)
		{
			return std::nullopt;
		}

		//���뵽8�ֽ�
		memory_size = size_utils::align(memory_size);

		if (memory_size > size_utils::MAX_CACHED_UNIT_SIZE) {
			auto result = allocate_from_central_cache(memory_size);
			if (result) {
				return std::optional<void*>(result->data());
			}
			else {
				return std::nullopt; 
			}
		}

		const size_t index = size_utils::get_index(memory_size);
		if (!pimpl->m_free_cache[index].empty())
		{
			auto result = pimpl->m_free_cache[index].front();
			pimpl->m_free_cache[index].pop_front();
			assert(result.size() == memory_size);
			return result.data();
		}
		auto result = allocate_from_central_cache(memory_size);
		if (result) {
			return std::optional<void*>(result->data());
		}
		else {
			return std::nullopt;
		}
	}

	void thread_cache::deallocate(void* start_p, size_t memory_size)
	{
		if (memory_size == 0) {
			return;
		}
		memory_size = size_utils::align(memory_size);
		span<byte> memory(static_cast<byte*>(start_p), memory_size);
		// �����������󻺴�ֵ�ˣ�˵����ֱ�Ӵ����Ļ���������ģ�����ֱ�ӷ��������Ļ�����
		if (memory_size > size_utils::MAX_CACHED_UNIT_SIZE) {
			list<span<byte>> free_list;
			free_list.push_back(memory);
			central_cache::get_instance().deallocate(std::move(free_list));
			return;
		}
		const size_t index = size_utils::get_index(memory_size);
		pimpl->m_free_cache[index].push_front(memory);

		// ���һ���費��Ҫ����
		// �����ǰ���б���ά���Ĵ�С�Ѿ���������ֵ���򴥷���Դ����
		// ά���Ĵ�С = ���� �� �����ռ�Ĵ�С
		if (pimpl->m_free_cache[index].size() * memory_size > MAX_FREE_BYTES_PER_LISTS) {
			// ��������ˣ������һ��Ķ�����ڴ��
			size_t deallocate_block_size = pimpl->m_free_cache[index].size() / 2;
			list<span<byte>> memory_to_deallocate;
			auto middle = pimpl->m_free_cache[index].begin();
			advance(middle, deallocate_block_size);
			memory_to_deallocate.splice(memory_to_deallocate.begin(), pimpl->m_free_cache[index], middle, pimpl->m_free_cache[index].end());
			central_cache::get_instance().deallocate(std::move(memory_to_deallocate));
			// �ڻ��չ�������Ժ󣬻�Ҫ��������ռ��С������ĸ���
			// ������һ������ĸ���
			pimpl->m_next_allocate_count[index] /= 2;
		}
	}

	std::optional<span<byte>> thread_cache::allocate_from_central_cache(size_t memory_size) {
		size_t block_count = compute_allocate_count(memory_size);
		auto allocation_result = central_cache::get_instance().allocate(memory_size, block_count);
		if (allocation_result)
		{
			auto memory_list = move(allocation_result.value());

			span<byte> result = memory_list.front();
			assert(result.size() == memory_size);
			memory_list.pop_front();
			const size_t index = size_utils::get_index(memory_size);

			pimpl->m_free_cache[index].splice(pimpl->m_free_cache[index].end(), memory_list);

			return result;
		}
		else
		{
			return std::nullopt;
		}
	}

	size_t thread_cache::compute_allocate_count(size_t memory_size) {
		// ��ȡ���±�
		size_t index = size_utils::get_index(memory_size);

		if (index >= size_utils::CACHE_LINE_SIZE) {
			return 1;
		}

		// ��������4����
		size_t result = std::max(pimpl->m_next_allocate_count[index], static_cast<size_t>(4));


		// ������һ��Ҫ����ĸ�����Ĭ�ϳ�2
		size_t next_allocate_count = result * 2;
		// Ҫȷ�����ᳬ��center_cacheһ�������������
		next_allocate_count = std::min(next_allocate_count, page_span::MAX_UNIT_COUNT);
		// ͬʱҲҪȷ�����ᳬ��һ���б�ά�����������
		// ����16KB���ڴ�飬����һ��������128����
		// 256 * 1024 B / 16 * 1024 B / 2 = 8��������ͽ�16KB���ڴ�һ�����������8����Ҫ��������(��2)����Ȼ���ܻᷴ�����룩
		next_allocate_count = std::min(next_allocate_count, MAX_FREE_BYTES_PER_LISTS / memory_size / 2);
		// ������һ��Ҫ����ĸ���
		pimpl->m_next_allocate_count[index] = next_allocate_count;
		// ������һ������ĸ���
		return result;
	}
}
