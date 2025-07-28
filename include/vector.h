#pragma once
#include <initializer_list>//括号初始化

#include "algobase.h"
#include "iterator.h"
#include "memory.h"
#include "utils.h"
#include "algo.h"
#include "exceptdef.h"
#include "allocator.h"
#include "type_traits.h"
namespace mystl
{
	template<class T>
	class vector
	{
		static_assert(!mystl::is_same<bool, T>::value, "vector<bool> is abandoned in mystl");
		//vector<bool> 是一个饱受争议的特化版本,我们选择禁用他
		//vector<bool> 的特殊行为破坏了容器的一致性
		//位压缩存储：标准库的 vector<bool> 会将 bool 压缩存储为单个位（bit），而非完整的 bool 对象（通常占 1 字节）
		//这虽然节省了空间，但导致其行为与其他 vector<T> 不一致,无法直接返回bool&
	public:
		using allocator_type	= mystl::allocator<T>;
		using data_allocator	= mystl::allocator<T>;//指向同一个

		using value_type		= typename allocator_type::value_type;
		using pointer			= typename allocator_type::pointer;
		using const_pointer		= typename allocator_type::const_pointer;
		using reference			= typename allocator_type::reference;
		using const_reference	= typename allocator_type::const_reference;
		using size_type			= typename allocator_type::size_type;
		using difference_type	= typename allocator_type::difference_type;

		using iterator			= typename allocator_type::pointer;
		using const_iterator	= typename allocator_type::const_pointer;
		using reverse_iterator = mystl::reverse_iterator<iterator>;
		using const_reverse_iterator = mystl::reverse_iterator<const_iterator>;

		allocator_type get_allocator() const { return data_allocator(); }

	private:
		iterator begin_;  // 表示目前使用空间的头部
		iterator end_;    // 表示目前使用空间的尾部
		iterator cap_;    // 表示目前储存空间的尾部
	public:
		//迭代器相关操作
		iterator begin()noexcept
		{
			return begin_;
		}
		const_iterator begin()const noexcept
		{
			return begin_;
		}
		iterator end()noexcept
		{
			return end_;
		}
		const_iterator end()const noexcept
		{
			return end_;
		}

		//反向迭代器
		reverse_iterator rbegin()noexcept
		{
			return reverse_iterator(end());
		}
		//使用的是explicit reverse_iterator(iterator_type i) : current(i) {}
		//实际上是接受end()作为current（反向迭代器本质是一个包装过的正向迭代器）
		//反向迭代器的 operator*() 会先将内部的正向迭代器 current 向前移动一位，再解引用
		//传入end()时，current 初始为 end()，--tmp 使其指向最后一个元素，返回该元素的值
		//传入 begin() 时，current 初始为 begin()，--tmp 使其指向 begin() - 1（越界），但 rend() 作为结束标志，不会被实际解引用。
		const_reverse_iterator rbegin()const noexcept
		{
			return const_reverse_iterator(end());
		}
		reverse_iterator rend()noexcept
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator rend()const noexcept
		{
			return const_reverse_iterator(begin());
		}

		//const_iterator版本
		const_iterator cbegin()const noexcept
		{
			return begin();
		}
		const_iterator cend()const noexcept
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
		bool empty()const noexcept
		{
			return begin_ == end_;
		}
		size_type size()const noexcept
		{
			return static_cast<size_type>(end_ - begin_);
		}
		size_type max_size()const noexcept
		{
			return static_cast<size_type>(-1) / sizeof(T);
		}
		//
		size_type capacity()const noexcept
		{
			return static_cast<size_type>(cap_ - begin_);
		}
		void reserve(size_type n);
		void shrink_to_fit();


