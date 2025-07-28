#pragma once
#include <initializer_list>//���ų�ʼ��

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
		//vector<bool> ��һ������������ػ��汾,����ѡ�������
		//vector<bool> ��������Ϊ�ƻ���������һ����
		//λѹ���洢����׼��� vector<bool> �Ὣ bool ѹ���洢Ϊ����λ��bit�������������� bool ����ͨ��ռ 1 �ֽڣ�
		//����Ȼ��ʡ�˿ռ䣬����������Ϊ������ vector<T> ��һ��,�޷�ֱ�ӷ���bool&
	public:
		using allocator_type	= mystl::allocator<T>;
		using data_allocator	= mystl::allocator<T>;//ָ��ͬһ��

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
		iterator begin_;  // ��ʾĿǰʹ�ÿռ��ͷ��
		iterator end_;    // ��ʾĿǰʹ�ÿռ��β��
		iterator cap_;    // ��ʾĿǰ����ռ��β��
	public:
		//��������ز���
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

		//���������
		reverse_iterator rbegin()noexcept
		{
			return reverse_iterator(end());
		}
		//ʹ�õ���explicit reverse_iterator(iterator_type i) : current(i) {}
		//ʵ�����ǽ���end()��Ϊcurrent�����������������һ����װ���������������
		//����������� operator*() ���Ƚ��ڲ������������ current ��ǰ�ƶ�һλ���ٽ�����
		//����end()ʱ��current ��ʼΪ end()��--tmp ʹ��ָ�����һ��Ԫ�أ����ظ�Ԫ�ص�ֵ
		//���� begin() ʱ��current ��ʼΪ begin()��--tmp ʹ��ָ�� begin() - 1��Խ�磩���� rend() ��Ϊ������־�����ᱻʵ�ʽ����á�
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

		//const_iterator�汾
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


		//������ز���
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


		//����Ԫ����ز���
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

		//�޸�������ز���
		// assign���+�������
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

		//insert����
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
		//���ߺ���
		void try_init() noexcept;//������ʧ������ԣ����׳��쳣
		void fill_init(size_type n, const value_type& value);//����ռ䣬���n����ֵ
		void init_space(size_type size, size_type cap);//���׳��쳣

		template<class Iter>
		void range_init(Iter first, Iter last);//ͨ�����Ƶ�������Χ�ڵ�Ԫ�ؽ��г�ʼ��

		void destroy_and_recover(iterator first, iterator last, size_type n);

		//����������ģ
		size_type get_new_cap(size_type add_size);

		//assign���
		void fill_assign(size_type n, const value_type& value);

		template<class IIter>
		void copy_assign(IIter first, IIter last, mystl::input_iterator_tag);

		//reallocate���
		template<class...Args>
		void reallocate_emplace(iterator pos, Args&& ...args);
		void reallocate_insert(iterator pos, const value_type& value);

		//insert���
		iterator fill_insert(iterator pos, size_type n, const value_type& value);

		template<class IIter>
		void copy_insert(iterator pos, IIter first, IIter last);

		//shrink_to_fit�����������ڴ�ռ��
		void reinsert(size_type size);
	public:
		//����
		//Ĭ�Ϲ��캯��
		vector() noexcept
		{
			try_init();
		}
		//��乹�캯��
		explicit vector(size_type n)
		{
			const size_type init_size = mystl::max(static_cast<size_type>(16), n);
			init_space(n, init_size);
			mystl::uninitialized_value_construct_n(begin_, n);
		}//���캯��ʹ��Ĭ��ֵ
		vector(size_type n, const value_type& value)
		{
			fill_init(n, value);
		}
		//��Χ���캯��
		template<class Iter, typename mystl::enable_if<
			mystl::is_input_iterator<Iter>::value, int>::type = 0>
		vector(Iter first, Iter last)
		{
			MYSTL_DEBUG(!(last < first));
			range_init(first, last);
		}
		//���ƹ��캯���������
		vector(const vector& rhs)
		{
			range_init(rhs.begin_, rhs.end_);
		}
		//�ƶ����캯��
		vector(vector&& rhs)noexcept
			:begin_(rhs.begin_),
			end_(rhs.end_),
			cap_(rhs.cap_)
		{
			//�����ͷŵ�ԭ����
			rhs.begin_ = nullptr;
			rhs.end_ = nullptr;
			rhs.cap_ = nullptr;
		}

		//�����ų�ʼ��
		vector(std::initializer_list<value_type> ilist)
		{
			range_init(ilist.begin(), ilist.end());
		}
		//std::initializer_list �� C++11 ���������
		//��Ҫ����֧�ֻ����ų�ʼ���﷨
		//������ڶ�����캯��ʱ�������ſ�������ƥ�� initializer_list �汾
		//��std::vector<int> v{5, 0};��������Ԫ�� 5 �� 0
		//std::vector<int> v(5, 0); // �������Ԫ�� 0

		vector& operator=(const vector& rhs);
		vector& operator= (vector&& rhs) noexcept;

		vector& operator=(std::initializer_list<value_type> ilist)
		{
			vector tmp(ilist.begin(), ilist.end());
			swap(tmp);//vector�����swap
			return *this;
		}

		~vector()
		{
			destroy_and_recover(begin_, end_, cap_ - begin_);
			begin_ = end_ = cap_ = nullptr;
		}
	};
	//���Ƹ�ֵ
	template<class T>
	vector<T>& vector<T>::operator=(const vector& rhs)
	{
		if (this != &rhs)
		{
			const auto len = rhs.size();
			if (len > capacity())//rhs�Ĵ�С������ǰ����
			{
				vector tmp(rhs.begin(), rhs.end());
				swap(tmp);//�õ���vector�ػ���swap
			}
			else if (size() >= len)//��ǰԪ�������㹻��
			{
				auto i = mystl::copy(rhs.begin(), rhs.end(), begin());//����
				data_allocator::destroy(i, end_);//���ٶ����Ԫ��
				end_ = begin_ + len;
			}
			else//�����㹻��Ԫ�ز���
			{
				mystl::copy(rhs.begin(), rhs.begin() + size(), begin_);//������Ԫ�ص�
				mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);//��δ��ʼ���ڴ��й���
				end_ = begin_ + len;//cap_��Ӧ�ñ�
			}
		}
		return *this;
	}

	//�ƶ���ֵ
	template<class T>
	vector<T>& vector<T>::operator=(vector&& rhs)noexcept
	{
		if (this != &rhs) { // ����Ը�ֵ���
			destroy_and_recover(begin_, end_, cap_ - begin_);
			begin_ = rhs.begin_;
			end_ = rhs.end_;
			cap_ = rhs.cap_;
			rhs.begin_ = rhs.end_ = rhs.cap_ = nullptr;
		}
		return *this;
	}

	//Ԥ���ռ��С����ԭ����С��Ҫ���Сʱ���Ż����·���
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

	//�������������
	template<class T>
	void vector<T>::shrink_to_fit()
	{
		if (end_ < cap_)
		{
			reinsert(size());
		}
	}

	//��posλ�þ͵ع���Ԫ�أ��������ĸ��ƻ��ƶ�����
	template<class T>
	template<class ...Args>
	typename vector<T>::iterator
		vector<T>::emplace(const_iterator pos, Args&& ...args)//��Ȼʹ���˿ɱ����ģ�嵫����ֻ����һ������
	{
		MYSTL_DEBUG(pos >= begin() && pos <= end());//����ĺϷ���
		iterator xpos = const_cast<iterator>(pos);//��const_iteratorתΪ��ͨiterator
		const size_type n = xpos - begin_;//�������λ��
		if (end_ != cap_ && xpos == end_)//β������
		{
			data_allocator::construct(mystl::addressof(*end_), mystl::forward<Args>(args)...);//��end_λ�þ͵ع���һ��Ԫ�أ������ƶ�����Ԫ��
			++end_;
		}
		else if (end_ != cap_)//�м����
		{
			auto new_end = end_;
			data_allocator::construct(mystl::addressof(*end_), *(end_ - 1));//�������һ��Ԫ�صĸ���
			++new_end;
			mystl::copy_backward(xpos, end_ - 1, end_);//��[xpos,end_-2]��Ԫ������ƶ�һλ�ڳ�λ��
			data_allocator::destroy(xpos); // ���������ǵľɶ���
			data_allocator::construct(xpos, mystl::forward<Args>(args)...); // �͵ع����¶���
			end_ = new_end;
		}
		else
		{
			reallocate_emplace(xpos, mystl::forward<Args>(args)...);//�ռ䲻��
		}
		return begin() + n;//���ز���λ��
	}


	//��β���͵ع��캯��,ע��ֻ�ܲ���һ��Ԫ��
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

	// ��β������һ���Ѿ�����õ�Ԫ��
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

	// ����β��Ԫ��
	template <class T>
	void vector<T>::pop_back()
	{
		MYSTL_DEBUG(!empty());
		data_allocator::destroy(end_ - 1);
		--end_;
	}

	//��pos������Ԫ��
	template<class T>
	typename vector<T>::iterator vector<T>::insert(const_iterator pos, const value_type& value)
	{
		MYSTL_DEBUG(pos >= begin() && pos <= end());
		iterator xpos = const_cast<iterator>(pos);
		const size_type n = pos - begin_;
		if (end_ != cap_ && xpos == end_)//��ĩβ����
		{
			data_allocator::construct(mystl::addressof(*end_), value);
			++end_;
		}
		else if (end_ != cap_)
		{
			auto new_end = end_;
			data_allocator::construct(mystl::addressof(*end_), *(end_ - 1));//ʹ��һλ�µ�
			++new_end;
			auto value_copy = value;  // ����Ԫ�������¸��Ʋ��������ı�
			mystl::copy_backward(xpos, end_ - 1, end_);//ͳһ���Ų
			*xpos = mystl::move(value_copy);
			end_ = new_end;
		}
		else
		{
			reallocate_insert(xpos, value);
		}
		return begin_ + n;
	}

	//ɾ�� pos λ���ϵ�Ԫ�أ���ָ����һ��������
	template <class T>
	typename vector<T>::iterator vector<T>::erase(const_iterator pos)
	{
		MYSTL_DEBUG(pos >= begin() && pos < end());
		//����ֱ��xpos=pos����Ϊpos��const�ģ���Ҫת��Ϊ���޸ĵĵ�����
		iterator xpos = begin_ + (pos - begin());
		mystl::move(xpos + 1, end_, xpos);//��ǰŲ��һλ����Ҫɾ�������������Ѿ�������
		data_allocator::destroy(end_ - 1);//����ԭ���һ������
		--end_;
		return xpos;
	}

	// ɾ��[first, last)�ϵ�Ԫ��
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

	// ����������С
	template <class T>
	void vector<T>::resize(size_type new_size, const value_type& value)
	{
		if (new_size < size())
		{
			erase(begin() + new_size, end());
		}
		else
		{
			reserve(new_size); // Ԥ�ȷ�����ڴ�
			while (size() < new_size) {
				emplace_back(value); // ����Ĭ�Ϲ��캯��
			}
		}
	}

	/*****�޸�������ز���******/
	//����vector
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
	/*****�������*****/
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
			end_ = begin_ + size;//����size�Ŀռ�
			cap_ = begin_ + cap;//ʵ����ӵ��cap�Ŀռ�
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
		const size_type init_size = mystl::max(static_cast<size_type>(16), n);//���ٷ���16�Ŀռ䣬��һ����
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

	//���ڼ������������ڲ�����Ԫ��ʱȷ�����ݺ��������Сȷ���������������
	template<class T>
	//typename ��ȷ���߱����� vector<T>::size_type��һ������
	typename vector<T>::size_type vector<T>::get_new_cap(size_type add_size)
	{
		const auto old_size = capacity();
		//����󳬳��������
		THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size, "vector<T>'s size too big");
		//��ǰ�����ӽ��������
		if (old_size > max_size() - old_size / 2)
		{
			return old_size + add_size > max_size() - 16 ? old_size + add_size : old_size + add_size + 16;
			//+16��Ԥ����������ռ��Լ��ٺ������ݴ���
		}
		//����ǰ����Ϊ0����Ҫ��ʼ��add_size��16�еĽϴ�ֵ������50%����
		const size_type new_size = (old_size == 0 ? mystl::max(add_size, static_cast<size_type>(16)) : mystl::max(old_size + old_size / 2, old_size + add_size));
		//���������������巽�����
		return new_size;
	}

	//��vector�����ݸ�ֵΪn��value
	template <class T>
	void vector<T>::fill_assign(size_type n, const value_type& value)
	{
		//��Ҫ����
		if (n > capacity())
		{
			vector tmp(n, value);
			swap(tmp);//ԭ���ݶ���
		}
		else if (n > size())
		{
			mystl::fill(begin(), end(), value);//��������Ԫ��
			end_ = mystl::uninitialized_fill_n(end_, n - size(), value);//����Ԫ�ز�����new
		}
		else
		{
			erase(mystl::fill_n(begin_, n, value), end_);//����ǰn��Ԫ�ز�ɾ������Ԫ��
		}
	}
	// �� [first, last) Ϊ������ֵ
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

	//���·���ռ䲢��pos���͵ع��캯��
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

	// ���·���ռ䲢�� pos ������Ԫ��
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
			//������ÿռ䲻��
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
			//���ÿռ��С�㹻
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
			//���ÿռ䲻��
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

	//���·����ڴ沢�ƶ�Ԫ��
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
		data_allocator::destroy(begin_, end_);//�����ɶ������ͷ��ڴ�
		data_allocator::deallocate(begin_, cap_ - begin_);
		begin_ = new_begin;
		end_ = begin_ + size;
		cap_ = begin_ + size;
	}

	//���رȽϲ�����
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

	// ���� mystl �� swap
	template <class T>
	void swap(vector<T>& lhs, vector<T>& rhs)
	{
		lhs.swap(rhs);
	}
}