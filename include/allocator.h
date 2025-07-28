#pragma once
#include "construct.h"
#include "utils.h" // 确保 utils.h 提供了 mystl::move 和 mystl::forward
//接入TCMalloc
#include "TCMalloc/ThreadCache.h"
#include "TCMalloc/PageCache.h"
#include "TCMalloc/TCMallocBootstrap.h"
namespace mystl
{
    // 模板类allocator
    // 简单的空间配置器
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

        // 函数声明
        static pointer allocate();
        static pointer allocate(size_type n);

        static void deallocate(pointer ptr);
        static void deallocate(pointer ptr, size_type n); // 兼容标准容器接口

        // 构造对象，转发到 mystl::construct
        static void construct(pointer ptr);
        static void construct(pointer ptr, const_reference value);
        static void construct(pointer ptr, value_type&& value); // C++11 右值引用

        template <class... Args>
        static void construct(pointer ptr, Args&& ...args); // C++11 可变参数模板，完美转发

        // 销毁对象，转发到 mystl::destroy
        static void destroy(pointer ptr);
        static void destroy(pointer first, pointer last);

        template <class U>
        struct rebind {
            typedef allocator<U> other;
        };
    };

    // 函数实现

    
    //原始allocator
    // 分配未初始化的内存，返回指向内存块的指针
    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate()
    {
        return static_cast<typename allocator<T>::pointer>(::operator new(sizeof(T)));
    }
    //为什么使用typename allocator<T>::pointer而不直接使用T*
    //为了泛型编程

    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate(typename allocator<T>::size_type n)
    {
        if (n == 0)
            return static_cast<typename allocator<T>::pointer>(nullptr); // 明确返回 nullptr

        return static_cast<typename allocator<T>::pointer>(::operator new(n * sizeof(T)));
    }
    
    /*
    //接入TCMalloc
    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate()
    {
        return allocate(1);
    }

    template<class T>
    typename allocator<T>::pointer allocator<T>::allocate(typename allocator<T>::size_type n)
    {
        if (n == 0)
            return static_cast<typename allocator<T>::pointer>(nullptr); // 明确返回 nullptr

        size_t total_size = n * sizeof(T);
        void* result = nullptr;

        //当处于构造阶段 或 申请的是大内存时
        if (g_is_allocator_constructing || total_size > size_utils::MAX_CACHED_UNIT_SIZE)
        {
            //大内存直接进行申请
            std::optional<span<byte>> page = page_cache::allocate_unit(total_size);
            if (page)
            {
                result = page->data();
            }
        }
        else
        {
            //小内存接入ThreadCache
            result = thread_cache::get_instance().allocate(total_size).value_or(nullptr);
        }

        return static_cast<pointer>(result);
    }
    //接入结束
    */
    //原始allocator实现
    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr)
    {
        // operator delete 已经可以处理 nullptr 了
        ::operator delete(ptr);
    }

    template<class T>
    void allocator<T>::deallocate(typename allocator<T>::pointer ptr, typename allocator<T>::size_type n)// 参数n未使用，可加注释
    {
        deallocate(ptr); // 内部仍调用单参数版本
    }

    /*
    //接入TCMalloc
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
    //接入结束
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
        mystl::construct(ptr, mystl::move(value)); // 使用 mystl::move
    }

    template<class T>
    template<class ...Args> // 这里的 template 关键字只作用于内层成员函数模板
    void allocator<T>::construct(typename allocator<T>::pointer ptr, Args&&...args) // 完美转发，左右值都行
    {
        mystl::construct(ptr, mystl::forward<Args>(args)...); // 使用 mystl::forward
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

    // destroy和deallocate的区别
    // destroy：对象析构
    // 对象生命周期终结：调用对象的析构函数,释放对象占用的非内存资源（如文件句柄、锁、动态分配的子对象等）
    // 不释放内存：仅析构对象，内存块本身仍保留（可复用或稍后通过 deallocate 释放）
    // deallocate：内存释放
    // 内存资源回收：释放由allocate分配的原始内存块，归还给系统或内存池
    // 不调用析构函数：仅操作内存，不关心内存中的对象状态（若对象未析构destroy直接 deallocate，会导致资源泄漏）

} // namespace mystl