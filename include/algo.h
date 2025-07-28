#pragma once
#include "iterator.h"
// 这个头文件包含了一系列比较复杂的算法
namespace mystl
{
	//all_of
	//检查[first,last)内是否全部元素都满足一元操作unary_pred为true的情况，满足则返回 true
	template<class InputIter, class UnaryPredicate>
	bool all_of(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		for (; first != last; first++)
		{
			if (!unary_pred(*first))
				return false;
		}
		return true;
	}

	//any_of
	//检查[first, last)内是否存在某个元素满足一元操作unary_pred为true 的情况，满足则返回 true
	template <class InputIter, class UnaryPredicate>
	bool any_of(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		for (; first != last; ++first)
		{
			if (unary_pred(*first))
				return true;
		}
		return false;
	}

	// none_of
	// 检查[first, last)内是否全部元素都不满足一元操作 unary_pred 为 true 的情况，满足则返回 true
	template <class InputIter, class UnaryPredicate>
	bool none_of(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		for (; first != last; ++first)
		{
			if (unary_pred(*first))
				return false;
		}
		return true;
	}

	//count
	//对[first, last)区间内的元素与给定值进行比较，缺省使用 operator==，返回元素相等的个数
	template <class InputIter, class T>
	size_t count(InputIter first, InputIter last, const T& value)
	{
		size_t n = 0;
		for (; first != last; first++)
		{
			if (*first == value)
				n++;
		}
		return n;
	}

	// count_if
	// 对[first, last)区间内的每个元素都进行一元 unary_pred 操作，返回结果为 true 的个数
	template <class InputIter, class UnaryPredicate>
	size_t count_if(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		size_t n = 0;
		for (; first != last; ++first)
		{
			if (unary_pred(*first))
				++n;
		}
		return n;
	}
	// find
	// 在[first, last)区间内找到等于 value 的元素，返回指向该元素的迭代器
	template<class InputIter, class T>
	InputIter find(InputIter first, InputIter last, const T& value)
	{
		while (first != last && *first != value)
			first++;
		return first;
	}

	//find_if
	//在[first, last)区间内找到第一个令一元操作 unary_pred 为 true 的元素并返回指向该元素的迭代器
	template <class InputIter, class UnaryPredicate>
	InputIter find_if(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		while (first != last && !unary_pred(*first))
			++first;
		return first;
	}

	// find_if_not
	// 在[first, last)区间内找到第一个令一元操作 unary_pred 为 false 的元素并返回指向该元素的迭代器
	template <class InputIter, class UnaryPredicate>
	InputIter find_if_not(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		while (first != last && unary_pred(*first))
			++first;
		return first;
	}

	// search
	// 在[first1, last1)中查找[first2, last2)的首次出现点
	template<class ForwardIter1, class ForwardIter2>
	ForwardIter1 search(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2, ForwardIter2 last2)
	{
		auto d1 = mystl::distance(first1, last1);
		auto d2 = mystl::distance(first2, last2);
		if (d1 < d2)
			return last1;
		auto current1 = first1;
		auto current2 = first2;
		while (current2 != last2)
		{
			if (*current1 == *current2)
			{
				++current1;
				++current2;
			}
			else
			{
				if (d1 == d2)
				{
					return last1;
				}
				else
				{
					current1 = ++first1;
					current2 = first2;
					--d1;
				}
			}
		}
		return first1;
	}

	// 重载版本使用自定义函数对象 comp 代替比较操作
	template <class ForwardIter1, class ForwardIter2, class Compared>
	ForwardIter1 search(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2, ForwardIter2 last2, Compared comp)
	{
		auto d1 = mystl::distance(first1, last1);
		auto d2 = mystl::distance(first2, last2);
		if (d1 < d2)
			return last1;
		auto current1 = first1;
		auto current2 = first2;
		while (current2 != last2)
		{
			if (comp(*current1, *current2))
			{
				++current1;
				++current2;
			}
			else
			{
				if (d1 == d2)
				{
					return last1;
				}
				else
				{
					current1 = ++first1;
					current2 = first2;
					--d1;
				}
			}
		}
		return first1;
	}

