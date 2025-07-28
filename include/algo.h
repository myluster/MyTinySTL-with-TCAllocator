#pragma once
#include "iterator.h"
// ���ͷ�ļ�������һϵ�бȽϸ��ӵ��㷨
namespace mystl
{
	//all_of
	//���[first,last)���Ƿ�ȫ��Ԫ�ض�����һԪ����unary_predΪtrue������������򷵻� true
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
	//���[first, last)���Ƿ����ĳ��Ԫ������һԪ����unary_predΪtrue ������������򷵻� true
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
	// ���[first, last)���Ƿ�ȫ��Ԫ�ض�������һԪ���� unary_pred Ϊ true ������������򷵻� true
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
	//��[first, last)�����ڵ�Ԫ�������ֵ���бȽϣ�ȱʡʹ�� operator==������Ԫ����ȵĸ���
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
	// ��[first, last)�����ڵ�ÿ��Ԫ�ض�����һԪ unary_pred ���������ؽ��Ϊ true �ĸ���
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
	// ��[first, last)�������ҵ����� value ��Ԫ�أ�����ָ���Ԫ�صĵ�����
	template<class InputIter, class T>
	InputIter find(InputIter first, InputIter last, const T& value)
	{
		while (first != last && *first != value)
			first++;
		return first;
	}

	//find_if
	//��[first, last)�������ҵ���һ����һԪ���� unary_pred Ϊ true ��Ԫ�ز�����ָ���Ԫ�صĵ�����
	template <class InputIter, class UnaryPredicate>
	InputIter find_if(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		while (first != last && !unary_pred(*first))
			++first;
		return first;
	}

	// find_if_not
	// ��[first, last)�������ҵ���һ����һԪ���� unary_pred Ϊ false ��Ԫ�ز�����ָ���Ԫ�صĵ�����
	template <class InputIter, class UnaryPredicate>
	InputIter find_if_not(InputIter first, InputIter last, UnaryPredicate unary_pred)
	{
		while (first != last && unary_pred(*first))
			++first;
		return first;
	}

	// search
	// ��[first1, last1)�в���[first2, last2)���״γ��ֵ�
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

	// ���ذ汾ʹ���Զ��庯������ comp ����Ƚϲ���
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
	//��[first, last)�в��ҵ�һ����С�� value ��Ԫ�أ�������ָ�����ĵ���������û���򷵻� last
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
			mystl::advance(middle, half);//middle�������middle
			if (*middle < value)
			{
				first = middle;
				first++;
				len = len - half - 1;//��ǰ�벿����
			}
			else
			{
				len = half;//�ں�벿����
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
	//��[first,last)�����ڵ�Ԫ�ط�ת������ҿ���

	//reverse_dispatch��˫��������汾
	template <class BidirectionalIter>
	void reverse_dispatch(BidirectionalIter first, BidirectionalIter last,
		bidirectional_iterator_tag)
	{
		while (true)
		{
			if (first == last || first == --last)//����--last����Ϊ�ҿ�
				return;
			mystl::iter_swap(first++, last);//first++����Ϊ���
		}
	}
	//������ʵ������汾
	template<class RandomIter>
	void reverse_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
	{
		while (first < last)//������ʵ�����֧��<�����ķ���
			mystl::iter_swap(first++, --last);
	}

	//����
	template <class BidirectionalIter>
	void reverse(BidirectionalIter first, BidirectionalIter last)
	{
		mystl::reverse_dispatch(first, last, iterator_category(first));
	}


	//is_permutation
	//�ж�[first1,last1)�Ƿ�Ϊ[first2,last2)���������
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

		// ���ҳ���ͬ��ǰ׺��
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

		// �ж�ʣ�ಿ��
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
				// ���� *i �� [first2, last2) ����Ŀ
				auto c2 = 0;
				for (auto j = first2; j != last2; ++j)
				{
					if (pred(*i, *j))
						++c2;
				}
				if (c2 == 0)
					return false;

				// ���� *i �� [first1, last1) ����Ŀ
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