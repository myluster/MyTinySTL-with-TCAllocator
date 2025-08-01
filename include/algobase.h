#pragma once
#include <cstring>
#include "iterator.h"
#include "utils.h"
//这个文件实现的是一些比较基础的算法
namespace mystl
{
	//max
	template<class T>
	const T& max(const T& lhs, const T& rhs)
	{
		return lhs < rhs ? rhs : lhs;
	}

	//自定义比较函数
	template<class T, class Compare>
	const T& max(const T& lhs, const T& rhs, Compare cmp)
	{
		return cmp(lhs, rhs) ? rhs : lhs;
	}


	//min
	template <class T>
	const T& min(const T& lhs, const T& rhs)
	{
		return rhs < lhs ? rhs : lhs;
	}

	template <class T, class Compare>
	const T& min(const T& lhs, const T& rhs, Compare comp)
	{
		return comp(rhs, lhs) ? rhs : lhs;
	}


	//iter_swap
	//将两个迭代器所指的对象对调
	template<class FIter1, class FIter2>
	void iter_swap(FIter1 lhs, FIter2 rhs)
	{
		mystl::swap(*lhs, *rhs);
	}

	//copy
	// 把 [first, last)区间内的元素拷贝到 [result, result + (last - first))内

	//input_iterator_tag版本
	template<class InputIter, class OutputIter>
	OutputIter unchecked_copy_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag)
	{
		for (; first != last; ++first, ++result)
		{
			*result = *first;
		}
		return result;
	}

	//ramdom_access_iterator_tag版本
	template <class RandomIter, class OutputIter>
	OutputIter unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result,
		mystl::random_access_iterator_tag)
	{
		for (auto n = last - first; n > 0; --n, ++first, ++result)
		{
			*result = *first;
		}
		return result;
	}

	template <class InputIter, class OutputIter>
	OutputIter unchecked_copy(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_copy_cat(first, last, result, iterator_category(first));
	}

	// 为支持平凡拷贝的类型提供特化版本
	template <class Tp, class Up>
	typename mystl::enable_if<
		mystl::is_same<typename mystl::remove_const<Tp>::type, Up>::value&&
		mystl::is_trivially_copy_assignable<Up>::value,
		Up*>::type
		unchecked_copy(Tp* first, Tp* last, Up* result)
	{
		const auto n = static_cast<size_t>(last - first);
		if (n != 0)
			//平凡类型允许直接按字节复制
			std::memmove(result, first, n * sizeof(Up));//从first指向的内存块复制n*sizeof(Up)字节到result
		return result + n;//返回目标内存块末尾的下一个位置，方便链式操作
	}

	template <class InputIter, class OutputIter>
	OutputIter copy(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_copy(first, last, result);
	}

	//copy_backward
	//将 [first, last)区间内的元素拷贝到 [result - (last - first), result)内
	template<class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_copy_backward_cat(BidirectionalIter1 first, BidirectionalIter2 last,
			BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
	{
		while (first != last)
			*--result = *--last;//从后往前拷贝
		return result;
	}

	template <class RandomIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_copy_backward_cat(RandomIter1 first, RandomIter1 last,
			BidirectionalIter2 result, mystl::random_access_iterator_tag)
	{
		for (auto n = last - first; n > 0; --n)
			*--result = *--last;
		return result;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_copy_backward(BidirectionalIter1 first, BidirectionalIter1 last,
			BidirectionalIter2 result)
	{
		return unchecked_copy_backward_cat(first, last, result,
			iterator_category(first));
	}

	// 为 trivially_copy_assignable 类型提供特化版本
	template <class Tp, class Up>
	typename mystl::enable_if<
		mystl::is_same<typename mystl::remove_const<Tp>::type, Up>::value&&
		mystl::is_trivially_copy_assignable<Up>::value,
		Up*>::type
		unchecked_copy_backward(Tp* first, Tp* last, Up* result)
	{
		const auto n = static_cast<size_t>(last - first);
		if (n != 0)
		{
			result -= n;
			std::memmove(result, first, n * sizeof(Up));
		}
		return result;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
	{
		return unchecked_copy_backward(first, last, result);
	}

	// copy_n
	// 把 [first, first + n)区间上的元素拷贝到 [result, result + n)上
	// 返回一个 pair 分别指向拷贝结束的尾部
	template <class InputIter, class Size, class OutputIter>
	mystl::pair<InputIter, OutputIter>
		unchecked_copy_n(InputIter first, Size n, OutputIter result, mystl::input_iterator_tag)
	{
		for (; n > 0; --n, ++first, ++result)
		{
			*result = *first;
		}
		return mystl::pair<InputIter, OutputIter>(first, result);
	}

	template <class RandomIter, class Size, class OutputIter>
	mystl::pair<RandomIter, OutputIter>
		unchecked_copy_n(RandomIter first, Size n, OutputIter result,
			mystl::random_access_iterator_tag)
	{
		auto last = first + n;
		return mystl::pair<RandomIter, OutputIter>(last, mystl::copy(first, last, result));
	}

	template <class InputIter, class Size, class OutputIter>
	mystl::pair<InputIter, OutputIter>
		copy_n(InputIter first, Size n, OutputIter result)
	{
		return unchecked_copy_n(first, n, result, iterator_category(first));
	}

	//move
	//把[first,last)区间内的元素移动到[result, result + (last - first))内
	template<class InputIter, class OutputIter>
	OutputIter unchecked_move_cat(InputIter first, InputIter last, OutputIter result, mystl::input_iterator_tag)
	{
		for (; first != last; first++, result++)//不同版本区别主要在于循环条件
		{
			*result = mystl::move(*first);
		}
		return result;
	}

	template <class RandomIter, class OutputIter>
	OutputIter unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result, mystl::random_access_iterator_tag)
	{
		for (auto n = last - first; n > 0; --n, ++first, ++result)
		{
			*result = mystl::move(*first);
		}
		return result;
	}

	template <class InputIter, class OutputIter>
	OutputIter unchecked_move(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_move_cat(first, last, result, iterator_category(first));
	}
	//针对可以平凡移动的类型
	//通过内存级别的批量操作memmove替代逐元素的移动，以大幅提升性能
	template<class Tp, class Up>
	typename mystl::enable_if<mystl::is_same<typename mystl::remove_const<Tp>::type, Up>::value&&
		mystl::is_trivially_move_assignable<Up>::value, Up*>::type
		//这里std::enable_if的用法与常见的模板参数默认值方式不同，它直接通过返回值类型触发SFINAE机制
		//此处的std::enable_if被用作 返回值类型的一部分，而非模板参数的默认值
		//条件为真时：std::enable_if<Condition, Up*>::type等价于Up*，函数正常存在
		//条件为假时：std::enable_if无type成员，导致模板替换失败，该函数从重载集中被剔除（SFINAE）
		unchecked_move(Tp* first, Tp* last, Up* result)
	{
		const size_t n = static_cast<size_t>(last - first);
		if (n != 0)
			std::memmove(result, first, n * sizeof(Up));
		return result + n;
	}

	template<class InputIter, class OutputIter>
	OutputIter move(InputIter first, InputIter last, OutputIter result)
	{
		return unchecked_move(first, last, result);
	}

	//move_back
	//将 [first, last)区间内的元素移动到 [result - (last - first), result)内
	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_move_backward_cat(BidirectionalIter1 first, BidirectionalIter1 last,
			BidirectionalIter2 result, mystl::bidirectional_iterator_tag)
	{
		while (first != last)
			*--result = mystl::move(*--last);
		return result;
	}

	// random_access_iterator_tag 版本
	template <class RandomIter1, class RandomIter2>
	RandomIter2
		unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last,
			RandomIter2 result, mystl::random_access_iterator_tag)
	{
		for (auto n = last - first; n > 0; --n)
			*--result = mystl::move(*--last);
		return result;
	}

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		unchecked_move_backward(BidirectionalIter1 first, BidirectionalIter1 last,
			BidirectionalIter2 result)
	{
		return unchecked_move_backward_cat(first, last, result,
			iterator_category(first));
	}

	// 为 trivially_copy_assignable 类型提供特化版本
	template <class Tp, class Up>
	typename mystl::enable_if<
		mystl::is_same<typename mystl::remove_const<Tp>::type, Up>::value&&
		mystl::is_trivially_move_assignable<Up>::value,
		Up*>::type
		unchecked_move_backward(Tp* first, Tp* last, Up* result)
	{
		const size_t n = static_cast<size_t>(last - first);
		if (n != 0)
		{
			result -= n;
			std::memmove(result, first, n * sizeof(Up));
		}
		return result;
	}
	//memmove是C++标准库中的一个内存操作函数
	//用于将一段内存数据从源地址复制到目标地址(memmove 直接操作内存，不会调用构造函数或析构函数)
	//它的核心特点是安全处理内存重叠(与memcpy不同,选择从后向前复制，确保正确结果)，适用于需要内存复制的场景
	//尤其是在源和目标内存区域可能存在重叠时.

	template <class BidirectionalIter1, class BidirectionalIter2>
	BidirectionalIter2
		move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result)
	{
		return unchecked_move_backward(first, last, result);
	}

	//equal
	//比较第一序列在 [first, last)区间上的元素值是否和第二序列相等
	template<class InputIter1, class InputIter2>
	bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2)
	{
		for (; first1 != last1; first1++, first2++)
		{
			if (*first1 != *first2)
				return false;
		}
		return true;
	}

	// 重载版本使用自定义比较函数对象 comp 代替比较操作
	template <class InputIter1, class InputIter2, class Compared>
	bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compared comp)
	{
		for (; first1 != last1; ++first1, ++first2)
		{
			if (!comp(*first1, *first2))
				return false;
		}
		return true;
	}

	// fill_n
	// 从 first 位置开始填充 n 个值
	template <class OutputIter, class Size, class T>
	OutputIter unchecked_fill_n(OutputIter first, Size n, const T& value)
	{
		for (; n > 0; --n, ++first)
		{
			*first = value;
		}
		return first;
	}

	// 为 one-byte 类型提供特化版本
	template <class Tp, class Size, class Up>
	typename mystl::enable_if<
		mystl::is_integral<Tp>::value && sizeof(Tp) == 1 &&
		!mystl::is_same<Tp, bool>::value&&//排除bool
		mystl::is_integral<Up>::value && sizeof(Up) == 1,
		Tp*>::type
		unchecked_fill_n(Tp* first, Size n, Up value)
	{
		if (n > 0)
		{
			std::memset(first, (unsigned char)value, (size_t)(n));
		}
		return first + n;
	}

	template <class OutputIter, class Size, class T>
	OutputIter fill_n(OutputIter first, Size n, const T& value)
	{
		return unchecked_fill_n(first, n, value);
	}



	// fill
	// 为 [first, last)区间内的所有元素填充新值
	template <class ForwardIter, class T>
	void fill_cat(ForwardIter first, ForwardIter last, const T& value,
		mystl::forward_iterator_tag)
	{
		for (; first != last; ++first)
		{
			*first = value;
		}
	}

	template <class RandomIter, class T>
	void fill_cat(RandomIter first, RandomIter last, const T& value,
		mystl::random_access_iterator_tag)
	{
		mystl::fill_n(first, last - first, value);
	}

	template <class ForwardIter, class T>
	void fill(ForwardIter first, ForwardIter last, const T& value)
	{
		fill_cat(first, last, value, iterator_category(first));
	}

	// lexicographical_compare
	// 以字典序排列对两个序列进行比较，当在某个位置发现第一组不相等元素时，有下列几种情况：
	// (1)如果第一序列的元素较小，返回 true ，否则返回 false
	// (2)如果到达 last1 而尚未到达 last2 返回 true
	// (3)如果到达 last2 而尚未到达 last1 返回 false
	// (4)如果同时到达 last1 和 last2 返回 false
	template<class InputIter1, class InputIter2>
	inline bool lexicographical_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2)
	{
		for (; first1 != last1 && first2 != last2; first1++, first2++)
		{
			if (*first1 < *first2)
				return true;
			if (*first2 < *first1)
				return false;
		}
		return first1 == last1 && first2 != last2;
	}

	template <class InputIter1, class InputIter2, class Compred>
	inline bool lexicographical_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2, Compred comp)
	{
		for (; first1 != last1 && first2 != last2; ++first1, ++first2)
		{
			if (comp(*first1, *first2))
				return true;
			if (comp(*first2, *first1))
				return false;
		}
		return first1 == last1 && first2 != last2;
	}

	// 针对 const unsigned char* 的特化版本
	inline bool lexicographical_compare(const unsigned char* first1,
		const unsigned char* last1,
		const unsigned char* first2,
		const unsigned char* last2)
	{
		const auto len1 = last1 - first1;
		const auto len2 = last2 - first2;
		// 先比较相同长度的部分
		const auto result = std::memcmp(first1, first2, mystl::min(len1, len2));
		// 若相等，长度较长的比较大
		return result != 0 ? result < 0 : len1 < len2;
	}


}