	//lower_bound
	//在[first, last)中查找第一个不小于 value 的元素，并返回指向它的迭代器，若没有则返回 last
	template<class ForwardIter, class T>
	ForwardIter lbound_dispatch(ForwardIter first, ForwardIter last, const T& value, forward_iterator_tag)
	{
		auto len = mystl::distance(first, last);
		auto half = len;
		ForwardIter middle;
		while (len > 0)
		{
			half = len >> 1;
			middle = first;
			mystl::advance(middle, half);//middle真正变成middle
			if (*middle < value)
			{
				first = middle;
				first++;
				len = len - half - 1;//在前半部分找
			}
			else
			{
				len = half;//在后半部分找
			}
		}
		return first;
	}

	template <class RandomIter, class T>
	RandomIter lbound_dispatch(RandomIter first, RandomIter last, const T& value, random_access_iterator_tag)
	{
		auto len = last - first;
		auto half = len;
		RandomIter middle;
		while (len > 0)
		{
			half = len >> 1;
			middle = first + half;
			if (*middle < value)
			{
				first = middle + 1;
				len = len - half - 1;
			}
			else
			{
				len = half;
			}
		}
		return first;
	}

	template <class ForwardIter, class T>
	ForwardIter lower_bound(ForwardIter first, ForwardIter last, const T& value)
	{
		return mystl::lbound_dispatch(first, last, value, iterator_category(first));
	}


	//reverse
	//将[first,last)区间内的元素反转（左闭右开）

	//reverse_dispatch的双向迭代器版本
	template <class BidirectionalIter>
	void reverse_dispatch(BidirectionalIter first, BidirectionalIter last,
		bidirectional_iterator_tag)
	{
		while (true)
		{
			if (first == last || first == --last)//这里--last是因为右开
				return;
			mystl::iter_swap(first++, last);//first++是因为左闭
		}
	}
	//随机访问迭代器版本
	template<class RandomIter>
	void reverse_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
	{
		while (first < last)//随机访问迭代器支持<带来的方便
			mystl::iter_swap(first++, --last);
	}

	//整合
	template <class BidirectionalIter>
	void reverse(BidirectionalIter first, BidirectionalIter last)
	{
		mystl::reverse_dispatch(first, last, iterator_category(first));
	}


	//is_permutation
	//判断[first1,last1)是否为[first2,last2)的排列组合
	template <class ForwardIter1, class ForwardIter2, class BinaryPred>
	bool is_permutation_aux(ForwardIter1 first1, ForwardIter1 last1,
		ForwardIter2 first2, ForwardIter2 last2,
		BinaryPred pred)
	{
		constexpr bool is_ra_it = mystl::is_random_access_iterator<ForwardIter1>::value
			&& mystl::is_random_access_iterator<ForwardIter2>::value;
		if (is_ra_it)
		{
			auto len1 = last1 - first1;
			auto len2 = last2 - first2;
			if (len1 != len2)
				return false;
		}

		// 先找出相同的前缀段
		for (; first1 != last1 && first2 != last2; ++first1, (void) ++first2)
		{
			if (!pred(*first1, *first2))
				break;
		}
		if (is_ra_it)
		{
			if (first1 == last1)
				return true;
		}
		else
		{
			auto len1 = mystl::distance(first1, last1);
			auto len2 = mystl::distance(first2, last2);
			if (len1 == 0 && len2 == 0)
				return true;
			if (len1 != len2)
				return false;
		}

		// 判断剩余部分
		for (auto i = first1; i != last1; ++i)
		{
			bool is_repeated = false;
			for (auto j = first1; j != i; ++j)
			{
				if (pred(*j, *i))
				{
					is_repeated = true;
					break;
				}
			}

			if (!is_repeated)
			{
				// 计算 *i 在 [first2, last2) 的数目
				auto c2 = 0;
				for (auto j = first2; j != last2; ++j)
				{
					if (pred(*i, *j))
						++c2;
				}
				if (c2 == 0)
					return false;

				// 计算 *i 在 [first1, last1) 的数目
				auto c1 = 1;
				auto j = i;
				for (++j; j != last1; ++j)
				{
					if (pred(*i, *j))
						++c1;
				}
				if (c1 != c2)
					return false;
			}
		}
		return true;
	}

	template <class ForwardIter1, class ForwardIter2, class BinaryPred>
	bool is_permutation(ForwardIter1 first1, ForwardIter1 last1,
		ForwardIter2 first2, ForwardIter2 last2,
		BinaryPred pred)
	{
		return mystl::is_permutation_aux(first1, last1, first2, last2, pred);
	}
}