		//访问元素相关操作
		reference operator[](size_type n)
		{
			MYSTL_DEBUG(n < size());
			return *(begin_ + n);
		}
		const_reference operator[](size_type n)const
		{
			MYSTL_DEBUG(n < size());
			return *(begin_ + n);
		}
		reference at(size_type n)
		{
			THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
			return (*this)[n];
		}
		const_reference at(size_type n)const
		{
			THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
			return (*this)[n];
		}

		reference front()
		{
			MYSTL_DEBUG(!empty());
			return *begin_;
		}

		reference back()
		{
			MYSTL_DEBUG(!empty());
			return *(end_ - 1);
		}

		const_reference back() const
		{
			MYSTL_DEBUG(!empty());
			return *(end_ - 1);
		}

		pointer data()noexcept
		{
			return begin_;
		}
		const_pointer data()const noexcept
		{
			return begin_;
		}

		//修改容器相关操作
		// assign清空+重新填充
		void assign(size_type n, const value_type& value)
		{
			fill_assign(n, value);
		}

		template<class Iter, typename mystl::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
		void assign(Iter first, Iter last)
		{
			MYSTL_DEBUG(!(last < first));
			copy_assign(first, last, iterator_category(first));
		}

		void assign(std::initializer_list<value_type> il)
		{
			copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{});
		}

		//emplace/emplace_back
		template<class... Args>
		iterator emplace(const_iterator pos, Args&&...args);

		template <class... Args>
		void emplace_back(Args&& ...args);

		//push_back/pop_back
		void push_back(const value_type& value);
		void push_back(value_type&& value)
		{
			emplace_back(mystl::move(value));
		}
		void pop_back();

		//insert插入
		iterator insert(const_iterator pos, const value_type& value);
		iterator insert(const_iterator pos, value_type&& value)
		{
			return emplace(pos, mystl::move(value));
		}
		iterator insert(const_iterator pos, size_type n, const value_type& value)
		{
			MYSTL_DEBUG(pos >= begin() && pos <= end());
			return fill_insert(const_cast<iterator>(pos), n, value);
		}
		template<class Iter, typename mystl::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
		void insert(const_iterator pos, Iter first, Iter last)
		{
			MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
			copy_insert(const_cast<iterator>(pos), first, last);
		}

		//erase/clear
		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		void clear()
		{
			erase(begin(), end());
		}

		//resize/reverse
		void resize(size_type new_size)
		{
			return resize(new_size, value_type());
		}
		void resize(size_type new_size, const value_type& value);
		void reverse()
		{
			mystl::reverse(begin(), end());
		}

		//swap
		void swap(vector& rhs)noexcept;
	private:
		//工具函数
		void try_init() noexcept;//若分配失败则忽略，不抛出异常
		void fill_init(size_type n, const value_type& value);//分配空间，填充n个初值
		void init_space(size_type size, size_type cap);//会抛出异常

		template<class Iter>
		void range_init(Iter first, Iter last);//通过复制迭代器范围内的元素进行初始化

		void destroy_and_recover(iterator first, iterator last, size_type n);

		//计算增长规模
		size_type get_new_cap(size_type add_size);

		//assign相关
		void fill_assign(size_type n, const value_type& value);

		template<class IIter>
		void copy_assign(IIter first, IIter last, mystl::input_iterator_tag);

		//reallocate相关
		template<class...Args>
		void reallocate_emplace(iterator pos, Args&& ...args);
		void reallocate_insert(iterator pos, const value_type& value);

		//insert相关
		iterator fill_insert(iterator pos, size_type n, const value_type& value);

		template<class IIter>
		void copy_insert(iterator pos, IIter first, IIter last);

