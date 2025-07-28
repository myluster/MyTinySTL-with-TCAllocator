#pragma once
//STL�ڴ����ĺ��Ĳ��֣�������߼��Ķ�̬�ڴ����
//���������������ռ���������δ��ʼ���Ĵ���ռ�����Լ�����ָ�� unique_ptr
#include <cstddef>//�ṩ���ڴ沼����صĻ������ͺͳ���
#include <cstdlib>//�ṩ C ��׼����ڴ������malloc\free\realloc
#include <climits>//����������������صļ���ֵ

#include "utils.h"//ʹ��move swap
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"
#include "iterator.h"
namespace mystl
{
	// ��ȡ / �ͷ� ��ʱ������

	template <class T>
	pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*)
	{
		//len * sizeof(T) ������ INT_MAX���������
		//static_cast<ptrdiff_t>��������ǿ��ת��Ϊ�з���������ƥ�� len
		if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
			len = INT_MAX / sizeof(T);
		while (len > 0)
		{
			T* tmp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
			if (tmp)
				//���ط�����ڴ�ָ���ʵ�ʳɹ�����ĳ���
				return pair<T*, ptrdiff_t>(tmp, len);
			len /= 2;  // ����ʧ��ʱ���� len �Ĵ�С
		}
		return pair<T*, ptrdiff_t>(nullptr, 0);
	}

	template<class T>
	pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len)
	{
		return get_buffer_helper(len, static_cast<T*>(0));
	}

	template<class T>
	pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len, T*)
	{
		return get_buffer_helper(len, static_cast<T*>(0));
	}

	template<class T>
	void release_temporary_buffer(T* ptr)
	{
		free(ptr);
	}

	// ��ģ�� : temporary_buffer
	// ������ʱ���������������ͷ�
	template<class T>
	class temporary_buffer
	{
	private:
		ptrdiff_t	original_len;	//����������Ĵ�С
		ptrdiff_t	len;			//������ʵ�ʵĴ�С
		T* buffer;			//ָ�򻺳�����ָ��
	public:
		//���졢��������
		template<class ForwardIterator>
		temporary_buffer(ForwardIterator first, ForwardIterator last);

		~temporary_buffer()
		{
			if (buffer)
			{
				mystl::destroy(buffer, buffer + len);
				free(buffer);
			}
		}

		ptrdiff_t size()           const noexcept { return len; }
		ptrdiff_t requested_size() const noexcept { return original_len; }
		T* begin()                noexcept { return buffer; }
		T* end()                  noexcept { return buffer + len; }

		// ���ÿ�������Ϳ�����ֵ
		temporary_buffer(const temporary_buffer&) = delete;//��ֹǳ�������µ���Դ�ظ��ͷ�
		temporary_buffer& operator=(const temporary_buffer&) = delete;

	private:
		void allocate_buffer();
	};

	//���캯��
	template<class T>
	template<class ForwardIterator>
	temporary_buffer<T>::temporary_buffer(ForwardIterator first, ForwardIterator last)
	{
		len = mystl::distance(first, last);
		original_len = len;
		try
		{
			allocate_buffer();
			if (len > 0)
			{
				uninitialized_copy(first, last, buffer);
			}
		}
		catch (...)
		{
			if (buffer) {
				mystl::destroy(buffer, buffer + len);  // �����ѹ���Ķ���
				free(buffer);
			}
			buffer = nullptr;
			len = 0;
			throw;
		}
	}

	// allocate_buffer ����
	template <class T>
	void temporary_buffer<T>::allocate_buffer()
	{
		if (len <= 0)
		{
			buffer = nullptr;
			return;
		}
		size_t max_size = static_cast<ptrdiff_t>(INT_MAX / sizeof(T));
		if (len > max_size)
			len = max_size;
		while (len > 0)
		{
			buffer = static_cast<T*>(malloc(len * sizeof(T)));
			if (buffer)
			{
				break;
			}
			len /= 2;  // ����ʧ��ʱ��������ռ��С
		}
		if (len == 0)
		{
			throw std::bad_alloc();
		}
	}

	// uninitialized_value_construct_n
	// �� [first, first + n) �����ڽ���ֵ��ʼ��
	template <class ForwardIter>
	void unchecked_value_construct_n_aux(ForwardIter first, size_t n, std::false_type)
	{
		ForwardIter current = first;
		try {
			for (; n > 0; --n, ++current) {
				// ��ÿ��Ԫ�ص���Ĭ�Ϲ��캯�� T()
				mystl::construct(mystl::addressof(*current));
			}
		}
		catch (...) {
			// �������������׳��쳣�������������ѳɹ�����Ķ���
			mystl::destroy(first, current);
			throw; // �����׳��쳣
		}
	}

	// �ڲ��������� (ƽ�����Ͱ汾) - ����·��
	template <class ForwardIter>
	void unchecked_value_construct_n_aux(ForwardIter first, size_t n, std::true_type)
	{
		// ���� POD (Plain Old Data) ���ͣ�ֵ��ʼ����ͬ�����ʼ��
		// ʹ�� fill_n Ч�ʸ���
		mystl::fill_n(first, n, typename iterator_traits<ForwardIter>::value_type());
	}

	template <class ForwardIter>
	void uninitialized_value_construct_n(ForwardIter first, size_t n)
	{
		// ͨ�� is_trivially_default_constructible ���зַ�
		// ������� T ��һ������Ҫ��(trivial)Ĭ�Ϲ��캯�������ǾͿ��Խ����Ż�
		unchecked_value_construct_n_aux(first, n,
			std::is_trivially_default_constructible<
			typename iterator_traits<ForwardIter>::value_type
			>{});
	}

	//unique_ptr
	//�޸���auto_ptr�����ȱ��
	//�������:��ռ����Ȩ����Դ��ȫ��������⿪��

	template<class T>
	class unique_ptr
	{
	public:
		typedef T	element_type;
	private:
		T* m_ptr;	//ʵ��ָ��
	public:
		//Ĭ�Ϲ��캯��
		explicit unique_ptr(T* p = nullptr)noexcept :m_ptr(p) {}
		//ɾ���������캯��
		unique_ptr(const unique_ptr&) = delete;
		//�ƶ����캯��
		unique_ptr(unique_ptr&& rhs)noexcept :m_ptr(rhs.release()) {}
		//ģ���ƶ����캯����֧�������ൽ�����ת����
		template<class U>
		unique_ptr(unique_ptr<U>&& rhs)noexcept :m_ptr(rhs.release()) {}
		//��������
		~unique_ptr() { reset(); }

		//ɾ��������ֵ
		unique_ptr& operator=(const unique_ptr&) = delete;

		//�ƶ���ֵ������
		unique_ptr& operator=(unique_ptr&& rhs)noexcept
		{
			reset(rhs.release());
			return *this;
		}

		//ģ���ƶ���ֵ��֧�������ൽ�����ת����
		template<class U>
		unique_ptr& operator=(unique_ptr<U>&& rhs)noexcept
		{
			reset(rhs.release());
			return *this;
		}

		//����ָ�������
		T& operator*() const noexcept
		{
			return *m_ptr;
		}
		T* operator->()const noexcept
		{
			return m_ptr;
		}

		//��ȡԭʼָ��
		T* get()const noexcept
		{
			return m_ptr;
		}

		//�ͷ�����Ȩ
		T* release()noexcept
		{
			T* ret = m_ptr;
			m_ptr = nullptr;
			return ret;
		}

		//����ָ��
		void reset(T* ptr = nullptr)noexcept
		{
			T* old = m_ptr;
			m_ptr = ptr;
			if (old)
				delete old;
		}

		//����ָ��
		void swap(unique_ptr& other)noexcept
		{
			using mystl::swap;
			swap(m_ptr, other.m_ptr);
		}
	};
	//�����ػ�
	template<class T>
	void swap(unique_ptr<T>& lhs, unique_ptr<T>& rhs)noexcept
	{
		lhs.swap(rhs);
	}
}
