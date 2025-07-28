#pragma once
#include "type_traits.h" 
//这个文件包含一些通用工具，包括 move, forward, swap 等函数，以及 pair 等 
namespace mystl
{
	// --------------------------------------------------------------------------------------
	//                                     mem Function
	// --------------------------------------------------------------------------------------
	//这些都是底层内存操作函数
	//将指定内存区域连续填充指定值
	//dest：指向要设置的内存块的起始地址的指针，val：要用来填充的值（只有它的前八位会被用来填充）
	//bytes：要设置的字节数
	inline void* memset(void* dest, int val, size_t bytes)noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);//每个元素都是一个字节
		for (size_t i = 0; i < bytes; i++)
		{
			d[i] = static_cast<unsigned char>(val);
		}
		return dest;
	}

	//将源内存区域的内容复制到目标内存区域（不处理重叠）
	inline void* memcpy(void* dest, const void* src, size_t bytes)noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);
		const unsigned char* s = static_cast<const unsigned char*>(src);
		for (size_t i = 0; i < bytes; ++i) {
			d[i] = s[i];
		}
		return dest;
	}

	//将源内存区域的内容移动到目标内存区域 (安全处理重叠)
	inline void* memmove(void* dest, const void* src, size_t bytes) noexcept
	{
		unsigned char* d = static_cast<unsigned char*>(dest);
		const unsigned char* s = static_cast<const unsigned char*>(src);

		if (d < s) { // 从前往后复制
			for (size_t i = 0; i < bytes; ++i) {
				d[i] = s[i];
			}
		}
		else if (d > s) { // 从后往前复制
			for (size_t i = bytes; i-- > 0;) { // 注意循环条件和递减
				d[i] = s[i];
			}
		}
		return dest;
	}


	// --------------------------------------------------------------------------------------
	//                                     Move Function
	// --------------------------------------------------------------------------------------
	//将对象转换为右值引用以支持移动语义
	template<class T>
	typename mystl::remove_reference<T>::type&& move(T&& arg)noexcept//T&&既能绑定左值也能绑定右值
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
		lhs = mystl::move(rhs);//若T实现了移动构造/移动赋值，则资源高效转移；否则回退到拷贝操作
		rhs = mystl::move(tmp);
	}

	template<class ForwardIter1, class ForwardIter2>//前向迭代器类型
	ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2)
	{
		for (; first1 != last1; first1++, (void)first2++)//左闭右开区间，使用void显式忽略first2++的返回值（因为其在循环判断中未使用）
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

	//forward完美转发
	//完美转发用于通用引用 (T&& + 类型推导)
	//目的：保持参数的值类别（左值还是右值），在泛型编程中将参数原封不动地传递给其他函数。
	//原理解释：
	//std::remove_reference<T>::type的作用：移除T的所有引用修饰符
	//将arg强制转换为T&&，使用引用折叠规则(C++11)实现完美转发
	//引用折叠规则：
	//T& &->T&
	//T&& &->T&
	//T& &&->T&
	//T&& &&->T&&

	//注意折叠规则只适用于模板参数推导中，即template的使用中
	//如果是写死的&&是明确的右值引用

	//处理左值引用
	template<class T>
	T&& forward(typename std::remove_reference<T>::type& arg) noexcept
	{
		return static_cast<T&&>(arg);
	}

	//处理右值引用
	template<class T>
	T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
	{
		//T&&不一定是右值引用，T&&被称为通用
		//引用它的实际类型会根据传入参数的值类别（左值或右值）被推导为不同的引用类型（满足折叠规则）
		//断言判断，is_lvalue_reference判断每个类型是否为左值
		//禁止T为左值引用类型，防止将左值误转为右值
		static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
		return static_cast<T&&>(arg);
	}

	// --------------------------------------------------------------------------------------
	//                                     Pair Function
	// --------------------------------------------------------------------------------------

	//pair工具类
	template<class T1, class T2>
	struct pair
	{
		typedef T1 first_type;
		typedef T2 second_type;
		first_type first;
		second_type second;

		//默认构造函数
		//使用匿名模板参数和std::enable_if进行条件编译（SFINAE技巧）
		//如果is_default_constructible均成立，即T1和T2都可默认构造时
		//std::enable_if提供void类型，否则缺失参数
		template< class Other1 = T1, class Other2 = T2,
			typename = typename std::enable_if<
			std::is_default_constructible<Other1>::value&&
			std::is_default_constructible<Other2>::value, void>::type>
		constexpr pair() :first(), second() {}

		//双参数构造函数
		//隐式版本
		template<class U1 = T1, class U2 = T2,
			typename std::enable_if<
			std::is_copy_constructible<U1>::value&&//判断U1是否可显式拷贝构造
			std::is_copy_constructible<U2>::value&&
			std::is_convertible<const U1&, T1>::value&&//判断U1类型是否能隐式转换为T1
			std::is_convertible<const U2&, T2>::value, int>::type = 0>
		constexpr pair(const U1& a, const U2& b) ://左值引用
			first(a), second(b) {
		}

		//显式版本
		template<class U1 = T1, class U2 = T2,
			typename std::enable_if<
			std::is_copy_constructible<U1>::value&&
			std::is_copy_constructible<U2>::value &&
			(!std::is_convertible<const U1&, T1>::value ||
				!std::is_convertible<const U2&, T2>::value), int>::type = 0>
		explicit constexpr pair(const U1& a, const U2& b) ://explicit阻止隐式转换，必须全部显式
			first(a), second(b) {
		}

		//默认拷贝构造，逐个成员拷贝
		pair(const pair& rhs) = default;
		//默认移动构造，逐个成员移动
		pair(pair&& rhs) = default;

		//通用完美转发构造函数
		//隐式版本
		template<class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1&&>::value&&//判断T1是否能用Other1&&类型构造
			std::is_constructible<T2, Other2&&>::value&&
			std::is_convertible<Other1&&, T1>::value&&
			std::is_convertible<Other2&&, T2>::value, int>::type = 0>
		constexpr pair(Other1&& a, Other2&& b) ://通用引用
			first(mystl::forward<Other1>(a)), second(mystl::forward<Other2>(b)) {
		}//完美转发
		//通用转发构造函数与隐式构造函数条件同时满足时使用隐式构造函数
		//因为重载时优先更特化的模板，在这里是使用了默认值的模板

		//显式版本
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
		explicit constexpr pair(Other1&& a, Other2&& b) :
			first(mystl::forward<Other1>(a)), second(mystl::forward<Other2>(b)) {
		}

		//跨类型构造函数
		//左值隐式版本
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value&&
			std::is_convertible<const Other1&, T1>::value&&
			std::is_convertible<const Other2&, T2>::value, int>::type = 0>
		constexpr pair(const pair<Other1, Other2>& other) :
			first(other.first), second(other.second) {
		}

		//左值显式版本
		template <class Other1, class Other2,
			typename std::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value &&
			(!std::is_convertible<const Other1&, T1>::value ||
				!std::is_convertible<const Other2&, T2>::value), int>::type = 0>
		explicit constexpr pair(const pair<Other1, Other2>& other) :
			first(other.first), second(other.second) {
		}

		//右值隐式版本
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

		//右值显式版本
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

		//运算符重载
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
				first = mystl::move(rhs.first);//强制转为右值触发转移赋值
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
		//这不是构造所以不用严格检查是否可以隐式转换，该报错报错

		template <class Other1, class Other2>
		pair& operator=(pair<Other1, Other2>&& other)
		{
			first = mystl::forward<Other1>(other.first);
			second = mystl::forward<Other2>(other.second);
			return *this;
		}
		//因为这里其实并不保证Other不是引用类型等，所以需要使用完美转发

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
		// 字典序比较：先比较 first，如果 first 相等再比较 second
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
