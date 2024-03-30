## MCOROUTINE

a tiny coroutine library

### 编译环境

C++14 环境 / C++11 + make_unique
C++17 -> 日志支持
C++11 doctest 进行代码测试

### 

采用 unique_ptr 进行内存资源管理，接口传递裸指针方式处理内存
采用 call_once 实现线程安全单例模式