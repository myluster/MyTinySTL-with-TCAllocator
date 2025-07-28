# C++11 STL 标准库与高性能内存池实现
[![Language](https://img.shields.io/badge/language-C++11-blue.svg)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE) 本项目是一个基于 **C++11** 标准实现的 **STL (标准模板库)**，并深度整合了一个**三层式高性能内存池**。它不仅是对 STL 的一次完整复刻，更是一个系统性的学习与实践平台，旨在整合 C++ 现代语法、核心算法、并发编程、内存管理及项目架构设计。

整个开发过程被详细记录，包含了丰富的学习笔记、心路历程以及在关键节点绘制的技术示意图，希望能为同样走在 C++ 学习道路上的朋友们提供一些参考。

参考项目与文档:
* [ghost-him/memory_pool](https://github.com/ghost-him/memory_pool)
* [Alinshans/MyTinySTL](https://github.com/Alinshans/MyTinySTL)
* [SGI-STL 二级空间配置器学习笔记](https://choubin.site/2020/01/07/STL3-SGI2StageAllocator/)

---

## 核心特点

* **现代 C++ 实现**: 大部分基于 C++11 标准，使用右值引用、`auto`、`lambda` 表达式等新特性，代码风格现代、简洁。
* **高性能内存管理**: 集成了一个三层式的高性能内存池 (`memory pool`)，显著减少 `new`/`delete` 带来的性能开销和内存碎片，是学习和实践内存管理的绝佳案例。
* **详尽的 STL 容器与算法**: 实现了大部分常用的 STL 容器 (如 `vector`, `list`, `map`, `unordered_map`) 和相关算法，深入理解其底层数据结构与设计哲学。
* **为学习而生**: 本项目的最大特色是其**详实的学习文档与代码演进痕迹**。记录了开发过程中遇到的挑战、踩过的坑，并在重要和复杂的逻辑部分配有**自己梳理的示意图**，力求将抽象的理论可视化、具体化。

## 项目目标

创建这个项目的主要目的不是重复造轮子，而是通过亲手实现一个复杂的、基础的 C++ 组件库，达到以下学习目标：

1.  **深化 C++ 语言理解**: 在实践中应用和理解 C++11/14/17 的各项语法细节。
2.  **掌握数据结构与算法**: 通过实现 STL 容器，彻底搞懂各种数据结构的底层原理与性能优劣。
3.  **探索内存管理机制**: 从零开始设计并实现一个高性能内存池，理解操作系统内存分配的原理以及内存优化的重要性。
4.  **提升项目架构能力**: 学习如何组织一个中小型 C++ 项目，包括代码结构、模块划分和接口设计。

## 学习笔记示例
<img width="3146" height="2530" alt="image" src="https://github.com/user-attachments/assets/d1dd22db-551a-4ac6-a5de-803240fa8379" />
