﻿#pragma once
#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "utils.h"
#include "exceptdef.h"
#include "allocator.h"
//list:双向循环链表

namespace mystl
{
	//前置声明模板类
	template<class T> struct list_node_base;
	template<class T> struct list_node;
	template<class T>
	//类型萃取模板
	struct node_traits
	{
		//别名
		typedef list_node_base<T>* base_ptr;
		typedef list_node<T>* node_ptr;
	};

	//list的结点结构(基类)
	template<class T>
	struct list_node_base
	{
		//别名
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;

		base_ptr prev;//前一结点
		base_ptr next;//下一结点

		list_node_base() = default;//使用默认构造函数

		node_ptr as_node()
		{
			return static_cast<node_ptr>(self());//基类转派生类
		}

		void unlink()
		{
			prev = next = self();//将结点变成自环(移除结点)
		}

		base_ptr self()
		{
			return this;
		}
	};
	//派生类带有数据
	template<class T>
	struct list_node :public list_node_base<T>
	{
		typedef typename node_traits<T>::base_ptr base_ptr;
		typedef typename node_traits<T>::node_ptr node_ptr;

		T value;//数据域

		list_node() = default;
		list_node(const T& v) :value(v) {}//拷贝构造
		list_node(T&& v) :value(mystl::move(v)) {}//移动构造

		base_ptr as_base()
		{
			return static_cast<base_ptr>(this);//转化为基类
		}
		node_ptr self()
		{
			return this;
		}
	};

	//list的迭代器设计
	template<class T>
	struct list_iterator : public iterator<mystl::bidirectional_iterator_tag, T>
	{
		//代称
		typedef T									value_type;
		typedef T* pointer;
		typedef T& reference;
		typedef typename node_traits<T>::base_ptr	base_ptr;
		typedef typename node_traits<T>::node_ptr	node_ptr;
		typedef list_iterator<T>					self;

		base_ptr node_;//当前结点

		//构造函数
		list_iterator() = default;
		list_iterator(base_ptr x) :node_(x) {}
		list_iterator(node_ptr x) :node_(x->as_base()) {}
		list_iterator(const list_iterator& rhs) :node_(rhs.node_) {}

		//重载操作符
		reference operator*()const
		{
			return node_->as_node()->value;//operator*()的返回值通常是被管理的数据引用
		}
		pointer operator->()const
		{
			return &(operator*());
		}

		self& operator++()
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->next;
			return *this;
		}

		self operator++(int)
		{
			self tmp = *this;
			++*this;
			return tmp;
		}

