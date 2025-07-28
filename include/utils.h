#pragma once
#include "type_traits.h" 
//����ļ�����һЩͨ�ù��ߣ����� move, forward, swap �Ⱥ������Լ� pair �� 
namespace mystl
{
	// --------------------------------------------------------------------------------------
	//                                     mem Function
	// --------------------------------------------------------------------------------------
	//��Щ���ǵײ��ڴ��������
	//��ָ���ڴ������������ָ��ֵ
	//dest��ָ��Ҫ���õ��ڴ�����ʼ��ַ��ָ�룬val��Ҫ��������ֵ��ֻ������ǰ��λ�ᱻ������䣩
	//bytes��Ҫ���õ��ֽ���
	inline void* memset(void* dest, int val, size_t bytes)noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);//ÿ��Ԫ�ض���һ���ֽ�
		for (size_t i = 0; i < bytes; i++)
		{
			d[i] = static_cast<unsigned char>(val);
		}
		return dest;
	}

	//��Դ�ڴ���������ݸ��Ƶ�Ŀ���ڴ����򣨲������ص���
	inline void* memcpy(void* dest, const void* src, size_t bytes)noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);
		const unsigned char* s = static_cast<const unsigned char*>(src);
		for (size_t i = 0; i < bytes; ++i) {
			d[i] = s[i];
		}
		return dest;
	}

	//��Դ�ڴ�����������ƶ���Ŀ���ڴ����� (��ȫ�����ص�)
	inline void* memmove(void* dest, const void* src, size_t bytes) noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);
		const unsigned char* s = static_cast<const unsigned char*>(src);

		if (d < s) { // ��ǰ������
			for (size_t i = 0; i < bytes; ++i) {
				d[i] = s[i];
			}
		}
		else if (d > s) { // �Ӻ���ǰ����
			for (size_t i = bytes; i-- > 0;) { // ע��ѭ�������͵ݼ�
				d[i] = s[i];
			}
		}
		return dest;
	}


	// --------------------------------------------------------------------------------------
	//                                     Move Function
	// --------------------------------------------------------------------------------------
	//������ת��Ϊ��ֵ������֧���ƶ�����
	template<class T>
	typename mystl::remove_reference<T>::type&& move(T&& arg)noexcept//T&&���ܰ���ֵҲ�ܰ���ֵ
	{
		return static_cast<typename mystl::remove_reference<T>::type&&>(arg);
	}

	// --------------------------------------------------------------------------------------
	//                                     Swap Function
	// --------------------------------------------------------------------------------------
	template<class T>
	void swap(T& lhs, T& rhs)
	{
		auto tmp(mystl::move(lhs));
		lhs = mystl::move(rhs);//��Tʵ�����ƶ�����/�ƶ���ֵ������Դ��Чת�ƣ�������˵���������
		rhs = mystl::move(tmp);
	}

	template<class ForwardIter1, class ForwardIter2>//ǰ�����������
	ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
	{
		for (; first1 != last1; first1++, (void)first2++)//����ҿ����䣬ʹ��void��ʽ����first2++�ķ���ֵ����Ϊ����ѭ���ж���δʹ�ã�
			mystl::swap(*first1, *first2);
		return first2;
	}

	template<class T, size_t N>
	void swap(T(&a)[N], T(&b)[N])
	{
		mystl::swap_range(a, a + N, b);
	}

	// --------------------------------------------------------------------------------------
	//                                     Forward Function
	// --------------------------------------------------------------------------------------

	//forward����ת��
	//����ת������ͨ������ (T&& + �����Ƶ�)
	//Ŀ�ģ����ֲ�����ֵ�����ֵ������ֵ�����ڷ��ͱ���н�����ԭ�ⲻ���ش��ݸ�����������
	//ԭ����ͣ�
	//std::remove_reference<T>::type�����ã��Ƴ�T�������������η�
	//��argǿ��ת��ΪT&&��ʹ�������۵�����(C++11)ʵ������ת��
	//�����۵�����
	//T& &->T&
	//T&& &->T&
	//T& &&->T&
	//T&& &&->T&&

	//ע���۵�����ֻ������ģ������Ƶ��У���template��ʹ����
	//�����д����&&����ȷ����ֵ����

	//������ֵ����
	template<class T>
	T&& forward(typename std::remove_reference<T>::type& arg) noexcept
	{
		return static_cast<T&&>(arg);
	}

	//������ֵ����
	template<class T>
	T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
	{
		//T&&��һ������ֵ���ã�T&&����Ϊͨ��
		//��������ʵ�����ͻ���ݴ��������ֵ�����ֵ����ֵ�����Ƶ�Ϊ��ͬ���������ͣ������۵�����
		//�����жϣ�is_lvalue_reference�ж�ÿ�������Ƿ�Ϊ��ֵ
		//��ֹTΪ��ֵ�������ͣ���ֹ����ֵ��תΪ��ֵ
		static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
		return static_cast<T&&>(arg);
	}

	// --------------------------------------------------------------------------------------
	//                                     Pair Function
	// --------------------------------------------------------------------------------------

	//pair������
	template<class T1, class T2>
	struct pair
	{
		typedef T1 first_type;
		typedef T2 second_type;
		first_type first;
		second_type second;

		//Ĭ�Ϲ��캯��
		//ʹ������ģ�������std::enable_if�����������루SFINAE���ɣ�
		//���is_default_constructible����������T1��T2����Ĭ�Ϲ���ʱ
		//std::enable_if�ṩvoid���ͣ�����ȱʧ����
		template< class Other1 = T1, class Other2 = T2,
			typename = typename std::enable_if<
			std::is_default_constructible<Other1>::value&&
			std::is_default_constructible<Other2>::value, void>::type>
		constexpr pair() :first(), second() {}

		//˫�������캯��
		//��ʽ�汾
		template<class U1 = T1, class U2 = T2,
			typename std::enable_if<
			std::is_copy_constructible<U1>::value&&//�ж�U1�Ƿ����ʽ��������
			std::is_copy_constructible<U2>::value&&
			std::is_convertible<const U1&, T1>::value&&//�ж�U1�����Ƿ�����ʽת��ΪT1
			std::is_convertible<const U2&, T2>::value, int>::type = 0>
		constexpr pair(const U1& a, const U2& b) ://��ֵ����
			first(a), second(b) {
		}

		//��ʽ�汾
		template<class U1 = T1, class U2 = T2,
			typename std::enable_if<
			std::is_copy_constructible<U1>::value&&
			std::is_copy_constructible<U2>::value &&
			(!std::is_convertible<const U1&, T1>::value ||
				!std::is_convertible<const U2&, T2>::value), int>::type = 0>
		explicit constexpr pair(const U1& a, const U2& b) ://explicit��ֹ��ʽת��������ȫ����ʽ
			first(a), second(b) {
		}

		//Ĭ�Ͽ������죬�����Ա����
		pair(const pair& rhs) = default;
		//Ĭ���ƶ����죬�����Ա�ƶ�
		pair(pair&& rhs) = default;

		//ͨ������ת�����캯��
		//��ʽ�汾
		template<class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1&&>::value&&//�ж�T1�Ƿ�����Other1&&���͹���
			std::is_constructible<T2, Other2&&>::value&&
			std::is_convertible<Other1&&, T1>::value&&
			std::is_convertible<Other2&&, T2>::value, int>::type = 0>
		constexpr pair(Other1&& a, Other2&& b) ://ͨ������
			first(mystl::forward<Other1>(a)), second(mystl::forward<Other2>(b)) {
		}//����ת��
		//ͨ��ת�����캯������ʽ���캯������ͬʱ����ʱʹ����ʽ���캯��
		//��Ϊ����ʱ���ȸ��ػ���ģ�壬��������ʹ����Ĭ��ֵ��ģ��

		//��ʽ�汾
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
		explicit constexpr pair(Other1&& a, Other2&& b) :
			first(mystl::forward<Other1>(a)), second(mystl::forward<Other2>(b)) {
		}

		//�����͹��캯��
		//��ֵ��ʽ�汾
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value&&
			std::is_convertible<const Other1&, T1>::value&&
			std::is_convertible<const Other2&, T2>::value, int>::type = 0>
		constexpr pair(const pair<Other1, Other2>& other) :
			first(other.first), second(other.second) {
		}

		//��ֵ��ʽ�汾
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value &&
			(!std::is_convertible<const Other1&, T1>::value ||
				!std::is_convertible<const Other2&, T2>::value), int>::type = 0>
		explicit constexpr pair(const pair<Other1, Other2>& other) :
			first(other.first), second(other.second) {
		}

		//��ֵ��ʽ�汾
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value&&
			std::is_convertible<Other1, T1>::value&&
			std::is_convertible<Other2, T2>::value, int>::type = 0>
		constexpr pair(pair<Other1, Other2>&& other) :
			first(mystl::forward<Other1>(other.first)),
			second(mystl::forward<Other2>(other.second)) {
		}

		//��ֵ��ʽ�汾
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
		explicit constexpr pair(pair<Other1, Other2>&& other) :
			first(mystl::forward<Other1>(other.first)),
			second(mystl::forward<Other2>(other.second)) {
		}

		//���������
		pair& operator=(const pair& rhs)
		{
			if (this != &rhs)
			{
				first = rhs.first;
				second = rhs.second;
			}
			return *this;
		}

		pair& operator=(pair&& rhs)
		{
			if (this != &rhs)
			{
				first = mystl::move(rhs.first);//ǿ��תΪ��ֵ����ת�Ƹ�ֵ
				second = mystl::move(rhs.second);
			}
			return *this;
		}

		template <class Other1, class Other2>
		pair& operator=(const pair<Other1, Other2>& other)
		{
			first = other.first;
			second = other.second;
			return *this;
		}
		//�ⲻ�ǹ������Բ����ϸ����Ƿ������ʽת�����ñ�����

		template <class Other1, class Other2>
		pair& operator=(pair<Other1, Other2>&& other)
		{
			first = mystl::forward<Other1>(other.first);
			second = mystl::forward<Other2>(other.second);
			return *this;
		}
		//��Ϊ������ʵ������֤Other�����������͵ȣ�������Ҫʹ������ת��

		~pair() = default;

		void swap(pair& other)
		{
			if (this != &other)
			{
				mystl::swap(first, other.first);
				mystl::swap(second, other.second);
			}
		}

	};


	template <class T1, class T2>
	bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}

	template <class T1, class T2>
	bool operator!=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T1, class T2>
	bool operator<(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		// �ֵ���Ƚϣ��ȱȽ� first����� first ����ٱȽ� second
		return lhs.first < rhs.first || (!(rhs.first < lhs.first) && lhs.second < rhs.second);
	}

	template <class T1, class T2>
	bool operator<=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T1, class T2>
	bool operator>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return rhs < lhs;
	}

	template <class T1, class T2>
	bool operator>=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs)
	{
		return !(lhs < rhs);
	}

	// make_pair
	template <class T1, class T2>
	constexpr pair<
		typename std::decay<T1>::type,
		typename std::decay<T2>::type>
		make_pair(T1&& x, T2&& y)
	{
		return pair<
			typename std::decay<T1>::type,
			typename std::decay<T2>::type>(
				mystl::forward<T1>(x),
				mystl::forward<T2>(y));
	}
}
