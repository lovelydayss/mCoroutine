#ifndef MCOROUTINE_COROUTINE_COROUTINE_POOL_H
#define MCOROUTINE_COROUTINE_COROUTINE_POOL_H

#include "coroutine/coctx.h"
#include "coroutine/coroutine.h"
#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

MCOROUTINE_NAMESPACE_BEGIN

// configure the coroutine pool
class Config {
public:
    using s_ptr = std::shared_ptr<Config>;
    using u_ptr = std::unique_ptr<Config>;
    using ptr = Config*;

public:
    static Config::ptr GetGlobalConfig();
    static void SetGlobalConfig(uint32_t pool_size, uint32_t stack_size);

public:
    std::atomic<uint32_t> m_pool_size{128};
    std::atomic<uint32_t> m_stack_size{1024 * 128};

};

// handle working coroutine 
class CoroutinePool {

private:
	class CoroutinePair {
	public:
		template <typename... T>
		explicit CoroutinePair(bool use_flag, T... args)
		    : first(MAKE_UNIQUE(Coroutine, args...))
		    , second(use_flag) {}

        // emplace_back ..... (not use but if set it as delete it will can't complete compile)
        // CoroutinePair(const CoroutinePair& rhs) = delete;
        // CoroutinePair& operator=(const CoroutinePair& rhs) = delete;

	public:
		Coroutine::u_ptr first; // the coroutine
		bool second;            // use flag
	};

public:
    using s_ptr = std::shared_ptr<CoroutinePool>;
    using u_ptr = std::unique_ptr<CoroutinePool>;
    using ptr = CoroutinePool*;

public:
    explicit CoroutinePool(uint32_t pool_size, uint32_t stack_size = 1024 * 128);
    ~CoroutinePool() = default;

    CoroutinePool(const CoroutinePool& rhs) = delete;
    CoroutinePool& operator=(const CoroutinePool& rhs) = delete;

public:
    Coroutine::ptr getCoroutine();
    void backCoroutine(Coroutine::ptr cor); 

    uint32_t getPoolSize() const;
    void resize(uint32_t new_size);

public:
    static CoroutinePool::ptr GetGlobalCoroutinePool();

public:
	const uint32_t m_stack_size{1024 * 128};

    std::vector<CoroutinePair> m_coroutines;
	MemoryPool::u_ptr m_memory_pool;

    std::mutex m_mutex;
};

MCOROUTINE_NAMESPACE_END

#endif