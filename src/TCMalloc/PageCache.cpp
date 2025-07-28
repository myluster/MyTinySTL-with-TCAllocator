#define NOMINMAX//windows.h���Զ�����man��min����mystl�ĳ�ͻ
#if defined(_WIN32)
#include<windows.h>
#else
#include <sys/mman.h>
#endif

#include <iostream>

#include "../../include/TCMalloc/PageCache.h"
#include "../../include/TCMalloc/TCMallocBootstrap.h"
#include "../../include/set.h"
#include "../../include/map.h"
#include "../../include/vector.h"
namespace mystl
{
	class PageCacheImpl
	{
	public:
		// ��ԭ�е�˽�����ݳ�Ա�Ƶ�����
		mystl::map<size_t, mystl::set<span<byte>>> free_page_store;
		mystl::map<byte*, span<byte>> free_page_map;
		mystl::vector<span<byte>> page_vector;
	};

	page_cache::page_cache() : pimpl(nullptr), m_stop(false) 
	{
		g_is_allocator_constructing = true;
		pimpl = new PageCacheImpl();
		g_is_allocator_constructing = false;
	}

	page_cache::~page_cache()
	{
		stop();
		delete pimpl;
	}



	std::optional<span<byte>> page_cache::allocate_page(size_t page_count)
	{
		if (page_count == 0)
		{
			return std::nullopt;
		}
		std::unique_lock<std::mutex> guard(m_mutex);

		std::cout << "    [PageCache] INFO: Request to allocate " << page_count << " pages.\n";

		auto it = pimpl->free_page_store.lower_bound(page_count);
		while (it != pimpl->free_page_store.end())
		{
			if (!it->second.empty())
			{
				//�������һ��ҳ�棬���ҳ��Ĵ�С�Ǵ��ڻ����Ҫ�����ҳ��
				auto mem_iter = it->second.begin();
				span<byte> free_memory = *mem_iter;
				it->second.erase(mem_iter);
				pimpl->free_page_map.erase(free_memory.data());

				//�ָ��ȡ�Ŀռ�
				size_t memory_to_use = page_count * size_utils::PAGE_SIZE;
				span<byte> memory = free_memory.subspan(0, memory_to_use);
				free_memory = free_memory.subspan(memory_to_use);
				//�黹
				if (free_memory.size())
				{
					pimpl->free_page_store[free_memory.size() / size_utils::PAGE_SIZE];
					pimpl->free_page_map.emplace(free_memory.data(), free_memory);
				}
				return memory;
			}
			++it;
		}
		
		std::cout << "    [PageCache] WARNING: No suitable page in cache. Requesting from OS...\n";

		//����Ѿ�û���㹻���ҳ���ˣ�����ϵͳ����
		//һ������ϵͳ�������С����2048��ҳ��
		size_t page_to_allocate = mystl::max(PAGE_ALLOCATE_COUNT, page_count);
		auto memory_opt = system_allocate_memory(page_to_allocate);
		if (!memory_opt)
		{
			std::cout << "    [PageCache] FATAL: System memory allocation failed!\n";
			return std::nullopt; // ����ʧ��
		}
		span<byte> memory = *memory_opt;

		//�����ܵ��ڴ����ڽ�β�����ڴ�
		pimpl->page_vector.push_back(memory);
		size_t memory_to_use = page_count * size_utils::PAGE_SIZE;
		span<byte> result = memory.subspan(0, memory_to_use);
		span<byte> free_memory = memory.subspan(memory_to_use);
		if (free_memory.size())
		{
			size_t index = free_memory.size() / size_utils::PAGE_SIZE;
			pimpl->free_page_store[index].emplace(free_memory);
			pimpl->free_page_map.emplace(free_memory.data(), free_memory);
		}
		return result;
	}

	void page_cache::deallocate_page(span<byte> page)
	{
		//������һҳһҳ�Ļ��յģ����Դ�Сһ���ǻᱻ������
		assert(page.size() % size_utils::PAGE_SIZE == 0);
		std::unique_lock<std::mutex> guard(m_mutex);

		while (true) {
			//����ռ䲻Ӧ���Ѿ���������
			assert(pimpl->free_page_map.find(page.data()) == pimpl->free_page_map.end());
			auto it = pimpl->free_page_map.upper_bound(page.data());
			if (it != pimpl->free_page_map.begin()) {
				// ���ǰһ��span
				--it;
				const span<byte>& memory = it->second;
				if (memory.data() + memory.size() == page.data()) {
					// ���ǰ��һ�εĿռ��뵱ǰ�����ڣ���ϲ�
					page = span<byte>(memory.data(), memory.size() + page.size());
					// �ڴ������Ҳɾ��
					pimpl->free_page_store[memory.size() / size_utils::PAGE_SIZE].erase(memory);
					pimpl->free_page_map.erase(it);
				}
				else {
					break; // û��ǰ�����ڵĿ���Ժϲ�
				}
			}
			else {
				break; // û��ǰ���
			}
		}

		// ���������ڵ�span
		while (true) {
			assert(pimpl->free_page_map.find(page.data()) == pimpl->free_page_map.end());
			// **������**: �� map::contains �滻Ϊ map::find
			auto it = pimpl->free_page_map.find(page.data() + page.size());
			if (it != pimpl->free_page_map.end()) {
				span<byte> next_memory = it->second;
				pimpl->free_page_store[next_memory.size() / size_utils::PAGE_SIZE].erase(next_memory);
				pimpl->free_page_map.erase(it);
				page = span<byte>(page.data(), page.size() + next_memory.size());
			}
			else {
				break; // û�к������ڵĿ���Ժϲ�
			}
		}
		size_t index = page.size() / size_utils::PAGE_SIZE;
		pimpl->free_page_store[index].emplace(page);
		pimpl->free_page_map.emplace(page.data(), page);
	}

	std::optional<span<byte>> page_cache::allocate_unit(size_t memory_size) {
		auto ret = malloc(memory_size);
		if (ret != nullptr) {
			return span<byte>{ static_cast<byte*>(ret), memory_size };
		}
		return std::nullopt;
	}

	void page_cache::deallocate_unit(span<byte> memories)
	{
		free(memories.data());
	}

	void page_cache::stop() {
		std::unique_lock<std::mutex> guard(m_mutex);
		if (m_stop == false) {
			m_stop = true;
			for (auto& i : pimpl->page_vector) {
				system_deallocate_memory(i);
			}
		}
	}

	std::optional<span<byte>> page_cache::system_allocate_memory(size_t page_count)
	{
		const size_t size = page_count * size_utils::PAGE_SIZE;

#if defined(_WIN32)
		// Windows ʵ��
		void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (ptr == nullptr)
		{
			return std::nullopt;
		}
		// VirtualAlloc ������ڴ�Ĭ���������
		// memset(ptr, 0, size);
#else
		// POSIX (Linux, macOS) ʵ��
		void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED)
		{
			return std::nullopt;
		}
		// �����ڴ�
		memset(ptr, 0, size);
#endif
		return span<byte>{ static_cast<byte*>(ptr), size};
	}

	void page_cache::system_deallocate_memory(span<byte> page)
	{
#if defined(_WIN32)
		// Windows ʵ��
		// ���� VirtualAlloc ������ڴ棬size ����Ϊ 0����������Ϊ MEM_RELEASE
		VirtualFree(page.data(), 0, MEM_RELEASE);
#else
		// POSIX ʵ��
		munmap(page.data(), page.size());
#endif
	}
}