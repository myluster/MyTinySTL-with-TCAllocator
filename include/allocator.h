#pragma once
#include "construct.h"
#include "utils.h" // ȷ�� utils.h �ṩ�� mystl::move �� mystl::forward
//����TCMalloc
#include "TCMalloc/ThreadCache.h"
#include "TCMalloc/PageCache.h"
#include "TCMalloc/TCMallocBootstrap.h"
namespace mystl
{
    // ģ����allocator
    // �򵥵Ŀռ�������
    template <class T>
    class allocator
    {
    public:
        typedef T           value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef size_t      size_type;
        typedef ptrdiff_t   difference_type;

        // ��������
        static pointer allocate();
        static pointer allocate(size_type n);

        static void deallocate(pointer ptr);
        static void deallocate(pointer ptr, size_type n); // ���ݱ�׼�����ӿ�

        // �������ת���� mystl::construct
        static void construct(pointer ptr);
        static void construct(pointer ptr, const_reference value);
        static void construct(pointer ptr, value_type&& value); // C++11 ��ֵ����

        template <class... Args>
        static void construct(pointer ptr, Args&& ...args); // C++11 �ɱ����ģ�壬����ת��

        // ���ٶ���ת���� mystl::destroy
        static void destroy(pointer ptr);
        static void destroy(pointer first, pointer last);

        template <class U>
        struct rebind {
            typedef allocator<U> other;
        };
    };

    // ����ʵ��

    
    //ԭʼallocator
    // ����δ��ʼ�����ڴ棬����ָ���ڴ���ָ��
    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate()
    {
        return static_cast<typename allocator<T>::pointer>(::operator new(sizeof(T)));
    }
    //Ϊʲôʹ��typename allocator<T>::pointer����ֱ��ʹ��T*
    //Ϊ�˷��ͱ��

    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate(typename allocator<T>::size_type n)
    {
        if (n == 0)
            return static_cast<typename allocator<T>::pointer>(nullptr); // ��ȷ���� nullptr

        return static_cast<typename allocator<T>::pointer>(::operator new(n * sizeof(T)));
    }
    
    /*
    //����TCMalloc
    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate()
    {
        return allocate(1);
    }

    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate(typename allocator<T>::size_type n)
    {
        if (n == 0)
            return static_cast<typename allocator<T>::pointer>(nullptr); // ��ȷ���� nullptr

        size_t total_size = n * sizeof(T);
        void* result = nullptr;

        //�����ڹ���׶� �� ������Ǵ��ڴ�ʱ
        if (g_is_allocator_constructing || total_size > size_utils::MAX_CACHED_UNIT_SIZE)
        {
            //���ڴ�ֱ�ӽ�������
            std::optional<span<byte>> page = page_cache::allocate_unit(total_size);
            if (page)
            {
                result = page->data();
            }
        }
        else
        {
            //С�ڴ����ThreadCache
            result = thread_cache::get_instance().allocate(total_size).value_or(nullptr);
        }

        return static_cast<pointer>(result);
    }
    //�������
    */
    //ԭʼallocatorʵ��
    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr)
    {
        // operator delete �Ѿ����Դ��� nullptr ��
        ::operator delete(ptr);
    }

    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr, typename allocator<T>::size_type n)// ����nδʹ�ã��ɼ�ע��
    {
        deallocate(ptr); // �ڲ��Ե��õ������汾
    }

    /*
    //����TCMalloc
    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr)
    {
        deallocate(ptr, 1);
    }

    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr, typename allocator<T>::size_type n)
    {
        if (!ptr) return;

        size_t total_size = n * sizeof(T);

        if (total_size > size_utils::MAX_CACHED_UNIT_SIZE)
        {
            page_cache::deallocate_unit(mystl::span<byte>(reinterpret_cast<byte*>(ptr), total_size));
        }
        else
        {
            thread_cache::get_instance().deallocate(ptr, total_size);
        }
    }
    //�������
    */
    template<class T>
    void allocator<T>::construct(typename allocator<T>::pointer ptr)
    {
        mystl::construct(ptr);
    }

    template<class T>
    void allocator<T>::construct(typename allocator<T>::pointer ptr, typename allocator<T>::const_reference value)
    {
        mystl::construct(ptr, value);
    }

    template<class T>
    void allocator<T>::construct(typename allocator<T>::pointer ptr, typename allocator<T>::value_type&& value)
    {
        mystl::construct(ptr, mystl::move(value)); // ʹ�� mystl::move
    }

    template<class T>
    template<class ...Args> // ����� template �ؼ���ֻ�������ڲ��Ա����ģ��
    void allocator<T>::construct(typename allocator<T>::pointer ptr, Args&&...args) // ����ת��������ֵ����
    {
        mystl::construct(ptr, mystl::forward<Args>(args)...); // ʹ�� mystl::forward
    }

    template <class T>
    void allocator<T>::destroy(typename allocator<T>::pointer ptr)
    {
        mystl::destroy(ptr);
    }

    template <class T>
    void allocator<T>::destroy(typename allocator<T>::pointer first, typename allocator<T>::pointer last)
    {
        mystl::destroy(first, last);
    }

    // destroy��deallocate������
    // destroy����������
    // �������������ս᣺���ö������������,�ͷŶ���ռ�õķ��ڴ���Դ�����ļ������������̬������Ӷ���ȣ�
    // ���ͷ��ڴ棺�����������ڴ�鱾���Ա������ɸ��û��Ժ�ͨ�� deallocate �ͷţ�
    // deallocate���ڴ��ͷ�
    // �ڴ���Դ���գ��ͷ���allocate�����ԭʼ�ڴ�飬�黹��ϵͳ���ڴ��
    // �����������������������ڴ棬�������ڴ��еĶ���״̬��������δ����destroyֱ�� deallocate���ᵼ����Դй©��

} // namespace mystl