#include "coroutine/coroutine.h"
#include "coroutine/coroutine_pool.h"
#include "coroutine/utils.h"
#include <cstdint>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("test for coroutine config") {

SETLOGLEVEL(fmtlog::LogLevel::DBG);
SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

mcoroutine::Config::GetGlobalConfig();

CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 128);
CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 1024 * 128);

mcoroutine::Config::setGlobalConfig(0, 0);

CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 0);
CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 0);

mcoroutine::Config::setGlobalConfig(0, 4096);

CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 0);
CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 4096);

}

TEST_CASE("test for coroutine pool (GET and BACK Coroutine)") {

DEBUGFMTLOG("test for coroutine pool begin!");

mcoroutine::Config::setGlobalConfig(1024, 1024 * 256);
auto test = mcoroutine::CoroutinePool::GetGlobalCoroutinePool();

CHECK_EQ(test->m_stack_size, 1024 * 256);
CHECK_EQ(test->getPoolSize(), 1024);
CHECK_EQ(test->m_coroutines.size(), test->getPoolSize() + 1);

CHECK_EQ(test->m_coroutines[0].second, true);

for(uint32_t i = 1; i <= test->getPoolSize(); i++) {
    
    auto test_cor_f = test->m_coroutines[i].first.get();
    auto test_cor_s = test->m_coroutines[i].second;

    CHECK_EQ(test_cor_s, false);
    CHECK_EQ(test_cor_f->getCorId(), i);
    CHECK_EQ(test_cor_f->getStackSize(), test->m_stack_size);
    CHECK_NE(test_cor_f->getStackPtr(), nullptr);
    CHECK_EQ(test_cor_f->getCallBackRunningFlag(), false);
    CHECK_EQ(test_cor_f->getResumeFlag(), false);

}

std::vector<mcoroutine::Coroutine::ptr> test_get(1);
for(uint32_t i = 1; i <= 2000; i++) {

    auto temp = mcoroutine::CoroutinePool::GetGlobalCoroutinePool()->getCoroutine();
    
    auto test_cor_f = test->m_coroutines[i].first.get();
    auto test_cor_s = test->m_coroutines[i].second;

    CHECK_EQ(temp->getCorId(), i);
    CHECK_EQ(temp->getStackSize(), test->m_stack_size);
    CHECK_NE(temp->getStackPtr(), nullptr);
    CHECK_EQ(temp->getCallBackRunningFlag(), false);
    CHECK_EQ(temp->getResumeFlag(), false);

    CHECK_EQ(test_cor_s, true);
    CHECK_EQ(test_cor_f, temp);

    test_get.push_back(temp);
}

for(uint32_t i = 1; i <= 2000; i++) {
    
    auto temp = test_get[i];
    mcoroutine::CoroutinePool::GetGlobalCoroutinePool()->backCoroutine(temp);

    auto test_cor_f = test->m_coroutines[i].first.get();
    auto test_cor_s = test->m_coroutines[i].second;

    CHECK_EQ(temp->getCorId(), i);
    CHECK_EQ(temp->getStackSize(), test->m_stack_size);
    CHECK_NE(temp->getStackPtr(), nullptr);
    CHECK_EQ(temp->getCallBackRunningFlag(), false);
    CHECK_EQ(temp->getResumeFlag(), false);

    CHECK_EQ(test_cor_s, false);
    CHECK_EQ(test_cor_f, temp);

}

DEBUGFMTLOG("test for coroutine pool end!");


}

TEST_CASE("test for coroutine pool (Coroutine Operator)") {
    



}