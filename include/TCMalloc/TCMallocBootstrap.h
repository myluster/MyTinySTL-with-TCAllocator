#pragma once
namespace mystl {
    //如果我们的任何分配器组件当前正在当前线程上构造，则此标志将为真
    inline thread_local bool g_is_allocator_constructing = false;
}