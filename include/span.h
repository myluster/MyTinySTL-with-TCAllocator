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

		//���������
		using iterator = pointer;
		using const_iterator = const_pointer;
		//���������
		using reverse_iterator = mystl::reverse_iterator<iterator>;
		using const_reverse_iterator = mystl::reverse_iterator<const_iterator>;

		//���캯��
		//Ĭ�Ϲ��캯������������ExtentΪ0ʱ��̬��Сʱ����
		constexpr span() noexcept :m_ptr(nullptr), m_size(0)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == 0,
				"A default-constructed span can only have an extent of 0 or be dynamic/zero.");
		}	

		//ָ��ͳ��ȹ��캯��
		constexpr span(pointer ptr, size_type count) noexcept
			: m_ptr(ptr), m_size(count)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == count,
				"Static extent must match provided count or be dynamic.");
		}
		
		//��������Χ���캯��
		template<typename It,
			typename End,
			typename =typename mystl::enable_if<
			//ȷ���������Ϳ���ת��Ϊelement_type
			mystl::is_convertible<decltype(*mystl::declval<It>()),element_type>::value &&
			//��֤������ʱ������ʵ�����
			mystl::is_same<typename mystl::iterator_traits<It>::iterator_category, mystl::random_access_iterator_tag>::value
			>::type>
		constexpr span(It first, End last) noexcept
			:m_ptr(mystl::addressof(*first)),
			 m_size(static_cast<size_type>(mystl::distance(first, last)))
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == m_size,
				"Static extent must match calculated size or be dynamic.");
		}
		

		//���鹹�캯��
		template <size_t N,
			typename = typename mystl::enable_if<
			// ȷ������Ԫ�����Ϳ�ת��Ϊ span ��Ԫ������
			mystl::is_convertible<element_type(*)[], T(*)[]>::value
			>::type>
			constexpr span(element_type(&arr)[N]) noexcept
			: m_ptr(mystl::addressof(arr[0])), m_size(N)
		{
			static_assert(Extent == static_cast<size_t>(-1) || Extent == N,
				"Static extent must match array size or be dynamic.");
		}

		//�������캯�� (�� array, vector ����������)
		//��Ҫ���������� data() �� size() ��Ա����������ֵ���Ϳ�ת��Ϊ span ��Ԫ������
		template <typename Container,
			typename = typename mystl::enable_if<
			// ���������� data() �� size() ��Ա����
			!mystl::is_array<Container>::value && // �ų�C������飬��������������鹹�캯����ͻ
			!mystl::is_array<Container>::value &&//�ų�ֱ��ת��Ϊָ������
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

		//��ʽת�����캯�� (����Ӳ�ͬ���ͻ�ͬExtent��spanת��)
		template <typename U, size_t N,
			typename =typename mystl::enable_if<
			(Extent == static_cast<size_t>(-1) || Extent == N) && // Ŀ��span�Ƕ�̬�ģ�����extentƥ��
			mystl::is_convertible<U(*)[], T(*)[]>::value // U �������ת��Ϊ T ������ (���� const ת��)
			>::type>
			constexpr span(const span<U, N>& s) noexcept
			: m_ptr(s.data()), m_size(s.size())
		{}


		// �������캯���͸�ֵ�������Ĭ��Ϊǳ������
		constexpr span(const span& other) noexcept = default;
		constexpr span& operator=(const span& other) noexcept = default;

	public:
		// Ԫ�ط���
		constexpr reference operator[](size_type idx) const noexcept
		{
			assert(idx < m_size && "span index out of bounds");
			return m_ptr[idx];
		}

		constexpr reference at(size_type idx) const
		{
			if (idx >= m_size) {
				// ��ʵ��STL�л��׳�std::out_of_range�������Ϊassert
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

		// ��С������
		constexpr size_type size() const noexcept { return m_size; }
		constexpr size_type size_bytes() const noexcept { return m_size * sizeof(element_type); }
		constexpr bool empty() const noexcept { return m_size == 0; }
		constexpr static size_type extent() noexcept { return Extent; }

		// ����ͼ
		// ��ȡǰN��Ԫ�ص���span
		template <size_type Count>
		constexpr span<T, Count> first() const noexcept
		{
			static_assert(Extent == static_cast<size_t>(-1) || Count <= Extent,
				"Count must not exceed static extent.");
			assert(Count <= m_size && "Count exceeds span size.");
			return mystl::span<T, Count>(m_ptr, Count);
		}

		// ��ȡ��N��Ԫ�ص���span
		template <size_type Count>
		constexpr span<T, Count> last() const noexcept
		{
			static_assert(Extent == static_cast<size_t>(-1) || Count <= Extent,
				"Count must not exceed static extent.");
			assert(Count <= m_size && "Count exceeds span size.");
			return mystl::span<T, Count>(m_ptr + (m_size - Count), Count);
		}

		// ��ȡ��Offset��ʼ��Count��Ԫ�ص���span
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

		// ����ʱ�汾��subspan
		constexpr span<T, static_cast<size_t>(-1)> subspan(size_type offset, size_type count = static_cast<size_t>(-1)) const noexcept
		{
			assert(offset <= m_size && "Offset exceeds span size.");
			assert(count == static_cast<size_t>(-1) || offset + count <= m_size && "Count exceeds remaining span size.");

			return span<T, static_cast<size_t>(-1)>(m_ptr + offset,
				(count == static_cast<size_t>(-1)) ? (m_size - offset) : count);
		}

	public:
		// ������֧��
		constexpr iterator begin() const noexcept { return m_ptr; }
		constexpr iterator end() const noexcept { return m_ptr + m_size; }

		constexpr const_iterator cbegin() const noexcept { return m_ptr; }
		constexpr const_iterator cend() const noexcept { return m_ptr + m_size; }

		constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
		constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

	private:
		pointer m_ptr; // ָ�����ݵ�ָ��
		size_type m_size; // ���ݵĴ�С
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

	// ���Ⱥ�
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator!=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(lhs == rhs);
	}

	// С�ں� (���ģ��ֵ���Ƚ�)
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator<(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	// ���ں�
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator>(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return rhs < lhs;
	}

	// С�ڵ��ں�
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator<=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(rhs < lhs);
	}

	// ���ڵ��ں�
	template <typename T, size_t Extent, typename U, size_t N>
	constexpr bool operator>=(const span<T, Extent>& lhs, const span<U, N>& rhs)
	{
		return !(lhs < rhs);
	}
}
