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

// configure the coroutine pool
class CoroutinePoolConfig {
public:
    using s_ptr = std::shared_ptr<CoroutinePoolConfig>;
    using u_ptr = std::unique_ptr<CoroutinePoolConfig>;

public:
    static CoroutinePoolConfig* GetGlobalConfig();
    static void setGlobalConfig(uint32_t pool_size, uint32_t stack_size);

public:
    uint32_t m_pool_size{128};
    uint32_t m_stack_size{1024 * 128};

};

// handle working coroutine 
class CoroutinePool {

public:
    using s_ptr = std::shared_ptr<CoroutinePool>;
    using u_ptr = std::unique_ptr<CoroutinePool>;

public:
    explicit CoroutinePool(uint32_t pool_size, uint32_t stack_size = 1024 * 128);
    ~CoroutinePool() = default;

    CoroutinePool(const CoroutinePool& rhs) = delete;
    CoroutinePool& operator=(const CoroutinePool& rhs) = delete;

public:
    Coroutine* getCoroutine();
    void backCoroutine(Coroutine* cor); 

    void resize(uint32_t new_size);      // just use to allocator more memory 

public:
    static CoroutinePool* GetGlobalCoroutinePool();

private:
	const uint32_t m_stack_size{1024 * 128};

    std::vector<std::pair<Coroutine::u_ptr, bool>> m_coroutines;    // true -> used
	MemoryPool::u_ptr m_memory_pool;

    std::mutex m_mutex;
};

MCOROUTINE_NAMESPACE_END

#endif