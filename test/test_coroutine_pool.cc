#include "coroutine/coroutine.h"
#include "coroutine/coroutine_pool.h"
#include "coroutine/utils.h"
#include <chrono>
#include <cstdint>
#include <thread>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("test for coroutine config") {

	SETLOGLEVEL(fmtlog::LogLevel::DBG);
	SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

	mcoroutine::Config::GetGlobalConfig();

	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 128);
	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 1024 * 128);

	mcoroutine::Config::SetGlobalConfig(0, 0);

	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 0);
	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 0);

	mcoroutine::Config::SetGlobalConfig(0, 4096);

	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_pool_size, 0);
	CHECK_EQ(mcoroutine::Config::GetGlobalConfig()->m_stack_size, 4096);
}

TEST_CASE("test for coroutine pool ") {

	DEBUGFMTLOG("test for coroutine pool begin!");
	mcoroutine::Config::SetGlobalConfig(1024, 1024 * 256);

	SUBCASE("GET and BACK Coroutine") {

		auto test = mcoroutine::CoroutinePool::GetGlobalCoroutinePool();

		CHECK_EQ(test->m_stack_size, 1024 * 256);
		CHECK_EQ(test->getPoolSize(), 1024);
		CHECK_EQ(test->m_coroutines.size(), test->getPoolSize() + 1);

		CHECK_EQ(test->m_coroutines[0].second, true);

		for (uint32_t i = 1; i <= test->getPoolSize(); i++) {

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
		for (uint32_t i = 1; i <= 2000; i++) {

			auto temp = mcoroutine::CoroutinePool::GetGlobalCoroutinePool()
			                ->getCoroutine();

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

		for (uint32_t i = 1; i <= 2000; i++) {

			auto temp = test_get[i];
			mcoroutine::CoroutinePool::GetGlobalCoroutinePool()->backCoroutine(
			    temp);

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
	}

	SUBCASE("Coroutine Operator") {

		CHECK(mcoroutine::Coroutine::IsMainCoroutine());

		auto test_pool = mcoroutine::CoroutinePool::GetGlobalCoroutinePool();
		auto test_cor = test_pool->getCoroutine();
		CHECK(mcoroutine::Coroutine::IsMainCoroutine());

		CHECK_EQ(test_cor->getCorId(), 1);
		CHECK_EQ(test_cor->getResumeFlag(), false);
		CHECK_EQ(test_cor->getCallBackRunningFlag(), false);

		test_cor->setCallBack([&]() {
			auto main_coroutine = mcoroutine::Coroutine::GetMainCoroutine();
			auto current_coroutine =
			    mcoroutine::Coroutine::GetCurrentCoroutine();

			INFOFMTLOG("running callback function in coroutine[{}]",
			           current_coroutine->getCorId());

			CHECK_EQ(current_coroutine, test_cor);
			CHECK_EQ(current_coroutine->getCallBackRunningFlag(), true);

			INFOFMTLOG("callback function in coroutine[{}] end!",
			           current_coroutine->getCorId());
		});

		CHECK(mcoroutine::Coroutine::IsMainCoroutine());
		CHECK_EQ(test_cor->getResumeFlag(), true);

		mcoroutine::Coroutine::Resume(test_cor);
		// ... the coroutine is runing , wait yeild

		CHECK_EQ(test_cor->getCallBackRunningFlag(), false);
		CHECK(mcoroutine::Coroutine::IsMainCoroutine());

		mcoroutine::CoroutinePool::GetGlobalCoroutinePool()->backCoroutine(test_cor);
	}

	SUBCASE("Coroutine Multi-thread Test") {
		
		std::vector<std::thread> test_threads;
		test_threads.reserve(10);
		
		for (int i = 0; i < 10; i++) {
			test_threads.emplace_back([]() {

				INFOFMTLOG("create thread!");

				CHECK(mcoroutine::Coroutine::IsMainCoroutine());

				auto test_pool =
				    mcoroutine::CoroutinePool::GetGlobalCoroutinePool();
				auto test_cor = test_pool->getCoroutine();
				CHECK(mcoroutine::Coroutine::IsMainCoroutine());

				// CHECK_EQ(test_cor->getCorId(), 1);
				CHECK_EQ(test_cor->getResumeFlag(), false);
				CHECK_EQ(test_cor->getCallBackRunningFlag(), false);

				test_cor->setCallBack([&]() {
					auto main_coroutine =
					    mcoroutine::Coroutine::GetMainCoroutine();
					auto current_coroutine =
					    mcoroutine::Coroutine::GetCurrentCoroutine();

					INFOFMTLOG("running callback function in coroutine[{}]",
					           current_coroutine->getCorId());

					CHECK_EQ(current_coroutine, test_cor);
					CHECK_EQ(current_coroutine->getCallBackRunningFlag(), true);

					INFOFMTLOG("callback function in coroutine[{}] end!",
					           current_coroutine->getCorId());

					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				});

				CHECK(mcoroutine::Coroutine::IsMainCoroutine());
				CHECK_EQ(test_cor->getResumeFlag(), true);

				mcoroutine::Coroutine::Resume(test_cor);
				// ... the coroutine is runing , wait yeild

				CHECK_EQ(test_cor->getCallBackRunningFlag(), false);
				CHECK(mcoroutine::Coroutine::IsMainCoroutine());

				mcoroutine::CoroutinePool::GetGlobalCoroutinePool()->backCoroutine(test_cor);

			});
		}

		for(auto& item : test_threads)
			item.join();

	}

	DEBUGFMTLOG("test for coroutine pool end!");
}