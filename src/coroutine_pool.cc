#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <cstdint>
#include <mutex>
#include "coroutine/coroutine_pool.h"
#include "coroutine/coroutine.h"


MCOROUTINE_NAMESPACE_BEGIN

static Config::u_ptr g_coroutine_pool_config = nullptr;
static std::once_flag sigleton_coroutine_pool_config;

Config::ptr Config::GetGlobalConfig() {
    std::call_once(sigleton_coroutine_pool_config, [](){

        CREATEPOLLTHREAD(MILLISECONDS(100));
        SETLOGLEVEL(fmtlog::LogLevel::DBG);
	    SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");
        
        g_coroutine_pool_config = MAKE_UNIQUE(Config);
    });

    return g_coroutine_pool_config.get();
}

void Config::SetGlobalConfig(uint32_t pool_size, uint32_t stack_size) {

    GetGlobalConfig()->m_pool_size = pool_size;
    GetGlobalConfig()->m_stack_size = stack_size;

}

static CoroutinePool::u_ptr g_coroutine_pool = nullptr;
static std::once_flag sigleton_coroutine_pool;

CoroutinePool::ptr CoroutinePool::GetGlobalCoroutinePool() {

	std::call_once(sigleton_coroutine_pool, []() {
		auto config = Config::GetGlobalConfig();
		g_coroutine_pool = MAKE_UNIQUE(CoroutinePool, config->m_pool_size,
		                               config->m_stack_size);
	});

	return g_coroutine_pool.get();

}

CoroutinePool::CoroutinePool(uint32_t pool_size,
                             uint32_t stack_size /* = 1024 * 128 */) : m_stack_size(stack_size) {

	m_memory_pool = MAKE_UNIQUE(MemoryPool, pool_size, stack_size);

    m_coroutines.reserve(static_cast<uint32_t>(pool_size + 1));     // reserve to pool_size + 1
	m_coroutines.emplace_back(true);    // problem: copy construct
   
	resize(pool_size);
}

Coroutine::ptr CoroutinePool::getCoroutine() {

    std::lock_guard<std::mutex> lk(m_mutex);

    for(uint32_t i = 1; i <= getPoolSize(); i++) {

        // the coroutine has coroutine not be used
        if(m_coroutines[i].second == false) {
            m_coroutines[i].second = true;
            return m_coroutines[i].first.get();
        }

    }

    // create new coroutine 
    uint32_t old_size = m_coroutines.size();
    resize(static_cast<uint32_t>(1.5 * old_size));
    m_coroutines[old_size].second = true;
    return m_coroutines[old_size].first.get();

}

void CoroutinePool::backCoroutine(Coroutine::ptr cor) {
	uint32_t index = cor->getCorId();

	std::lock_guard<std::mutex> lk(m_mutex);

	if (unlikely(m_coroutines[index].second == false)) {
		DEBUGFMTLOG("the coroutine[id = {}, address = {}] is not be used!",
		            index, static_cast<void*>(cor));
		return;
	}

	m_coroutines[index].second = false; // set coroutine as not used
    m_coroutines[index].first->setCallBackRunningFlag(false);
    m_coroutines[index].first->setResumeFlag(false);

    DEBUGFMTLOG("back coroutin[{}]", cor->getCorId());

}

uint32_t CoroutinePool::getPoolSize() const {
        return m_coroutines.size() - 1;
}

void CoroutinePool::resize(uint32_t new_size) {

    if(unlikely(new_size <= getPoolSize())) {
        DEBUGFMTLOG("the coroutine pools can only be scaled up!");
        return;
    }

	// m_coroutines.reserve(static_cast<uint32_t>(1.5 * new_size));
    for(uint32_t i = m_coroutines.size(); i <= new_size; i++)
        m_coroutines.emplace_back(false, m_stack_size, m_memory_pool->getBlock());

}


MCOROUTINE_NAMESPACE_END