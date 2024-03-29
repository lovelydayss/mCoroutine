#ifndef MCOROUTINE_COROUTINE_COROUTINE_POOL_H
#define MCOROUTINE_COROUTINE_COROUTINE_POOL_H

#include "coroutine/coctx.h"
#include "coroutine/coroutine.h"
#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

MCOROUTINE_NAMESPACE_BEGIN

class CoroutinePool {

public:
    using s_ptr = std::shared_ptr<CoroutinePool>;
    using u_ptr = std::unique_ptr<CoroutinePool>;

public:
    explicit CoroutinePool(uint32_t pool_size, uint32_t stack_size = 1024 * 128);
    ~CoroutinePool() = default;

private:
	uint32_t m_pool_size{0};        // 协程池内协程数目   
	uint32_t m_stack_size{0};       // 协程栈大小

    std::vector<std::pair<Coroutine::u_ptr, bool>> m_free_coroutines;
	MemoryPool::u_ptr m_memory_pool;

    std::mutex m_mutex;
};

MCOROUTINE_NAMESPACE_END

#endif