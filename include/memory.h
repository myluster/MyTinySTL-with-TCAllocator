#pragma once
//STL内存管理的核心部分，负责更高级的动态内存管理
//包含基本函数、空间配置器、未初始化的储存空间管理，以及智能指针 unique_ptr
#include <cstddef>//提供与内存布局相关的基础类型和常量
#include <cstdlib>//提供 C 标准库的内存管理函数malloc\free\realloc
#include <climits>//定义与整数类型相关的极限值

#include "utils.h"//使用move swap
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"
#include "iterator.h"
namespace mystl
{
	// 获取 / 释放 临时缓冲区

	template <class T>
	pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*)
	{
		//len * sizeof(T) 不超过 INT_MAX，避免溢出
		//static_cast<ptrdiff_t>将计算结果强制转换为有符号类型以匹配 len
		if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
			len = INT_MAX / sizeof(T);
		while (len > 0)
		{
			T* tmp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
			if (tmp)
				//返回分配的内存指针和实际成功分配的长度
				return pair<T*, ptrdiff_t>(tmp, len);
			len /= 2;  // 申请失败时减少 len 的大小
		}
		return pair<T*, ptrdiff_t>(nullptr, 0);
	}

	template<class T>
	pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len)
	{
		return get_buffer_helper(len, static_cast<T*>(0));
	}

	template<class T>
	pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len, T*)
	{
		return get_buffer_helper(len, static_cast<T*>(0));
	}

	template<class T>
	void release_temporary_buffer(T* ptr)
	{
		free(ptr);
	}

	// 类模板 : temporary_buffer
	// 进行临时缓冲区的申请与释放
	template<class T>
	class temporary_buffer
	{
	private:
		ptrdiff_t	original_len;	//缓冲区申请的大小
		ptrdiff_t	len;			//缓冲区实际的大小
		T* buffer;			//指向缓冲区的指针
	public:
		//构造、析构函数
		template<class ForwardIterator>
		temporary_buffer(ForwardIterator first, ForwardIterator last);

		~temporary_buffer()
		{
			if (buffer)
			{
				mystl::destroy(buffer, buffer + len);
				free(buffer);
			}
		}

		ptrdiff_t size()           const noexcept { return len; }
		ptrdiff_t requested_size() const noexcept { return original_len; }
		T* begin()                noexcept { return buffer; }
		T* end()                  noexcept { return buffer + len; }

		// 禁用拷贝构造和拷贝赋值
		temporary_buffer(const temporary_buffer&) = delete;//防止浅拷贝导致的资源重复释放
		temporary_buffer& operator=(const temporary_buffer&) = delete;

	private:
		void allocate_buffer();
	};

	//构造函数
	template<class T>
	template<class ForwardIterator>
	temporary_buffer<T>::temporary_buffer(ForwardIterator first, ForwardIterator last)
	{
		len = mystl::distance(first, last);
		original_len = len;
		try
		{
			allocate_buffer();
			if (len > 0)
			{
				uninitialized_copy(first, last, buffer);
			}
		}
		catch (...)
		{
			if (buffer) {
				mystl::destroy(buffer, buffer + len);  // 销毁已构造的对象
				free(buffer);
			}
			buffer = nullptr;
			len = 0;
			throw;
		}
	}

	// allocate_buffer 函数
	template <class T>
	void temporary_buffer<T>::allocate_buffer()
	{
		if (len <= 0)
		{
			buffer = nullptr;
			return;
		}
		size_t max_size = static_cast<ptrdiff_t>(INT_MAX / sizeof(T));
		if (len > max_size)
			len = max_size;
		while (len > 0)
		{
			buffer = static_cast<T*>(malloc(len * sizeof(T)));
			if (buffer)
			{
				break;
			}
			len /= 2;  // 申请失败时减少申请空间大小
		}
		if (len == 0)
		{
			throw std::bad_alloc();
		}
	}

	// uninitialized_value_construct_n
	// 在 [first, first + n) 区间内进行值初始化
	template <class ForwardIter>
	void unchecked_value_construct_n_aux(ForwardIter first, size_t n, std::false_type)
	{
		ForwardIter current = first;
		try {
			for (; n > 0; --n, ++current) {
				// 对每个元素调用默认构造函数 T()
				mystl::construct(mystl::addressof(*current));
			}
		}
		catch (...) {
			// 如果构造过程中抛出异常，则销毁所有已成功构造的对象
			mystl::destroy(first, current);
			throw; // 重新抛出异常
		}
	}

	// 内部辅助函数 (平凡类型版本) - 快速路径
	template <class ForwardIter>
	void unchecked_value_construct_n_aux(ForwardIter first, size_t n, std::true_type)
	{
		// 对于 POD (Plain Old Data) 类型，值初始化等同于零初始化
		// 使用 fill_n 效率更高
		mystl::fill_n(first, n, typename iterator_traits<ForwardIter>::value_type());
	}

	template <class ForwardIter>
	void uninitialized_value_construct_n(ForwardIter first, size_t n)
	{
		// 通过 is_trivially_default_constructible 进行分发
		// 如果类型 T 有一个不重要的(trivial)默认构造函数，我们就可以进行优化
		unchecked_value_construct_n_aux(first, n,
			std::is_trivially_default_constructible<
			typename iterator_traits<ForwardIter>::value_type
			>{});
	}

	//unique_ptr
	//修复了auto_ptr的设计缺陷
	//设计理念:独占所有权、资源安全管理、零额外开销

	template<class T>
	class unique_ptr
	{
	public:
		typedef T	element_type;
	private:
		T* m_ptr;	//实际指针
	public:
		//默认构造函数
		explicit unique_ptr(T* p = nullptr)noexcept :m_ptr(p) {}
		//删除拷贝构造函数
		unique_ptr(const unique_ptr&) = delete;
		//移动构造函数
		unique_ptr(unique_ptr&& rhs)noexcept :m_ptr(rhs.release()) {}
		//模板移动构造函数（支持派生类到基类的转换）
		template<class U>
		unique_ptr(unique_ptr<U>&& rhs)noexcept :m_ptr(rhs.release()) {}
		//析构函数
		~unique_ptr() { reset(); }

		//删除拷贝赋值
		unique_ptr& operator=(const unique_ptr&) = delete;

		//移动赋值操作符
		unique_ptr& operator=(unique_ptr&& rhs)noexcept
		{
			reset(rhs.release());
			return *this;
		}

		//模板移动赋值（支持派生类到基类的转换）
		template<class U>
		unique_ptr& operator=(unique_ptr<U>&& rhs)noexcept
		{
			reset(rhs.release());
			return *this;
		}

		//重载指针操作符
		T& operator*() const noexcept
		{
			return *m_ptr;
		}
		T* operator->()const noexcept
		{
			return m_ptr;
		}

		//获取原始指针
		T* get()const noexcept
		{
			return m_ptr;
		}

		//释放所有权
		T* release()noexcept
		{
			T* ret = m_ptr;
			m_ptr = nullptr;
			return ret;
		}

		//重置指针
		void reset(T* ptr = nullptr)noexcept
		{
			T* old = m_ptr;
			m_ptr = ptr;
			if (old)
				delete old;
		}

		//交换指针
		void swap(unique_ptr& other)noexcept
		{
			using mystl::swap;
			swap(m_ptr, other.m_ptr);
		}
	};
	//交换特化
	template<class T>
	void swap(unique_ptr<T>& lhs, unique_ptr<T>& rhs)noexcept
	{
		lhs.swap(rhs);
	}
}