		//shrink_to_fit容器减少其内存占用
		void reinsert(size_type size);
	public:
		//构造
		//默认构造函数
		vector() noexcept
		{
			try_init();
		}
		//填充构造函数
		explicit vector(size_type n)
		{
			const size_type init_size = mystl::max(static_cast<size_type>(16), n);
			init_space(n, init_size);
			mystl::uninitialized_value_construct_n(begin_, n);
		}//构造函数使用默认值
		vector(size_type n, const value_type& value)
		{
			fill_init(n, value);
		}
		//范围构造函数
		template<class Iter, typename mystl::enable_if<
			mystl::is_input_iterator<Iter>::value, int>::type = 0>
		vector(Iter first, Iter last)
		{
			MYSTL_DEBUG(!(last < first));
			range_init(first, last);
		}
		//复制构造函数（深拷贝）
		vector(const vector& rhs)
		{
			range_init(rhs.begin_, rhs.end_);
		}
		//移动构造函数
		vector(vector&& rhs)noexcept
			:begin_(rhs.begin_),
			end_(rhs.end_),
			cap_(rhs.cap_)
		{
			//避免释放掉原对象
			rhs.begin_ = nullptr;
			rhs.end_ = nullptr;
			rhs.cap_ = nullptr;
		}

		//花括号初始化
		vector(std::initializer_list<value_type> ilist)
		{
			range_init(ilist.begin(), ilist.end());
		}
		//std::initializer_list 是 C++11 引入的特性
		//主要用于支持花括号初始化语法
		//当类存在多个构造函数时，花括号可能优先匹配 initializer_list 版本
		//如std::vector<int> v{5, 0};包含两个元素 5 和 0
		//std::vector<int> v(5, 0); // 包含五个元素 0

		vector& operator=(const vector& rhs);
		vector& operator= (vector&& rhs) noexcept;

		vector& operator=(std::initializer_list<value_type> ilist)
		{
			vector tmp(ilist.begin(), ilist.end());
			swap(tmp);//vector自身的swap
			return *this;
		}

