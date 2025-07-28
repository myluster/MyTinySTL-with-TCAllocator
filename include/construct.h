#pragma once
#include <new>  //new在此头文件内
#include "type_traits.h"
#include "iterator.h"
#include "utils.h"
namespace mystl {

    // 以下的函数内部并不复杂，实际上是只需要一两条语句就可以完成的“小操作”
    // 将这些小操作写成函数的好处有很多，比如方便阅读、修改、重用、统一行为。
    // 但调用函数会比使用等价的表达式慢很多：大多数时候，调用前要先保存寄存器，并在返回时恢复，复制实参，程序还必须转向一个新位置执行
    // 为了避免调用函数带来的开销，我们可以使用inline关键字将函数变为内联函数，内联函数会在程序的调用点上“内联地”展开；并且函数被内联后，编译器可以通过上下文相关的优化的技术对结果代码执行更深入的优化。最终达到提升效率的目的。

    // 全局 construct 函数，使用 new 将一个值放入指针所指内存
    template <typename T>
    inline void construct(T* ptr) {
        new (ptr) T();//调用默认构造函数
    }
    // 使用了两个模板参数，并使用显式转换
    template <typename T1, typename T2>
    inline void construct(T1* ptr, const T2& value) {
        new (ptr) T1(value);
    }

    template <typename T1, typename T2>
    inline void construct(T1* ptr, T2&& value) {
        new (ptr) T1(mystl::forward<T2>(value));
    }

    template<typename T, typename... Args>//可变参数模板(C++11)
    inline void construct(T* ptr, Args&&... args)//两个&&这里利用了引用折叠规则（见util.h）
    {
        ::new((void*)ptr) T(mystl::forward<Args>(args)...);//移动语义或拷贝语义（由完美转发的是左值还是右值决定）
    }


    // 单个参数的全局 destroy 函数
    //对是否平凡做出判断
    template <typename T>
    inline void destroy(T* ptr)
    {
        using is_trivial_dtor = typename type_traits<T>::has_trivial_destructor;
        destroy(ptr, is_trivial_dtor());
    }

    template <typename T>
    inline void destroy(T* ptr, _true_type) {}

    template <typename T>
    inline void destroy(T* ptr, _false_type) {
        ptr->~T();//非平凡调用析构函数
    }

    //对迭代器处理
    template <typename ForwardIterator>
    inline void destroy(ForwardIterator first,
        ForwardIterator last)
    {
        using value_type = typename mystl::iterator_traits<ForwardIterator>::value_type;
        using is_trivial_dtor = typename type_traits<value_type>::has_trivial_destructor;
        destroy_aux(first, last, is_trivial_dtor());
    }

    template <typename ForwardIterator>
    inline void destroy_aux(ForwardIterator first,
        ForwardIterator last,
        _true_type) {
    }

    template <typename ForwardIterator>
    inline void destroy_aux(ForwardIterator first,
        ForwardIterator last,
        _false_type) {
        for (; first != last; ++first)
            destroy(&*first);
        //对迭代器first解引用，然后获取它指向的元素的引用
        //为什么要这样做：
        // 无论迭代器是原生指针还是类类型，都能正确传递元素
        // 如果直接&，取得是forwardIter这个迭代器变量本身的地址
        // 先使用*，获取迭代器对象中存的元素地址（元素引用T&）
        // 再使用&，&作用于引用T&时，直接获取引用目标T&的地址
    }
}