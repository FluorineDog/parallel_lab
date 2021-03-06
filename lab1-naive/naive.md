# lab1
## 前言
这里主要使用了各种并行工具实现向量加法。

由于向量加法是典型的内存密集型应用，其计算/访问内存比为0.5，
实际上在打开Release后
编译器进行自动向量化已经可以填满内存带宽，
多核/GPU并不能提高效率，还可能因为增加了内存拷贝开销导致时间增加，
因此预期大多数程序结果为负优化。

## baseline
选用串行算法作为baseline. 同时，书写了统一的接口方便测试与性能对比。

在common.h中，提前申请所需内存，并填充初值.

为了减小误差，计时前运行一次程序作为warming up，随后多次运行向量加法程序，取运行时间均值。

为了避免编译器自动将计算过程优化掉，使用随机下标进行一次随机取值。

最后，计算结果的hash值，确保算法的正确性。

## pthread
对于pthread，我选用了C++11标准封装好的thread标准库，使用modern C++代码
风格书写高性能代码。
首先，我需要创建线程，线程数量为系统最大可支持的并行度。
创建完成时子线程即开始运行，此时只需在主线程中等待回收资源即可。
Debug模式下的对比得到了预期的优化效果：
code here
然而，由于Release下，自动优化的AVX256指令集已经可以填满内存带宽，因此效果不明显。

## openmp
在串行代码的基础上直接添加openmp编译制导指令即可

## MPI
MPI主要用于多机间通信，其在单机上的overhead极大。
加之本应用为通信密集型，因此不论在Release还是Debug模式下，预期应为负优化。
这里主要采用了Scatter和Gather函数，
Scatter将进程0中的数据分发到多个进程，随后在多个进程中进行计算，
计算完成后，使用Gather将结果收集到0号进程。
数据传输过程是整个程序的瓶颈所在，这是由应用特点决定的。
运行时，-np长度一般选择为机器数量。本实验在单机上选择2，仅对其正确性进行测试.

## CUDA 
使用GPU进行计算时，最大的不同有以下两点：
1. 数据需要拷贝至显存中才能进行计算
2. 基于SIMT架构，线程的数量越多越好。
参照这些规则，我们使用CUDA后计算如下


这里的核心语句时核函数VecAdd和它的调用。
核函数定义中，首先更加Block与thread的id，
计算数当前所需要处理的元素id，并进行元素加计算。
核函数调用时，需要填写<<<blockSize, threadSize>>>>这两个参数。
后者一般是有限制的，与显卡物理架构相关。
本实验测试于GTX1050Ti，限制为threadSize<=1024

