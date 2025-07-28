#pragma once
#include "type_traits.h"
#include <cstddef> //ptrdiff_t
namespace mystl {
	//五种迭代器类型标签
	struct input_iterator_tag {};//输入迭代器（只读，单次遍历）
	struct output_iterator_tag {};//输出迭代器（只写，单次遍历）
	struct forward_iterator_tag : public input_iterator_tag {};//前向迭代器（可读写，支持多次遍历）
	struct bidirectional_iterator_tag : public forward_iterator_tag {};//双向迭代器（支持前向和后向移动）
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};//随机访问迭代器（支持指针算术，如 it + n）
	//继承关系：
	//输入、输出->前向->双向->随机（从大到小）
	//在算法中，若未特化某迭代器类型的实现，编译器会沿继承链向上查找匹配的通用版本

	//iterator模板
	template<class Category, class T, class Distance = ptrdiff_t,
		class Pointer = T*, class Reference = T&>
	struct iterator
	{
		// 让同一别名穿透多层
		using iterator_category		= Category;// 迭代器类别
		using value_type			= T;// 迭代器所指对象的类型
		using difference_type		= Distance;// 表示两个迭代器之间的距离，也可以用来表示一个容器的最大容量
		using pointer				= Pointer;// 元素的指针类型
		using reference				= Reference;// 元素的引用类型
	};

	//迭代器特性萃取实现

	//检测类型T是否包含iterator_category类型成员
	namespace detail
	{
		template<class T>
		struct has_iterator_cat_impl
		{
		private:
			struct two { char a; char b; };//用于区分不同重载的占位类型
			//SFINAE（替换失败非错误）机制
			template<class U>
			static two test(...) {};//通用回退版本，返回two类型，字节大小为2
			//若不加上{}会报警告VCR001
			//但实际上不加也无所谓，因为我们这里使用了模板元编程，sizeof操作符并不会实际调用函数

			template<class U>
			static char test(typename U::iterator_category* = 0) {};//条件匹配版本，返回char类型，字节大小为1
		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(char);//通过检查返回值的大小进行判断
		};
	}

	//has_iterator_cat_impl<T>::value为true时，说明T包含iterator_category成员
	template<class T>
	struct has_iterator_cat: public integral_constant<bool, detail::has_iterator_cat_impl<T>::value>{ };

	//萃取迭代器特性的主模板
	template <class Iterator, bool has_cat = has_iterator_cat<Iterator>::value>
	struct iterator_traits_base {}; // default empty (for non-iterators or types without iterator_category)

	template <class Iterator>
	struct iterator_traits_base<Iterator, true>
	{
		// 只有当 Iterator 有 iterator_category 时，才提取其内部类型
		using iterator_category	= typename Iterator::iterator_category;
		using value_type		= typename Iterator::value_type;
		using pointer			= typename Iterator::pointer;
		using reference			= typename Iterator::reference;
		using difference_type	= typename Iterator::difference_type;
	};
	
	// 主要的 iterator_traits 模板
	template <class Iterator>
	struct iterator_traits : public iterator_traits_base<Iterator> {};

	// 针对原生指针的特化版本
	template <class T>
	struct iterator_traits<T*>
	{
		using iterator_category = random_access_iterator_tag; // 原生指针被视为随机访问迭代
		using value_type		= T; // 指针指向的类型 
		using pointer			= T*; // 指针类型
		using reference			= T&; // 引用类型
		using difference_type	= ptrdiff_t; // 指针差值类型
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

	//检测迭代器T的iterator_category是否可隐式转换为指定类型U
	//即判断T是否属于U或其派生类
	template <class T, class U, bool IsIterator = has_iterator_cat<iterator_traits<T> >::value>
	struct has_iterator_cat_of : public integral_constant<bool, false> {};
	//has_iterator_cat<iterator_traits<T>>::value确保只有当T是合法迭代器（定义了 iterator_category）时，才进行后续检查。

	template <class T, class U>
	struct has_iterator_cat_of<T, U, true> : public integral_constant<bool,
		mystl::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> {};


	//特化判断
	//理论上可以随便填，只要来这里注册一下就行
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

	//判断是不是迭代器
	template <class Iterator>
	struct is_iterator :
		public integral_constant<bool, is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value>
	{
	};
	//无论哪个最终都是input、output的派生

	//提取某个迭代器的category
	template <class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&)
	{
		using Category = typename iterator_traits<Iterator>::iterator_category;
		return Category();
	}

	//提取某个迭代器的value_type
	//返回一个空指针，用于类型推导
	template <class Iterator>
	inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	//distance 计算迭代器间的距离
	//input_iterator_tag版本(最通用，包括 forward 和 bidirectional)
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

	//random_access_iterator_tag版本
	template <class RandomIter>
	inline typename iterator_traits<RandomIter>::difference_type distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag)
	{
		return last - first;
	}

	template <class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type distance(InputIterator first, InputIterator last)
	{
		//统一
		return distance_dispatch(first, last, iterator_category(first));
	}

	//advance 让迭代器前进n个距离
	//input_iterator_tag版本 (最通用，包括 forward 和 bidirectional)
	template <class InputIterator, class Distance>
	inline void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag)
	{
		while (n--)
			++i;
	}
	//random_access_iterator_tag版本
	template <class RandomIter, class Distance>
	inline void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag)
	{
		i += n;
	}

	template <class InputIterator, class Distance>
	inline void advance(InputIterator& i, Distance n)
	{
		//统一
		advance_dispatch(i, n, iterator_category(i));
	}



	//模板类：reverse_iterator反向迭代器
	//前进为后退，后退为前进
	//依赖正向迭代器
	template<class Iterator>
	class reverse_iterator
	{
	private:
		Iterator current;//对应的正向迭代器
	public:
		//反向迭代器的五种相应型别
		using iterator_category = typename iterator_traits<Iterator>::iterator_category;
		using value_type		= typename iterator_traits<Iterator>::value_type;
		using difference_type	= typename iterator_traits<Iterator>::difference_type;
		using pointer			= typename iterator_traits<Iterator>::pointer;
		using reference			= typename iterator_traits<Iterator>::reference;

		using iterator_type		= Iterator;
		using self				= reverse_iterator<Iterator>;

		//构造函数
		reverse_iterator():current() {}//c++11允许显式默认构造
		explicit reverse_iterator(iterator_type i) :current(i) {}
		reverse_iterator(const self& rhs) :current(rhs.current) {}

		//取出对应的正向迭代器
		iterator_type base() const
		{
			return current;
		}

		//重载操作符
		//信任程序员，所以不会做边界处理
		reference operator*() const
		{ // 实际对应正向迭代器的前一个位置
			auto tmp = current;
			return *--tmp;
		}
		pointer operator->() const
		{
			return &(operator*());
		}
		// 前进(++)变为后退(--)
		self& operator++()//前置
		{
			--current;
			return *this;
		}
		self operator++(int)//后置
		{
			self tmp = *this;
			--current;
			return tmp;
		}
		// 后退(--)变为前进(++)
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

	//同类型比较
	//重载 operator-
	//非成员函数
	template <class Iterator>
	typename reverse_iterator<Iterator>::difference_type
		operator-(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs)
	{
		return rhs.base() - lhs.base();
	}

	//重载比较操作符
	//具体实现是在具体的容器.h中

	//异构和同构使用同一个比较版本
	template <class Iterator1, class Iterator2>
	bool operator==(const mystl::reverse_iterator<Iterator1>& lhs,
		const mystl::reverse_iterator<Iterator2>& rhs)
	{
		// 比较它们的基迭代器
		// 这里隐式地依赖于底层迭代器类型之间是可比较的
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
