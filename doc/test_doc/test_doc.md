# 开发及测试记录

## 单元测试

### Memory Pool

**问题：在创建一个指针 (unique_ptr) 指向一块内存时，不能使用 make_unique 函数**

**问题：使用unique_ptr 管理连续内存时需要自定义释放器**

#### 测试结果

![](images/1.png)





### Coroutine Pool

大坑，vector reserve 会调用拷贝构造？
似乎在 vector 不为空的时候使用 reserve 会调用一次拷贝构造

xmake f --toolchain=clang 切换 clang https://xmake.io/mirror/zh-cn/toolchain/builtin_toolchains.html
-m debug 切换debug模式 https://www.cnblogs.com/tboox/p/11994383.html
xmake f -v 查看配置文件

### hook function