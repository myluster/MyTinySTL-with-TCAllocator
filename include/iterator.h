#pragma once
#include "type_traits.h"
#include <cstddef> //ptrdiff_t
namespace mystl {
	//���ֵ��������ͱ�ǩ
	struct input_iterator_tag {};//�����������ֻ�������α�����
	struct output_iterator_tag {};//�����������ֻд�����α�����
	struct forward_iterator_tag : public input_iterator_tag {};//ǰ����������ɶ�д��֧�ֶ�α�����
	struct bidirectional_iterator_tag : public forward_iterator_tag {};//˫���������֧��ǰ��ͺ����ƶ���
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};//������ʵ�������֧��ָ���������� it + n��
	//�̳й�ϵ��
	//���롢���->ǰ��->˫��->������Ӵ�С��
	//���㷨�У���δ�ػ�ĳ���������͵�ʵ�֣����������ؼ̳������ϲ���ƥ���ͨ�ð汾

	//iteratorģ��
	template<class Category, class T, class Distance = ptrdiff_t,
		class Pointer = T*, class Reference = T&>
	struct iterator
	{
		// ��ͬһ������͸���
		using iterator_category		= Category;// ���������
		using value_type			= T;// ��������ָ���������
		using difference_type		= Distance;// ��ʾ����������֮��ľ��룬Ҳ����������ʾһ���������������
		using pointer				= Pointer;// Ԫ�ص�ָ������
		using reference				= Reference;// Ԫ�ص���������
	};

	//������������ȡʵ��

	//�������T�Ƿ����iterator_category���ͳ�Ա
	namespace detail
	{
		template<class T>
		struct has_iterator_cat_impl
		{
		private:
			struct two { char a; char b; };//�������ֲ�ͬ���ص�ռλ����
			//SFINAE���滻ʧ�ܷǴ��󣩻���
			template<class U>
			static two test(...) {};//ͨ�û��˰汾������two���ͣ��ֽڴ�СΪ2
			//��������{}�ᱨ����VCR001
			//��ʵ���ϲ���Ҳ����ν����Ϊ��������ʹ����ģ��Ԫ��̣�sizeof������������ʵ�ʵ��ú���

			template<class U>
			static char test(typename U::iterator_category* = 0) {};//����ƥ��汾������char���ͣ��ֽڴ�СΪ1
		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(char);//ͨ����鷵��ֵ�Ĵ�С�����ж�
		};
	}

	//has_iterator_cat_impl<T>::valueΪtrueʱ��˵��T����iterator_category��Ա
	template<class T>
	struct has_iterator_cat: public integral_constant<bool, detail::has_iterator_cat_impl<T>::value>{ };

	//��ȡ���������Ե���ģ��
	template <class Iterator, bool has_cat = has_iterator_cat<Iterator>::value>
	struct iterator_traits_base {}; // default empty (for non-iterators or types without iterator_category)

	template <class Iterator>
	struct iterator_traits_base<Iterator, true>
	{
		// ֻ�е� Iterator �� iterator_category ʱ������ȡ���ڲ�����
		using iterator_category	= typename Iterator::iterator_category;
		using value_type		= typename Iterator::value_type;
		using pointer			= typename Iterator::pointer;
		using reference			= typename Iterator::reference;
		using difference_type	= typename Iterator::difference_type;
	};
	
	// ��Ҫ�� iterator_traits ģ��
	template <class Iterator>
	struct iterator_traits : public iterator_traits_base<Iterator> {};

	// ���ԭ��ָ����ػ��汾
	template <class T>
	struct iterator_traits<T*>
	{
		using iterator_category = random_access_iterator_tag; // ԭ��ָ�뱻��Ϊ������ʵ���
		using value_type		= T; // ָ��ָ������� 
		using pointer			= T*; // ָ������
		using reference			= T&; // ��������
		using difference_type	= ptrdiff_t; // ָ���ֵ����
	};

	template <class T>
	struct iterator_traits<const T*>
	{
		using iterator_category = random_access_iterator_tag;
		using value_type		= T;
		using pointer			= const T*;
		using reference			= const T&;
		using difference_type	= ptrdiff_t;
	};

	//��������T��iterator_category�Ƿ����ʽת��Ϊָ������U
	//���ж�T�Ƿ�����U����������
	template <class T, class U, bool IsIterator = has_iterator_cat<iterator_traits<T> >::value>
	struct has_iterator_cat_of : public integral_constant<bool, false> {};
	//has_iterator_cat<iterator_traits<T>>::valueȷ��ֻ�е�T�ǺϷ��������������� iterator_category��ʱ���Ž��к�����顣

	template <class T, class U>
	struct has_iterator_cat_of<T, U, true> : public integral_constant<bool,
		mystl::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> {};


	//�ػ��ж�
	//�����Ͽ�������ֻҪ������ע��һ�¾���
	template <class Iter>
	struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

	template <class Iter>
	struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

	template <class Iter>
	struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

	template <class Iter>
	struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

	template <class Iter>
	struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

	//�ж��ǲ��ǵ�����
	template <class Iterator>
	struct is_iterator :
		public integral_constant<bool, is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value>
	{
	};
	//�����ĸ����ն���input��output������

	//��ȡĳ����������category
	template <class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&)
	{
		using Category = typename iterator_traits<Iterator>::iterator_category;
		return Category();
	}

	//��ȡĳ����������value_type
	//����һ����ָ�룬���������Ƶ�
	template <class Iterator>
	inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	//distance �����������ľ���
	//input_iterator_tag�汾(��ͨ�ã����� forward �� bidirectional)
	template <class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag)
	{
		typename iterator_traits<InputIterator>::difference_type n = 0;
		while (first != last)
		{
			++first;
			++n;
		}
		return n;
	}

	//random_access_iterator_tag�汾
	template <class RandomIter>
	inline typename iterator_traits<RandomIter>::difference_type distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
	{
		return last - first;
	}

	template <class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type distance(InputIterator first, InputIterator last)
	{
		//ͳһ
		return distance_dispatch(first, last, iterator_category(first));
	}

	//advance �õ�����ǰ��n������
	//input_iterator_tag�汾 (��ͨ�ã����� forward �� bidirectional)
	template <class InputIterator, class Distance>
	inline void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
	{
		while (n--)
			++i;
	}
	//random_access_iterator_tag�汾
	template <class RandomIter, class Distance>
	inline void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag)
	{
		i += n;
	}

	template <class InputIterator, class Distance>
	inline void advance(InputIterator& i, Distance n)
	{
		//ͳһ
		advance_dispatch(i, n, iterator_category(i));
	}



	//ģ���ࣺreverse_iterator���������
	//ǰ��Ϊ���ˣ�����Ϊǰ��
	//�������������
	template<class Iterator>
	class reverse_iterator
	{
	private:
		Iterator current;//��Ӧ�����������
	public:
		//�����������������Ӧ�ͱ�
		using iterator_category = typename iterator_traits<Iterator>::iterator_category;
		using value_type		= typename iterator_traits<Iterator>::value_type;
		using difference_type	= typename iterator_traits<Iterator>::difference_type;
		using pointer			= typename iterator_traits<Iterator>::pointer;
		using reference			= typename iterator_traits<Iterator>::reference;

		using iterator_type		= Iterator;
		using self				= reverse_iterator<Iterator>;

		//���캯��
		reverse_iterator():current() {}//c++11������ʽĬ�Ϲ���
		explicit reverse_iterator(iterator_type i) :current(i) {}
		reverse_iterator(const self& rhs) :current(rhs.current) {}

		//ȡ����Ӧ�����������
		iterator_type base() const
		{
			return current;
		}

		//���ز�����
		//���γ���Ա�����Բ������߽紦��
		reference operator*() const
		{ // ʵ�ʶ�Ӧ�����������ǰһ��λ��
			auto tmp = current;
			return *--tmp;
		}
		pointer operator->() const
		{
			return &(operator*());
		}
		// ǰ��(++)��Ϊ����(--)
		self& operator++()//ǰ��
		{
			--current;
			return *this;
		}
		self operator++(int)//����
		{
			self tmp = *this;
			--current;
			return tmp;
		}
		// ����(--)��Ϊǰ��(++)
		self& operator--()
		{
			++current;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			++current;
			return tmp;
		}

		self& operator+=(difference_type n)
		{
			current -= n;
			return *this;
		}
		self operator+(difference_type n) const
		{
			return self(current - n);
		}
		self& operator-=(difference_type n)
		{
			current += n;
			return *this;
		}
		self operator-(difference_type n) const
		{
			return self(current + n);
		}

		reference operator[](difference_type n) const
		{
			return *(*this + n);
		}
	};

	//ͬ���ͱȽ�
	//���� operator-
	//�ǳ�Ա����
	template <class Iterator>
	typename reverse_iterator<Iterator>::difference_type
		operator-(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
	{
		return rhs.base() - lhs.base();
	}

	//���رȽϲ�����
	//����ʵ�����ھ��������.h��

	//�칹��ͬ��ʹ��ͬһ���Ƚϰ汾
	template <class Iterator1, class Iterator2>
	bool operator==(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		// �Ƚ����ǵĻ�������
		// ������ʽ�������ڵײ����������֮���ǿɱȽϵ�
		return lhs.base() == rhs.base();
	}

	template <class Iterator1, class Iterator2>
	typename mystl::reverse_iterator<Iterator1>::difference_type
		operator-(const mystl::reverse_iterator<Iterator1>& lhs,
			const mystl::reverse_iterator<Iterator2>& rhs)
	{
		return rhs.base() - lhs.base();
	}

	template <class Iterator1, class Iterator2>
	bool operator<(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		return rhs.base() < lhs.base();
	}

	template <class Iterator1, class Iterator2>
	bool operator!=(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class Iterator1, class Iterator2>
	bool operator<=(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class Iterator1, class Iterator2>
	bool operator>=(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		return !(lhs < rhs);
	}
} // namespace mystl
