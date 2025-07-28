#pragma once
#include <new>  //new�ڴ�ͷ�ļ���
#include "type_traits.h"
#include "iterator.h"
#include "utils.h"
namespace mystl {

    // ���µĺ����ڲ��������ӣ�ʵ������ֻ��Ҫһ�������Ϳ�����ɵġ�С������
    // ����ЩС����д�ɺ����ĺô��кܶ࣬���緽���Ķ����޸ġ����á�ͳһ��Ϊ��
    // �����ú������ʹ�õȼ۵ı��ʽ���ࣺܶ�����ʱ�򣬵���ǰҪ�ȱ���Ĵ��������ڷ���ʱ�ָ�������ʵ�Σ����򻹱���ת��һ����λ��ִ��
    // Ϊ�˱�����ú��������Ŀ��������ǿ���ʹ��inline�ؼ��ֽ�������Ϊ���������������������ڳ���ĵ��õ��ϡ������ء�չ�������Һ����������󣬱���������ͨ����������ص��Ż��ļ����Խ������ִ�и�������Ż������մﵽ����Ч�ʵ�Ŀ�ġ�

    // ȫ�� construct ������ʹ�� new ��һ��ֵ����ָ����ָ�ڴ�
    template <typename T>
    inline void construct(T* ptr) {
        new (ptr) T();//����Ĭ�Ϲ��캯��
    }
    // ʹ��������ģ���������ʹ����ʽת��
    template <typename T1, typename T2>
    inline void construct(T1* ptr, const T2& value) {
        new (ptr) T1(value);
    }

    template <typename T1, typename T2>
    inline void construct(T1* ptr, T2&& value) {
        new (ptr) T1(mystl::forward<T2>(value));
    }

    template<typename T, typename... Args>//�ɱ����ģ��(C++11)
    inline void construct(T* ptr, Args&&... args)//����&&���������������۵����򣨼�util.h��
    {
        ::new((void*)ptr) T(mystl::forward<Args>(args)...);//�ƶ�����򿽱����壨������ת��������ֵ������ֵ������
    }


    // ����������ȫ�� destroy ����
    //���Ƿ�ƽ�������ж�
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
        ptr->~T();//��ƽ��������������
    }

    //�Ե���������
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
        //�Ե�����first�����ã�Ȼ���ȡ��ָ���Ԫ�ص�����
        //ΪʲôҪ��������
        // ���۵�������ԭ��ָ�뻹�������ͣ�������ȷ����Ԫ��
        // ���ֱ��&��ȡ����forwardIter�����������������ĵ�ַ
        // ��ʹ��*����ȡ�����������д��Ԫ�ص�ַ��Ԫ������T&��
        // ��ʹ��&��&����������T&ʱ��ֱ�ӻ�ȡ����Ŀ��T&�ĵ�ַ
    }
}