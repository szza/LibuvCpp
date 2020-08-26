# uv-webserver

## 介绍
以[`Libuv`](https://github.com/libuv/libuv)为基础，实现异步回调机制的TCP协议通讯，并且基于TCP协议实现了HTTP协议。用Valgrind检测，在正常使用下，未发现内存泄漏。主要特性如下：
+ 处于内存安全，代码中涉及指针地方多数使用智能指针。为数不多的使用裸指针，比如是因为生命周期的考虑在回调函数中使用裸指针。
+ C++11风格的回调函数，支持非静态类成员函数以及Lambda表达式、
+ TCP封装：`TcpServer`, `TcpConnection`,`TcpClient`, `Acceptor`
+ Async：异步回调机制的封装，相对原生的Libuv async接口，增加了调用多次只运行一次的问题，并且增加了`writeInLoop`和`write`两种接口，对应是否线程安全
+ Signal：Libuv信号封装
+ `Timer/TimerWheel`：Libuv定时器封装
+ 设计了`CycleBuffer`来缓存数据
+ 日志使用的是基于C++11实现的高性能多线程日志库[`NanoLog`]

详细使用可参考[src](./src)及[example](./example)

**编译**

```bash
    mkdir build && cd build 
    cmake .. && make 
```
**运行测试案例**
```bash
    cd build/bin
    // 根据需求测试运行各个
```

## 参考链接
+ [libuv](https://github.com/libuv/libuv)
