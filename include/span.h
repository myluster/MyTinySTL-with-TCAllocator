#pragma once
#include "type_traits.h"
#include "iterator.h"
#include "algobase.h"
#include <cassert>
#include <cstddef>

namespace mystl
{
	template<typename T,size_t Extent=static_cast<size_t>(-1)>
	class span
	{
	public:
		using element_type		= T;
		using value_type		= typename mystl::remove_cv<T>::type;
		using size_type			= size_t;
		using difference_type	= ptrdiff_t;
		using pointer			= T*;
		using const_pointer		= const T*;
		using reference			= T&;
		using const_reference	= const T&;

		//迭代器相关
		using iterator = pointer;
		using const_iterator = const_pointer;
		//反向迭代器
		using reverse_iterator = mystl::reverse_iterator<iterator>;
		using const_reverse_iterator = mystl::reverse_iterator<const_iterator>;

		//构造函数
		//默认构造函数，仅允许在Extent为0时或动态大小时可用
		constexpr span() noexcept :m_ptr(nullptr), m_size(0)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == 0,
				"A default-constructed span can only have an extent of 0 or be dynamic/zero.");
		}	

		//指针和长度构造函数
		constexpr span(pointer ptr, size_type count) noexcept
			: m_ptr(ptr), m_size(count)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == count,
				"Static extent must match provided count or be dynamic.");
		}
		
		//迭代器范围构造函数
		template<typename It,
			typename End,
			typename =typename mystl::enable_if<
			//确保迭代类型可以转换为element_type
			mystl::is_convertible<decltype(*mystl::declval<It>()),element_type>::value &&
			//保证迭代器时随机访问迭代器
			mystl::is_same<typename mystl::iterator_traits<It>::iterator_category, mystl::random_access_iterator_tag>::value
			>::type>
		constexpr span(It first, End last) noexcept
			:m_ptr(mystl::addressof(*first)),
			 m_size(static_cast<size_type>(mystl::distance(first, last)))
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == m_size,
				"Static extent must match calculated size or be dynamic.");
		}
		

		//数组构造函数
		template <size_t N,
			typename = typename mystl::enable_if<
			// 确保数组元素类型可转换为 span 的元素类型
			mystl::is_convertible<element_type(*)[], T(*)[]>::value
			>::type>
			constexpr span(element_type(&arr)[N]) noexcept
			: m_ptr(mystl::addressof(arr[0])), m_size(N)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == N,
				"Static extent must match array size or be dynamic.");
		}

		//容器构造函数 (从 array, vector 等容器构造)
		//需要满足容器有 data() 和 size() 成员函数，且其值类型可转换为 span 的元素类型
		template <typename Container,
			typename = typename mystl::enable_if<
			// 容器必须有 data() 和 size() 成员函数
			!mystl::is_array<Container>::value && // 排除C风格数组，避免与上面的数组构造函数冲突
			!mystl::is_array<Container>::value &&//排除直接转换为指针的情况
			!mystl::is_convertible<Container, pointer>::value &&
			mystl::is_convertible<decltype(mystl::declval<Container>().data()), pointer>::value &&
			mystl::is_convertible<decltype(mystl::declval<Container>().size()), size_type>::value
		>::type>
			constexpr span(Container& cont) noexcept
			: m_ptr(cont.data()), m_size(cont.size())
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == m_size,
				"Static extent must match container size or be dynamic.");
		}

		//隐式转换构造函数 (允许从不同类型或不同Extent的span转换)
		template <typename U, size_t N,
			typename =typename mystl::enable_if<
			(Extent == static_cast<size_t>(-1) || Extent == N) && // 目标span是动态的，或者extent匹配
			mystl::is_convertible<U(*)[], T(*)[]>::value // U 的数组可转换为 T 的数组 (处理 const 转换)
			>::type>
			constexpr span(const span<U, N>& s) noexcept
			: m_ptr(s.data()), m_size(s.size())
		{}


		// 拷贝构造函数和赋值运算符（默认为浅拷贝）
		constexpr span(const span& other) noexcept = default;
		constexpr span& operator=(const span& other) noexcept = default;

	public:
		// 元素访问
		constexpr reference operator[](size_type idx) const noexcept
		{
			assert(idx < m_size && "span index out of bounds");
			return m_ptr[idx];
		}

		constexpr reference at(size_type idx) const
		{
			if (idx >= m_size) {
				// 在实际STL中会抛出std::out_of_range，这里简化为assert
				assert(false && "span::at index out of range");
			}
			return m_ptr[idx];
		}

		constexpr reference front() const noexcept
		{
			assert(!empty() && "front() on empty span");
			return m_ptr[0];
		}

		constexpr reference back() const noexcept
		{
			assert(!empty() && "back() on empty span");
			return m_ptr[m_size - 1];
		}

		constexpr pointer data() const noexcept { return m_ptr; }

		// 大小和容量
		constexpr size_type size() const noexcept { return m_size; }
		constexpr size_type size_bytes() const noexcept { return m_size * sizeof(element_type); }
		constexpr bool empty() const noexcept { return m_size == 0; }
		constexpr static size_type extent() noexcept { return Extent; }

		// 子视图
		// 获取前N个元素的子span
		template <size_type Count>
		constexpr span<T, Count> first() const noexcept
		{
			static_assert(Extent == static_cast<size_t>(-1) || Count <= Extent,
				"Count must not exceed static extent.");
			assert(Count <= m_size && "Count exceeds span size.");
			return mystl::span<T, Count>(m_ptr, Count);
		}

		// 获取后N个元素的子span
		template <size_type Count>
		constexpr span<T, Count> last() const noexcept
		{
			static_assert(Extent == static_cast<size_t>(-1) || Count <= Extent,
				"Count must not exceed static extent.");
			assert(Count <= m_size && "Count exceeds span size.");
			return mystl::span<T, Count>(m_ptr + (m_size - Count), Count);
		}

		// 获取从Offset开始，Count个元素的子span
		template <size_type Offset, size_type Count = static_cast<size_t>(-1)>
		constexpr auto subspan() const noexcept
		{
			static_assert(Extent == static_cast<size_t>(-1) || Offset <= Extent,
				"Offset must not exceed static extent.");
			assert(Offset <= m_size && "Offset exceeds span size.");

			constexpr size_t NewExtent = (Count == static_cast<size_t>(-1))
				? ((Extent == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : Extent - Offset)
				: Count;

			assert(Count == static_cast<size_t>(-1) || Offset + Count <= m_size);

			return span<T, NewExtent>(m_ptr + Offset,
				(Count == static_cast<size_t>(-1)) ? (m_size - Offset) : Count);
		}

		// 运行时版本的subspan
		constexpr span<T, static_cast<size_t>(-1)> subspan(size_type offset, size_type count = static_cast<size_t>(-1)) const noexcept
		{
			assert(offset <= m_size && "Offset exceeds span size.");
			assert(count == static_cast<size_t>(-1) || offset + count <= m_size && "Count exceeds remaining span size.");

			return span<T, static_cast<size_t>(-1)>(m_ptr + offset,
				(count == static_cast<size_t>(-1)) ? (m_size - offset) : count);
		}

	public:
		// 迭代器支持
		constexpr iterator begin() const noexcept { return m_ptr; }
		constexpr iterator end() const noexcept { return m_ptr + m_size; }

		constexpr const_iterator cbegin() const noexcept { return m_ptr; }
		constexpr const_iterator cend() const noexcept { return m_ptr + m_size; }

		constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

	private:
		pointer m_ptr; // 指向数据的指针
		size_type m_size; // 数据的大小
	};

	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator==(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		if (lhs.size() != rhs.size()) {
			return false;
		}
		for (size_t i = 0; i < lhs.size(); ++i) {
			if (lhs[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	// 不等号
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator!=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(lhs == rhs);
	}

	// 小于号 (核心：字典序比较)
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator<(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// 大于号
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator>(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return rhs < lhs;
	}

	// 小于等于号
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator<=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(rhs < lhs);
	}

	// 大于等于号
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator>=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(lhs < rhs);
	}
}
