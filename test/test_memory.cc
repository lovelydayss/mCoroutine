#include "coroutine/memory.h"
#include "coroutine/utils.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("fmtlog") {

	SETLOGLEVEL(fmtlog::LogLevel::DBG);
	// fmtlog::setLogLevel(fmtlog::LogLevel::OFF);
	SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

	DEBUGFMTLOG("test of the memory pool begin!");

}

TEST_CASE("Memory pool Basic") {

	// set private member to public to make this test
	mcoroutine::MemoryPool test1(128, 60);

	CHECK_EQ(test1.m_use_counts, 0);
	CHECK_EQ(test1.m_block_size, 128);
	CHECK_EQ(test1.m_block_group_size, 128);
	CHECK_EQ(test1.m_block_groups.size(), 1);
	CHECK_EQ(test1.m_all_counts, 128);
	CHECK_EQ(test1.getAllCount(), test1.m_all_counts);
	CHECK_EQ(test1.getUseCount(), test1.m_use_counts);

	for (auto item : test1.m_block_groups[0].m_use_flags)
		CHECK_EQ(item, false);

#if 0

	mcoroutine::MemoryPool test2(128, 312, 100);

	CHECK_EQ(test2.m_use_counts, 0);
	CHECK_EQ(test2.m_block_size, 128);
	CHECK_EQ(test2.m_block_group_size, 100);
	CHECK_EQ(test2.m_block_groups.size(), 4);
	CHECK_EQ(test2.m_all_counts, 400);
	CHECK_EQ(test2.getAllCount(), test2.m_all_counts);
	CHECK_EQ(test2.getUseCount(), test2.m_use_counts);

#endif

	std::vector<uint8_t*> test_vec;
	for (int i = 0; i < 1000; i++) {

		auto m = test1.getBlock();
		// MESSAGE(m);
		CHECK_EQ(test1.m_use_counts, i + 1);

		auto pair = test1.hasBlockHelp(m);
		CHECK_EQ(pair.first, true);
        CHECK_EQ(test1.usedBlockHelp(m), true);

        test_vec.push_back(m);
	}

	for(auto& item : test1.m_block_groups) {
		// MESSAGE(item.m_start.get());
	}

	for (int i = 0; i < 500; i++) {

		test1.backBlock(test_vec[i]);
		// CHECK_EQ(test1.m_use_counts, 1000 - i - 1);

		// MESSAGE(test_vec[i]);

		auto pair = test1.hasBlockHelp(test_vec[i]);
		CHECK_EQ(pair.first, true);
        CHECK_MESSAGE(test1.usedBlockHelp(test_vec[i]) == false, i);
    
    }

	CHECK_EQ(test1.m_use_counts, 500);
	CHECK_EQ(test1.m_all_counts, 1024);

	test1.recovery();

	CHECK_EQ(test1.m_use_counts, 500);
	CHECK_EQ(test1.m_all_counts, 1024 - 384);


	for (int i = 500; i < 1000; i++) {

		test1.backBlock(test_vec[i]);
		// CHECK_EQ(test1.m_use_counts, 1000 - i - 1);

		// MESSAGE(test_vec[i]);

		auto pair = test1.hasBlockHelp(test_vec[i]);
		CHECK_EQ(pair.first, true);
        CHECK_MESSAGE(test1.usedBlockHelp(test_vec[i]) == false, i);
	}

	CHECK_EQ(test1.m_use_counts, 0);
	CHECK_EQ(test1.m_all_counts, 1024 - 384);

	test1.recovery();

	CHECK_EQ(test1.m_use_counts, 0);
	CHECK_EQ(test1.m_all_counts, 0);

	std::vector<uint8_t*> test_vec1;
	for (int i = 0; i < 1000; i++) {

		auto m = test1.getBlock();
		// MESSAGE(m);
		CHECK_EQ(test1.m_use_counts, i + 1);

		auto pair = test1.hasBlockHelp(m);
		CHECK_EQ(pair.first, true);
        CHECK_EQ(test1.usedBlockHelp(m), true);

        test_vec1.push_back(m);
	}

	CHECK_EQ(test1.m_use_counts, 1000);
	CHECK_EQ(test1.m_all_counts, 1024);

	DEBUGFMTLOG("test of the memory pool end!");

}