		self& operator--()//前置
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->prev;
			return *this;
		}

		self operator--(int)//后置
		{
			self tmp = *this;//取值
			--*this;
			return tmp;
		}
		//为什么后置不返回self&
		//临时对象tmp在函数结束时被销毁所以不返回引用

		bool operator==(const self& rhs)const
		{
			return node_ == rhs.node_;
		}
		bool operator!=(const self& rhs)const
		{
			return node_ != rhs.node_;
		}
	};

	//const版本
	template <class T>
	struct list_const_iterator : public iterator<bidirectional_iterator_tag, T>
	{
		typedef T									value_type;
		typedef const T* pointer;//const引用
		typedef const T& reference;//const指针
		typedef typename node_traits<T>::base_ptr	base_ptr;
		typedef typename node_traits<T>::node_ptr	node_ptr;
		typedef list_const_iterator<T>				self;

		base_ptr node_;

		list_const_iterator() = default;
		list_const_iterator(base_ptr x)
			:node_(x) {
		}
		list_const_iterator(node_ptr x)
			:node_(x->as_base()) {
		}
		list_const_iterator(const list_iterator<T>& rhs)//支持隐式转换
			:node_(rhs.node_) {
		}
		list_const_iterator(const list_const_iterator& rhs)
			:node_(rhs.node_) {
		}

		reference operator*()  const { return node_->as_node()->value; }
		pointer   operator->() const { return &(operator*()); }

		self& operator++()
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->next;
			return *this;
		}
		self operator++(int)
		{
			self tmp = *this;
			++*this;
			return tmp;
		}
		self& operator--()
		{
			MYSTL_DEBUG(node_ != nullptr);
			node_ = node_->prev;
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			--*this;
			return tmp;
		}

		// 重载比较操作符
		bool operator==(const self& rhs) const { return node_ == rhs.node_; }
		bool operator!=(const self& rhs) const { return node_ != rhs.node_; }
	};

	template<class T>
	class list
	{
	public:
		// list 的嵌套型别定义
		typedef mystl::allocator<T>                      allocator_type;
		typedef mystl::allocator<T>                      data_allocator;
		typedef mystl::allocator<list_node_base<T>>      base_allocator;
		typedef mystl::allocator<list_node<T>>           node_allocator;

		typedef typename allocator_type::value_type      value_type;
		typedef typename allocator_type::pointer         pointer;
		typedef typename allocator_type::const_pointer   const_pointer;
		typedef typename allocator_type::reference       reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename allocator_type::size_type       size_type;
		typedef typename allocator_type::difference_type difference_type;

		typedef list_iterator<T>                         iterator;
		typedef list_const_iterator<T>                   const_iterator;
		typedef mystl::reverse_iterator<iterator>        reverse_iterator;
		typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;

		typedef typename node_traits<T>::base_ptr        base_ptr;
		typedef typename node_traits<T>::node_ptr        node_ptr;

		allocator_type get_allocator() { return node_allocator(); }
	private:
		base_ptr	node_;//指向末尾节点
		//list是循环的end->begin
		size_type	size_;//大小
	public:
		list()
		{
			fill_init(0, value_type());
		}

		explicit list(size_type n)
		{
			fill_init(n, value_type());
		}

		list(size_type n, const T& value)
		{
			fill_init(n, value);
		}

		template <class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
		list(Iter first, Iter last)
		{
			copy_init(first, last);
		}

		list(std::initializer_list<T> ilist)
		{
			copy_init(ilist.begin(), ilist.end());
		}

		list(const list& rhs)
		{
			copy_init(rhs.cbegin(), rhs.cend());
		}

		list(list&& rhs) noexcept :node_(rhs.node_), size_(rhs.size_)
		{
			rhs.node_ = base_allocator::allocate(1);
			rhs.node_->unlink();
			rhs.size_ = 0;
		}

		list& operator=(const list& rhs)
		{
			if (this != &rhs)
			{
				assign(rhs.begin(), rhs.end());
			}
			return *this;
		}

		list& operator=(list&& rhs) noexcept
		{
			clear();
			splice(end(), rhs);
			return *this;
		}

		list& operator=(std::initializer_list<T> ilist)//用于支持使用花括号构造列表
		{
			list tmp(ilist.begin(), ilist.end());
			swap(tmp);
			return *this;
		}

		~list()
		{
			if (node_)
			{
				clear();
				base_allocator::deallocate(node_);
				node_ = nullptr;
				size_ = 0;
			}
		}

	public:
		//迭代器相关操作
		//操作	非 const 对象	const 对象
		//begin()	返回 iterator	返回 const_iterator
		//cbegin()	返回 const_iterator	返回 const_iterator
		iterator               begin()         noexcept
		{
			return node_->next;
		}
		const_iterator         begin()   const noexcept
		{
			return node_->next;
		}
		iterator               end()           noexcept
		{
			return node_;
		}
		const_iterator         end()     const noexcept
		{
			return node_;
		}
		//注意是循环链表

		//反向迭代器相关操作
		reverse_iterator       rbegin()        noexcept
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator rbegin()  const noexcept
		{
			return const_reverse_iterator(end());
		}
		reverse_iterator       rend()          noexcept
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator rend()    const noexcept
		{
			return const_reverse_iterator(begin());
		}

		//支持标准库接口
		//无论对象是否 const​​，cbegin() 始终返回 const_iterator，明确表达只读意图
		const_iterator         cbegin()  const noexcept
		{
			return begin();
		}
		const_iterator         cend()    const noexcept
		{
			return end();
		}
		const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}
		const_reverse_iterator crend()   const noexcept
		{
			return rend();
		}

		//容量相关操作
		bool      empty()    const noexcept
		{
			return node_->next == node_;
		}

		size_type size()     const noexcept
		{
			return size_;
		}

		//返回理论上的最大容量
		size_type max_size() const noexcept
		{
			return static_cast<size_type>(-1);
		}
		//核心原理：
		//在二进制补码表示中-1为全1
		//将-1转换为无符号整数时，结果为该类型的最大值。

		//访问元素相关操作
		reference       front()
		{
			MYSTL_DEBUG(!empty());
			return *begin();
		}

		const_reference front() const
		{
			MYSTL_DEBUG(!empty());
			return *begin();
		}

		//最后一个元素，注意与end区分
		reference       back()
		{
			MYSTL_DEBUG(!empty());
			return *(--end());
		}

		const_reference back()  const
		{
			MYSTL_DEBUG(!empty());
			return *(--end());
		}

		//调整容器相关操作
		//assign
		void	assign(size_type n, const value_type& value)
		{
			fill_assign(n, value);
		}

		template<class Iter, typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
		void assign(Iter first, Iter last)
		{
			copy_assign(first, last);
		}

		void assign(std::initializer_list<T> ilist)
		{
			copy_assign(ilist.begin(), ilist.end());
		}

		//emplace_front
		template <class ...Args>
		void emplace_front(Args&& ...args)//可变长度用来构造一个元素
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(mystl::forward<Args>(args)...);
			link_nodes_at_front(link_node->as_base(), link_node->as_base());//只插入一个，first与last一样
			++size_;
		}

		template<class ...Args>
		void emplace_back(Args&& ...args)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(mystl::forward<Args>(args)...);
			link_nodes_at_back(link_node->as_base(), link_node->as_base());
			++size_;
		}

		template<class ...Args>
		iterator emplace(const_iterator pos, Args&& ...args)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() = 1, "list<T>'s size too big");
			auto link_node = create_node(mystl::forward<Args>(args)...);
			link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
			size_++;
			return iterator(link_node);
		}

		//insert
		iterator insert(const_iterator pos, const value_type& value)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(value);
			++size_;
			return link_iter_node(pos, link_node->as_base());
		}

		iterator insert(const_iterator pos, value_type&& value)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(mystl::move(value));
			++size_;
			return link_iter_node(pos, link_node->as_base());
		}

		iterator insert(const_iterator pos, size_type n, const value_type& value)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
			return fill_insert(pos, n, value);
		}

		template <class Iter, typename std::enable_if<
			mystl::is_input_iterator<Iter>::value, int>::type = 0>
		iterator insert(const_iterator pos, Iter first, Iter last)
		{
			size_type n = mystl::distance(first, last);
			THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
			return copy_insert(pos, n, first);
		}

		// push_front / push_back

		void push_front(const value_type& value)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(value);
			link_nodes_at_front(link_node->as_base(), link_node->as_base());
			++size_;
		}

		void push_front(value_type&& value)
		{
			emplace_front(mystl::move(value));
		}

		void push_back(const value_type& value)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
			auto link_node = create_node(value);
			link_nodes_at_back(link_node->as_base(), link_node->as_base());
			++size_;
		}

		void push_back(value_type&& value)
		{
			emplace_back(mystl::move(value));
		}

		//pop_front/pop_back
		void pop_front()
		{
			MYSTL_DEBUG(!empty());
			auto n = node_->next;
			unlink_nodes(n, n);//断连最后一个数据
			destroy_node(n->as_node());//回收
			--size_;
			//node_作为哨兵结点不变
		}

		void pop_back()
		{
			MYSTL_DEBUG(!empty());
			auto n = node_->prev;
			unlink_nodes(n, n);
			destroy_node(n->as_node());
			--size_;
		}

		//erase/clear
		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		void clear();

		//resize
		void resize(size_type new_size)
		{
			resize(new_size, value_type());
		}
		void resize(size_type new_size, const value_type& value);

		void swap(list& rhs)noexcept
		{
			mystl::swap(node_, rhs.node_);
			mystl::swap(size_, rhs.size_);
		}

		// list 相关操作
		//切片操作
		void splice(const_iterator pos, list& other);
		void splice(const_iterator pos, list& other, const_iterator it);
		void splice(const_iterator pos, list& other, const_iterator first, const_iterator last);

		//移除链表中所有值等于value的元素
		void remove(const value_type& value)
		{
			remove_if([&](const value_type& v) {return v == value; });
			//使用了lambda表达式作为remove_if函数的参数
			//Lambda 表达式的一般形式为：
			//[捕获列表] (参数列表) -> 返回类型 { 函数体 }
			//[&]，表示以​​引用方式捕获外部变量​​
			//​​参数列表​​：(const value_type& v),表示接受一个const引用类型的参数v
			//返回类型​​：省略，编译器自动推导为 bool（因为函数体返回v == value的布尔结果）
			//函数体​​：{ return v == value; }，判断参数 v 是否等于外部变量 value
		}
		template <class UnaryPredicate>
		void remove_if(UnaryPredicate pred);

		void unique()
		{
			unique(mystl::equal_to<T>());
		}
		template <class BinaryPredicate>
		void unique(BinaryPredicate pred);

		//合并另一个链表到当前链表并排序
		void merge(list& x)
		{
			merge(x, mystl::less<T>());
		}
		template<class Compare>
		void merge(list& x, Compare comp);

		//对当前链表内部排序
		void sort()
		{
			list_sort(begin(), end(), size(), mystl::less<T>());
		}
		template <class Compared>
		void sort(Compared comp)
		{
			list_sort(begin(), end(), size(), comp);
		}

		void reverse();

	private:
		//helper function

		//create/destroy node
		template<class ...Args>
		node_ptr create_node(Args&&...args);
		void destroy_node(node_ptr p);

		//initialize
		void fill_init(size_type n, const value_type& value);
		template<class Iter>
		void copy_init(Iter first, Iter last);

		//link/unlink
		iterator link_iter_node(const_iterator pos, base_ptr node);
		void link_nodes(base_ptr p, base_ptr first, base_ptr last);
		void link_nodes_at_front(base_ptr first, base_ptr last);
		void link_nodes_at_back(base_ptr first, base_ptr last);
		void unlink_nodes(base_ptr first, base_ptr last);

		//assign
		void fill_assign(size_type n, const value_type& value);
		template<class Iter>
		void copy_assign(Iter first, Iter last);

		// insert
		iterator  fill_insert(const_iterator pos, size_type n, const value_type& value);
		template <class Iter>
		iterator  copy_insert(const_iterator pos, size_type n, Iter first);

		// sort
		template <class Compared>
		iterator  list_sort(iterator first, iterator last, size_type n, Compared comp);
	};
	//删除pos处的元素
	template<class T>
	typename list<T>::iterator list<T>::erase(const_iterator pos)
	{
		MYSTL_DEBUG(pos != cend());
		auto n = pos.node_;
		auto next = n->next;
		unlink_nodes(n, n);
		destroy_node(n->as_node());
		size_--;
		return iterator(next);
	}

	//删除[first,last)内的元素
	template<class T>
	typename list<T>::iterator list<T>::erase(const_iterator first, const_iterator last)
	{
		if (first != last)
		{
			unlink_nodes(first.node_, last.node_->prev);
			while (first != last)
			{
				auto cur = first.node_;
				++first;
				destroy_node(cur->as_node());
				--size_;
			}
		}
		return iterator(last.node_);
	}

	// 清空 list
	template <class T>
	void list<T>::clear()
	{
		if (size_ != 0)
		{
			auto cur = node_->next;
			for (base_ptr next = cur->next; cur != node_; cur = next, next = cur->next)
			{
				destroy_node(cur->as_node());
			}
			node_->unlink();//end变为自环
			size_ = 0;
		}
	}

	// 重置容器大小
	template <class T>
	void list<T>::resize(size_type new_size, const value_type& value)
	{
		auto i = begin();
		size_type len = 0;
		while (i != end() && len < new_size)
		{
			++i;
			++len;
		}
		//少补多删
		if (len == new_size)
		{
			erase(i, node_);
		}
		else
		{
			insert(node_, new_size - len, value);
		}
	}

	// 将 list x 接合于 pos 之前
	template <class T>
	void list<T>::splice(const_iterator pos, list& x)
	{
		MYSTL_DEBUG(this != &x);
		if (!x.empty())
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_, "list<T>'s size too big");

			auto f = x.node_->next;
			auto l = x.node_->prev;

			x.unlink_nodes(f, l);
			link_nodes(pos.node_, f, l);

			size_ += x.size_;
			x.size_ = 0;
		}
	}

	// 将 it 所指的节点接合于 pos 之前
	template <class T>
	void list<T>::splice(const_iterator pos, list& x, const_iterator it)
	{
		if (pos.node_ != it.node_ && pos.node_ != it.node_->next)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");

			auto f = it.node_;

			x.unlink_nodes(f, f);
			link_nodes(pos.node_, f, f);

			++size_;
			--x.size_;
		}
	}

	// 将 list x 的 [first, last) 内的节点接合于 pos 之前
	template <class T>
	void list<T>::splice(const_iterator pos, list& x, const_iterator first, const_iterator last)
	{
		if (first != last && this != &x)
		{
			size_type n = mystl::distance(first, last);
			THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
			auto f = first.node_;
			auto l = last.node_->prev;

			x.unlink_nodes(f, l);
			link_nodes(pos.node_, f, l);

			size_ += n;
			x.size_ -= n;
		}
	}


	//将另一元操作pred为true的所有元素移除
	template<class T>
	template<class UnaryPredicate>
	void list<T>::remove_if(UnaryPredicate pred)//pred是一个函数
	{
		auto f = begin();
		auto l = end();
		for (auto next = f; f != l; f = next)
		{
			next++;
			if (pred(*f))//调用 pred，并将*f的值作为参数传递给pred
			{
				erase(f);
			}
		}
	}

	// 移除 list 中满足 pred 为 true 重复元素
	template <class T>
	template <class BinaryPredicate>
	void list<T>::unique(BinaryPredicate pred)
	{
		auto i = begin();
		auto e = end();
		auto j = i;
		++j;
		while (j != e)
		{
			if (pred(*i, *j))//比较相邻元素
			{
				erase(j);
			}
			else
			{
				i = j;
			}
			j = i;
			++j;
		}
	}

	// 与另一个 list 合并，按照 comp 为 true 的顺序
	template <class T>
	template <class Compare>
	void list<T>::merge(list& x, Compare comp)
	{
		if (this != &x)
		{
			THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_, "list<T>'s size too big");

			auto f1 = begin();
			auto l1 = end();
			auto f2 = x.begin();
			auto l2 = x.end();

			while (f1 != l1 && f2 != l2)
			{
				if (comp(*f2, *f1))
				{
					// 使 comp 为 true 的一段区间
					auto next = f2;
					++next;
					for (; next != l2 && comp(*next, *f1); ++next);//什么都不做，只是为了到达l
					auto f = f2.node_;
					auto l = next.node_->prev;//左闭右开
					f2 = next;

					// link node
					x.unlink_nodes(f, l);
					link_nodes(f1.node_, f, l);
					++f1;
				}
				else
				{
					++f1;
				}
			}
			// 连接剩余部分
			if (f2 != l2)
			{
				auto f = f2.node_;
				auto l = l2.node_->prev;
				x.unlink_nodes(f, l);
				link_nodes(l1.node_, f, l);
			}

			size_ += x.size_;
			x.size_ = 0;
		}
	}

	// 将 list 反转
	template <class T>
	void list<T>::reverse()
	{
		if (size_ <= 1)
		{
			return;
		}
		auto i = begin();
		auto e = end();
		while (i.node_ != e.node_)
		{
			mystl::swap(i.node_->prev, i.node_->next);
			i.node_ = i.node_->prev;
		}
		mystl::swap(e.node_->prev, e.node_->next);
	}



	//helper function
	//创建节点
	template <class T>
	template<class...Args>
	typename list<T>::node_ptr list<T>::create_node(Args&&...args)
	{
		node_ptr p = node_allocator::allocate(1);//分配一个node
		try
		{
			data_allocator::construct(mystl::addressof(p->value), mystl::forward<Args>(args)...);//相当于直接写上去构造
			p->prev = nullptr;
			p->next = nullptr;
		}
		catch (...)
		{
			node_allocator::deallocate(p);
			throw;
		}
		return p;
	}

	//销毁结点
	template<class T>
	void list<T>::destroy_node(node_ptr p)
	{
		data_allocator::destroy(mystl::addressof(p->value));//析构数据
		node_allocator::deallocate(p);
	}

	// 用 n 个元素初始化容器
	template <class T>
	void list<T>::fill_init(size_type n, const value_type& value)
	{
		node_ = base_allocator::allocate(1);
		node_->unlink();
		size_ = n;
		try
		{
			for (; n > 0; --n)
			{
				auto node = create_node(value);
				link_nodes_at_back(node->as_base(), node->as_base());
			}
		}
		catch (...)
		{
			clear();
			base_allocator::deallocate(node_);
			node_ = nullptr;
			throw;
		}
	}

	// 以 [first, last) 初始化容器
	template <class T>
	template <class Iter>
	void list<T>::copy_init(Iter first, Iter last)
	{
		node_ = base_allocator::allocate(1);
		node_->unlink();
		size_type n = mystl::distance(first, last);
		size_ = n;
		try
		{
			for (; n > 0; --n, ++first)
			{
				auto node = create_node(*first);
				link_nodes_at_back(node->as_base(), node->as_base());
			}
		}
		catch (...)
		{
			clear();
			base_allocator::deallocate(node_);
			node_ = nullptr;
			throw;
		}
	}

	//在pos处连接一个节点
	template <class T>
	typename list<T>::iterator
		list<T>::link_iter_node(const_iterator pos, base_ptr link_node)
	{
		//头尾特殊处理
		if (pos == node_->next)
		{
			link_nodes_at_front(link_node, link_node);
		}
		else if (pos == node_)
		{
			link_nodes_at_back(link_node, link_node);
		}
		else
		{
			link_nodes(pos.node_, link_node, link_node);
		}
		return iterator(link_node);
	}


	//在pos处连接[first,last]的结点
	template<class T>
	void list<T>::link_nodes(base_ptr pos, base_ptr first, base_ptr last)
	{
		pos->prev->next = first;
		first->prev = pos->prev;
		pos->prev = last;
		last->next = pos;
	}

	//在头部连接[first,last]结点
	template<class T>
	void list<T>::link_nodes_at_front(base_ptr first, base_ptr last)
	{
		first->prev = node_;
		last->next = node_->next;
		last->next->prev = last;//原本指向begin
		node_->next = first;//改结尾
	}

	template<class T>
	void list<T>::link_nodes_at_back(base_ptr first, base_ptr last)
	{
		last->next = node_;
		first->prev = node_->prev;
		first->prev->next = first;//原本指向node_
		node_->prev = last;
	}

	//容器与[first,last]结点断开
	template<class T>
	void list<T>::unlink_nodes(base_ptr first, base_ptr last)
	{
		first->prev->next = last->next;
		last->next->prev = first->prev;
	}

	// 用 n 个元素为容器赋值
	template <class T>
	void list<T>::fill_assign(size_type n, const value_type& value)
	{
		auto i = begin();
		auto e = end();
		for (; n > 0 && i != e; --n, ++i)
		{
			*i = value;
		}
		if (n > 0)
		{
			insert(e, n, value);
		}
		else
		{
			erase(i, e);
		}
	}

	// 复制[f2, l2)为容器赋值
	template <class T>
	template <class Iter>
	void list<T>::copy_assign(Iter f2, Iter l2)
	{
		auto f1 = begin();
		auto l1 = end();
		for (; f1 != l1 && f2 != l2; ++f1, ++f2)
		{
			*f1 = *f2;//值的等号是赋值（复制构造）
		}
		if (f2 == l2)
		{
			erase(f1, l1);
		}
		else
		{
			insert(l1, f2, l2);
		}
	}

	// 在 pos 处插入 n 个元素
	template <class T>
	typename list<T>::iterator
		list<T>::fill_insert(const_iterator pos, size_type n, const value_type& value)
	{
		iterator r(pos.node_);
		if (n != 0)
		{
			const auto add_size = n;
			auto node = create_node(value);
			node->prev = nullptr;
			r = iterator(node);
			iterator end = r;
			try
			{
				// 前面已经创建了一个节点，还需 n - 1 个
				for (--n; n > 0; --n, ++end)
				{
					auto next = create_node(value);
					end.node_->next = next->as_base();  // link node
					next->prev = end.node_;
				}
				size_ += add_size;
			}
			catch (...)
			{
				auto enode = end.node_;
				while (true)
				{
					auto prev = enode->prev;
					destroy_node(enode->as_node());
					if (prev == nullptr)
						break;
					enode = prev;
				}
				throw;
			}
			link_nodes(pos.node_, r.node_, end.node_);
		}
		return r;
	}

	// 在 pos 处插入 [first, last) 的元素
	template <class T>
	template <class Iter>
	typename list<T>::iterator
		list<T>::copy_insert(const_iterator pos, size_type n, Iter first)
	{
		iterator r(pos.node_);
		if (n != 0)
		{
			const auto add_size = n;
			auto node = create_node(*first);
			node->prev = nullptr;
			r = iterator(node);
			iterator end = r;
			try
			{
				for (--n, ++first; n > 0; --n, ++first, ++end)
				{
					auto next = create_node(*first);
					end.node_->next = next->as_base();  // link node
					next->prev = end.node_;
				}
				size_ += add_size;
			}
			catch (...)
			{
				auto enode = end.node_;
				while (true)
				{
					auto prev = enode->prev;
					destroy_node(enode->as_node());
					if (prev == nullptr)
						break;
					enode = prev;
				}
				throw;
			}
			link_nodes(pos.node_, r.node_, end.node_);
		}
		return r;
	}

	// 对 list进行归并排序，返回一个迭代器指向区间最小元素的位置
	//该函数通过递归分割链表为小块，逐步合并有序子序列，最终完成整体排序。
	template <class T>
	template <class Compared>
	typename list<T>::iterator list<T>::list_sort(iterator f1, iterator l2, size_type n, Compared comp)
		//f1起始迭代器，l2结束迭代器
	{
		if (n < 2)
			return f1;//单元素无需排序

		if (n == 2)//两元素简单排序
		{
			if (comp(*--l2, *f1))//后一元素更小则交换
			{
				auto ln = l2.node_;
				unlink_nodes(ln, ln);
				link_nodes(f1.node_, ln, ln);
				return l2;
			}
			return f1;
		}

		auto n2 = n / 2;//二分
		auto l1 = f1;
		mystl::advance(l1, n2);//由于是循环的，向前寻找n2个找到中点
		//递归
		auto result = f1 = list_sort(f1, l1, n2, comp);  // 前半段的最小位置
		auto f2 = l1 = list_sort(l1, l2, n - n2, comp);  // 后半段的最小位置

		// 把较小的一段区间移到前面
		if (comp(*f2, *f1))
		{
			auto m = f2;
			++m;
			for (; m != l2 && comp(*m, *f1); ++m)
				;
			auto f = f2.node_;
			auto l = m.node_->prev;
			result = f2;
			l1 = f2 = m;
			unlink_nodes(f, l);
			m = f1;
			++m;
			link_nodes(f1.node_, f, l);
			f1 = m;
		}
		else
		{
			++f1;
		}

		// 合并两段有序区间
		while (f1 != l1 && f2 != l2)
		{
			if (comp(*f2, *f1))
			{
				auto m = f2;
				++m;
				for (; m != l2 && comp(*m, *f1); ++m)
					;
				auto f = f2.node_;
				auto l = m.node_->prev;
				if (l1 == f2)
					l1 = m;
				f2 = m;
				unlink_nodes(f, l);
				m = f1;
				++m;
				link_nodes(f1.node_, f, l);
				f1 = m;
			}
			else
			{
				++f1;
			}
		}
		return result;
	}

	// 重载比较操作符
	template <class T>
	bool operator==(const list<T>& lhs, const list<T>& rhs)
	{
		auto f1 = lhs.cbegin();
		auto f2 = rhs.cbegin();
		auto l1 = lhs.cend();
		auto l2 = rhs.cend();
		for (; f1 != l1 && f2 != l2 && *f1 == *f2; ++f1, ++f2)
			;
		return f1 == l1 && f2 == l2;
	}

	template <class T>
	bool operator<(const list<T>& lhs, const list<T>& rhs)
	{
		return mystl::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
	}

	template <class T>
	bool operator!=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T>
	bool operator>(const list<T>& lhs, const list<T>& rhs)
	{
		return rhs < lhs;
	}

	template <class T>
	bool operator<=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T>
	bool operator>=(const list<T>& lhs, const list<T>& rhs)
	{
		return !(lhs < rhs);
	}

	// 重载 mystl 的 swap
	template <class T>
	void swap(list<T>& lhs, list<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}

}