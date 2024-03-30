#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <cstdint>
#include <mutex>
#include <utility>
#include "coroutine/coroutine_pool.h"
#include "coroutine/coroutine.h"


MCOROUTINE_NAMESPACE_BEGIN

static CoroutinePoolConfig::u_ptr g_coroutine_pool_config = nullptr;
static std::once_flag sigleton_coroutine_pool_config;

static CoroutinePoolConfig* GetGlobalConfig() {
    std::call_once(sigleton_coroutine_pool_config, [](){
        g_coroutine_pool_config = MAKE_UNIQUE(CoroutinePoolConfig);
    });

    return g_coroutine_pool_config.get();
}

static void setGlobalConfig(uint32_t pool_size, uint32_t stack_size) {

    GetGlobalConfig()->m_pool_size = pool_size;
    GetGlobalConfig()->m_stack_size = stack_size;

}

static CoroutinePool::u_ptr g_coroutine_pool = nullptr;
static std::once_flag sigleton_coroutine_pool;

CoroutinePool* CoroutinePool::GetGlobalCoroutinePool() {

	std::call_once(sigleton_coroutine_pool, []() {
		auto config = CoroutinePoolConfig::GetGlobalConfig();
		g_coroutine_pool = MAKE_UNIQUE(CoroutinePool, config->m_pool_size,
		                               config->m_stack_size);
	});

	return g_coroutine_pool.get();

}

CoroutinePool::CoroutinePool(uint32_t pool_size,
                             uint32_t stack_size /* = 1024 * 128 */) : m_stack_size(stack_size) {

	m_memory_pool = MAKE_UNIQUE(MemoryPool, pool_size, stack_size);

	m_coroutines.reserve(static_cast<uint32_t>(1.5 * pool_size));
	m_coroutines.emplace_back(MAKE_UNIQUE(Coroutine),
	                          true); // index zero -> thread main coroutine

	// index 1 ~ UINT32_MAX  working thread
	for (uint32_t i = 1; i <= pool_size; i++)
        m_coroutines.emplace_back(MAKE_UNIQUE(Coroutine, stack_size, m_memory_pool->getBlock()), false);

}

Coroutine* CoroutinePool::getCoroutine() {

    std::lock_guard<std::mutex> lk(m_mutex);

    auto search_coroutine = std::find(m_coroutines.begin(), m_coroutines.end(), [](const std::pair<Coroutine, bool>& item){
        return item.second == false;
    });

    // the coroutine has coroutine not be used
    if(search_coroutine != m_coroutines.end()) {
        search_coroutine->second = true;
        return search_coroutine->first.get();
    }

    // create new coroutine 
    uint32_t old_size = m_coroutines.size();
    resize(static_cast<uint32_t>(1.5 * static_cast<double>(old_size)));
    m_coroutines[old_size].second = true;
    return m_coroutines[old_size].first.get();

}

void CoroutinePool::backCoroutine(Coroutine* cor) {
    uint32_t index = cor->getCorId();

    std::lock_guard<std::mutex> lk(m_mutex);

    if(m_coroutines[index].second == false) {
        DEBUGFMTLOG("the coroutine[id = {}, address = {}] is not be used!", index, static_cast<void*>(cor));
        return;
    }

    m_coroutines[index].second = false;     // set coroutine as not used

}
    
void CoroutinePool::resize(uint32_t new_size) {

    if(new_size <= m_coroutines.size()) {
        DEBUGFMTLOG("the coroutine pools can only be scaled up!");
        return;
    }

    for(uint32_t i = m_coroutines.size(); i < new_size; i++)
        m_coroutines.emplace_back(MAKE_UNIQUE(Coroutine, m_stack_size, m_memory_pool->getBlock()), false);

}


MCOROUTINE_NAMESPACE_END