		~vector()
		{
			destroy_and_recover(begin_, end_, cap_ - begin_);
			begin_ = end_ = cap_ = nullptr;
		}
	};
	//复制赋值
	template<class T>
	vector<T>& vector<T>::operator=(const vector& rhs)
	{
		if (this != &rhs)
		{
			const auto len = rhs.size();
			if (len > capacity())//rhs的大小超过当前容量
			{
				vector tmp(rhs.begin(), rhs.end());
				swap(tmp);//用的是vector特化的swap
			}
			else if (size() >= len)//当前元素数量足够多
			{
				auto i = mystl::copy(rhs.begin(), rhs.end(), begin());//覆盖
				data_allocator::destroy(i, end_);//销毁多余的元素
				end_ = begin_ + len;
			}
			else//容量足够但元素不足
			{
				mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);//覆盖有元素的
				mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);//在未初始化内存中构造
				end_ = begin_ + len;//cap_不应该变
			}
		}
		return *this;
	}

	//移动赋值
	template<class T>
	vector<T>& vector<T>::operator=(vector&& rhs)noexcept
	{
		if (this != &rhs) { // 添加自赋值检查
			destroy_and_recover(begin_, end_, cap_ - begin_);
			begin_ = rhs.begin_;
			end_ = rhs.end_;
			cap_ = rhs.cap_;
			rhs.begin_ = rhs.end_ = rhs.cap_ = nullptr;
		}
		return *this;
	}

	//预留空间大小，当原容量小于要求大小时，才会重新分配
	template<class T>
	void vector<T>::reserve(size_type n)
	{
		if (capacity() < n)
		{
			THROW_LENGTH_ERROR_IF(n > max_size(), "n can not larger than max_size() in vector<T>::reserve(n)");
			const auto old_size = size();
			auto tmp = data_allocator::allocate(n);
			mystl::uninitialized_move(begin_, end_, tmp);
			data_allocator::deallocate(begin_, cap_ - begin_);
			begin_ = tmp;
			end_ = tmp + old_size;
			cap_ = begin_ + n;
		}
	}

	//放弃多余的容量
	template<class T>
	void vector<T>::shrink_to_fit()
	{
		if (end_ < cap_)
		{
			reinsert(size());
		}
	}

	//在pos位置就地构造元素，避免额外的复制或移动开销
	template<class T>
	template<class ...Args>
	typename vector<T>::iterator
		vector<T>::emplace(const_iterator pos, Args&& ...args)//虽然使用了可变参数模板但还是只插入一个函数
	{
		MYSTL_DEBUG(pos >= begin() && pos <= end());//插入的合法性
		iterator xpos = const_cast<iterator>(pos);//将const_iterator转为普通iterator
		const size_type n = xpos - begin_;//计算插入位置
		if (end_ != cap_ && xpos == end_)//尾部插入
		{
			data_allocator::construct(mystl::addressof(*end_), mystl::forward<Args>(args)...);//再end_位置就地构造一个元素，无需移动其他元素
			++end_;
		}
		else if (end_ != cap_)//中间插入
		{
			auto new_end = end_;
			data_allocator::construct(mystl::addressof(*end_), *(end_ - 1));//构造最后一个元素的副本
			++new_end;
			mystl::copy_backward(xpos, end_ - 1, end_);//将[xpos,end_-2]的元素向后移动一位腾出位置
			data_allocator::destroy(xpos); // 析构被覆盖的旧对象
			data_allocator::construct(xpos, mystl::forward<Args>(args)...); // 就地构造新对象
			end_ = new_end;
		}
		else
		{
			reallocate_emplace(xpos, mystl::forward<Args>(args)...);//空间不足
		}
		return begin() + n;//返回插入位置
	}


	//在尾部就地构造函数,注意只能插入一个元素
	template <class T>
	template <class ...Args>
	void vector<T>::emplace_back(Args&& ...args)
	{
		if (end_ < cap_)
		{
			data_allocator::construct(mystl::addressof(*end_), mystl::forward<Args>(args)...);
			++end_;
		}
		else
		{
			reallocate_emplace(end_, mystl::forward<Args>(args)...);
		}
	}

	// 在尾部插入一个已经构造好的元素
	template <class T>
	void vector<T>::push_back(const value_type& value)
	{
		if (end_ != cap_)
		{
			data_allocator::construct(mystl::addressof(*end_), value);
			++end_;
		}
		else
		{
			reallocate_insert(end_, value);
		}
	}

	// 弹出尾部元素
	template <class T>
	void vector<T>::pop_back()
	{
		MYSTL_DEBUG(!empty());
		data_allocator::destroy(end_ - 1);
		--end_;
	}

	//在pos处插入元素
	template<class T>
	typename vector<T>::iterator vector<T>::insert(const_iterator pos, const value_type& value)
	{
		MYSTL_DEBUG(pos >= begin() && pos <= end());
		iterator xpos = const_cast<iterator>(pos);
		const size_type n = pos - begin_;
		if (end_ != cap_ && xpos == end_)//在末尾插入
		{
			data_allocator::construct(mystl::addressof(*end_), value);
			++end_;
		}
		else if (end_ != cap_)
		{
			auto new_end = end_;
			data_allocator::construct(mystl::addressof(*end_), *(end_ - 1));//使用一位新的
			++new_end;
			auto value_copy = value;  // 避免元素因以下复制操作而被改变
			mystl::copy_backward(xpos, end_ - 1, end_);//统一向后挪
			*xpos = mystl::move(value_copy);
			end_ = new_end;
		}
		else
		{
			reallocate_insert(xpos, value);
		}
		return begin_ + n;
	}

	//删除 pos 位置上的元素，并指向下一个迭代器
	template <class T>
	typename vector<T>::iterator vector<T>::erase(const_iterator pos)
	{
		MYSTL_DEBUG(pos >= begin() && pos < end());
		//不能直接xpos=pos，因为pos是const的，需要转换为可修改的迭代器
		iterator xpos = begin_ + (pos - begin());
		mystl::move(xpos + 1, end_, xpos);//向前挪动一位，需要删除的数据在这已经销毁了
		data_allocator::destroy(end_ - 1);//销毁原最后一个数据
		--end_;
		return xpos;
	}

	// 删除[first, last)上的元素
	template <class T>
	typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last)
	{
		MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
		const auto n = first - begin();
		iterator r = begin_ + (first - begin());
		data_allocator::destroy(mystl::move(r + (last - first), end_, r), end_);
		end_ = end_ - (last - first);
		return begin_ + n;
	}

	// 重置容器大小
	template <class T>
	void vector<T>::resize(size_type new_size, const value_type& value)
	{
		if (new_size < size())
		{
			erase(begin() + new_size, end());
		}
		else
		{
			reserve(new_size); // 预先分配好内存
			while (size() < new_size) {
				emplace_back(value); // 调用默认构造函数
			}
		}
	}

	/*****修改容器相关操作******/
	//交换vector
	template<class T>
	void vector<T>::swap(vector<T>& rhs)noexcept
	{
		if (this != &rhs)
		{
			mystl::swap(begin_, rhs.begin_);
			mystl::swap(end_, rhs.end_);
			mystl::swap(cap_, rhs.cap_);
		}
	}
	/*****分配相关*****/
	template<class T>
	void vector<T>::try_init()noexcept
	{
		try
		{
			begin_ = data_allocator::allocate(16);
			end_ = begin_;
			cap_ = begin_ + 16;
		}
		catch (...)
		{
			begin_ = nullptr;
			end_ = nullptr;
			cap_ = nullptr;
		}
	}

	//helper function

	template<class T>
	void vector<T>::init_space(size_type size, size_type cap)
	{
		try
		{
			begin_ = data_allocator::allocate(cap);
			if (!begin_)
			{
				throw std::bad_alloc();
			}
			end_ = begin_ + size;//用了size的空间
			cap_ = begin_ + cap;//实际上拥有cap的空间
		}
		catch (...)
		{
			begin_ = nullptr;
			end_ = nullptr;
			cap_ = nullptr;
			throw;
		}
	}

	template<class T>
	void vector<T>::fill_init(size_type n, const value_type& value)
	{
		const size_type init_size = mystl::max(static_cast<size_type>(16), n);//至少分配16的空间，不一定用
		init_space(n, init_size);
		mystl::uninitialized_fill_n(begin_, n, value);
	}

	template<class T>
	template<class Iter>
	void vector<T>::range_init(Iter first, Iter last)
	{
		const size_type len = mystl::distance(first, last);
		const size_type init_size = mystl::max(len, static_cast<size_type>(16));
		init_space(len, init_size);
		mystl::uninitialized_copy(first, last, begin_);
	}

	template<class T>
	void vector<T>::destroy_and_recover(iterator first, iterator last, size_type n)
	{
		data_allocator::destroy(first, last);
		data_allocator::deallocate(first, n);
	}

	//用于计算新容量，在插入新元素时确定扩容后的容量大小确保不超过最大限制
	template<class T>
	//typename 明确告诉编译器 vector<T>::size_type是一个类型
	typename vector<T>::size_type vector<T>::get_new_cap(size_type add_size)
	{
		const auto old_size = capacity();
		//加入后超出最大限制
		THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size, "vector<T>'s size too big");
		//当前容量接近最大限制
		if (old_size > max_size() - old_size / 2)
		{
			return old_size + add_size > max_size() - 16 ? old_size + add_size : old_size + add_size + 16;
			//+16是预留少量额外空间以减少后续扩容次数
		}
		//若当前容量为0，就要初始化add_size和16中的较大值，否则按50%扩容
		const size_type new_size = (old_size == 0 ? mystl::max(add_size, static_cast<size_type>(16)) : mystl::max(old_size + old_size / 2, old_size + add_size));
		//最外层的括号无意义方便理解
		return new_size;
	}

	//将vector的内容赋值为n个value
	template <class T>
	void vector<T>::fill_assign(size_type n, const value_type& value)
	{
		//需要扩容
		if (n > capacity())
		{
			vector tmp(n, value);
			swap(tmp);//原数据丢弃
		}
		else if (n > size())
		{
			mystl::fill(begin(), end(), value);//覆盖已有元素
			end_ = mystl::uninitialized_fill_n(end_, n - size(), value);//新增元素并更新new
		}
		else
		{
			erase(mystl::fill_n(begin_, n, value), end_);//覆盖前n个元素并删除多余元素
		}
	}
	// 用 [first, last) 为容器赋值
	template<class T>
	template<class IIter>
	void vector<T>::copy_assign(IIter first, IIter last, mystl::input_iterator_tag)
	{
		const size_type len = mystl::distance(first, last);
		if (len > capacity())
		{
			vector tmp(first, last);
			swap(tmp);
		}
		else if (size() >= len)
		{
			auto new_end = mystl::copy(first, last, begin_);
			data_allocator::destroy(new_end, end_);
			end_ = new_end;
		}
		else
		{
			auto mid = first;
			mystl::advance(mid, size());
			mystl::copy(first, mid, begin_);
			auto new_end = mystl::uninitialized_copy(mid, last, end_);
			end_ = new_end;
		}
	}

	//重新分配空间并在pos处就地构造函数
	template<class T>
	template<class ...Args>
	void vector<T>::reallocate_emplace(iterator pos, Args&& ...args)
	{
		const auto new_size = get_new_cap(1);
		auto new_begin = data_allocator::allocate(new_size);
		auto new_end = new_begin;
		try
		{
			new_end = mystl::uninitialized_move(begin_, pos, new_begin);
			data_allocator::construct(mystl::addressof(*new_end), mystl::forward<Args>(args)...);
			++new_end;
			new_end = mystl::uninitialized_move(pos, end_, new_end);
		}
		catch (...)
		{
			data_allocator::deallocate(new_begin, new_size);
			throw;
		}
		destroy_and_recover(begin_, end_, cap_ - begin_);
		begin_ = new_begin;
		end_ = new_end;
		cap_ = new_begin + new_size;
	}

	// 重新分配空间并在 pos 处插入元素
	template <class T>
	void vector<T>::reallocate_insert(iterator pos, const value_type& value)
	{
		const auto new_size = get_new_cap(1);
		auto new_begin = data_allocator::allocate(new_size);
		auto new_end = new_begin;
		const value_type& value_copy = value;
		try
		{
			new_end = mystl::uninitialized_move(begin_, pos, new_begin);
			data_allocator::construct(mystl::addressof(*new_end), value_copy);
			++new_end;
			new_end = mystl::uninitialized_move(pos, end_, new_end);
		}
		catch (...)
		{
			data_allocator::deallocate(new_begin, new_size);
			throw;
		}
		destroy_and_recover(begin_, end_, cap_ - begin_);
		begin_ = new_begin;
		end_ = new_end;
		cap_ = new_begin + new_size;
	}

	template <class T>
	typename vector<T>::iterator vector<T>::fill_insert(iterator pos, size_type n, const value_type& value)
	{
		if (n == 0)
			return pos;
		const size_type xpos = pos - begin_;
		const value_type value_copy = value;
		if (static_cast<size_type>(cap_ - end_) >= n)
		{
			const size_type after_elems = end_ - pos;
			auto old_end = end_;
			if (after_elems > n)
			{
				mystl::uninitialized_copy(end_ - n, end_, end_);
				end_ += n;
				mystl::move_backward(pos, old_end - n, old_end);
				mystl::uninitialized_fill_n(pos, n, value_copy);
			}
			else
			{
				end_ = mystl::uninitialized_fill_n(end_, n - after_elems, value_copy);
				end_ = mystl::uninitialized_move(pos, old_end, end_);
				mystl::uninitialized_fill_n(pos, after_elems, value_copy);
			}
		}
		else
		{
			//如果备用空间不足
			const auto new_size = get_new_cap(n);
			auto new_begin = data_allocator::allocate(new_size);
			auto new_end = new_begin;
			try
			{
				new_end = mystl::uninitialized_move(begin_, pos, new_begin);
				new_end = mystl::uninitialized_fill_n(new_end, n, value);
				new_end = mystl::uninitialized_move(pos, end_, new_end);
			}
			catch (...)
			{
				destroy_and_recover(new_begin, new_end, new_size);
				throw;
			}
			data_allocator::deallocate(begin_, cap_ - begin_);
			begin_ = new_begin;
			end_ = new_end;
			cap_ = begin_ + new_size;
		}
		return begin_ + xpos;
	}

	template <class T>
	template <class IIter>
	void vector<T>::copy_insert(iterator pos, IIter first, IIter last)
	{
		if (first == last)
			return;
		const auto n = mystl::distance(first, last);
		if ((cap_ - end_) >= n)
		{
			//备用空间大小足够
			const auto after_elems = end_ - pos;
			auto old_end = end_;
			if (after_elems > n)
			{
				end_ = mystl::uninitialized_copy(end_ - n, end_, end_);
				mystl::move_backward(pos, old_end - n, old_end);
				mystl::uninitialized_copy(first, last, pos);
			}
			else
			{
				auto mid = first;
				mystl::advance(mid, after_elems);
				end_ = mystl::uninitialized_copy(mid, last, end_);
				end_ = mystl::uninitialized_move(pos, old_end, end_);
				mystl::uninitialized_copy(first, mid, pos);
			}
		}
		else
		{
			//备用空间不足
			const auto new_size = get_new_cap(n);
			auto new_begin = data_allocator::allocate(new_size);
			auto new_end = new_begin;
			try
			{
				new_end = mystl::uninitialized_move(begin_, pos, new_begin);
				new_end = mystl::uninitialized_copy(first, last, new_end);
				new_end = mystl::uninitialized_move(pos, end_, new_end);
			}
			catch (...)
			{
				destroy_and_recover(new_begin, new_end, new_size);
				throw;
			}
			data_allocator::deallocate(begin_, cap_ - begin_);
			begin_ = new_begin;
			end_ = new_end;
			cap_ = begin_ + new_size;
		}
	}

	//重新分配内存并移动元素
	template<class T>
	void vector<T>::reinsert(size_type size)
	{
		auto new_begin = data_allocator::allocate(size);
		try
		{
			mystl::uninitialized_move(begin_, end_, new_begin);
		}
		catch (...)
		{
			data_allocator::deallocate(new_begin, size);
			throw;
		}
		data_allocator::destroy(begin_, end_);//析构旧对象再释放内存
		data_allocator::deallocate(begin_, cap_ - begin_);
		begin_ = new_begin;
		end_ = begin_ + size;
		cap_ = begin_ + size;
	}

	//重载比较操作符
	template <class T>
	bool operator==(const vector<T>& lhs, const vector<T>& rhs)
	{
		return lhs.size() == rhs.size() &&
			mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template <class T>
	bool operator<(const vector<T>& lhs, const vector<T>& rhs)
	{
		return mystl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	template <class T>
	bool operator!=(const vector<T>& lhs, const vector<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T>
	bool operator>(const vector<T>& lhs, const vector<T>& rhs)
	{
		return rhs < lhs;
	}

	template <class T>
	bool operator<=(const vector<T>& lhs, const vector<T>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T>
	bool operator>=(const vector<T>& lhs, const vector<T>& rhs)
	{
		return !(lhs < rhs);
	}

	// 重载 mystl 的 swap
	template <class T>
	void swap(vector<T>& lhs, vector<T>& rhs)
	{
		lhs.swap(rhs);
	}
}