#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include "gtest/gtest.h"
#include <iterator>

int main(int argc, char const *argv[]) {

    /*
    SETLOGLEVEL(fmtlog::LogLevel::DBG);
    // fmtlog::setLogLevel(fmtlog::LogLevel::OFF);
    SETLOGHEADER("[{l}] [{YmdHMSe}] [{t}] [{g}] ");

    DEBUGFMTLOG("SDASADASDASDASDAS   {}", 11111);
    INFOFMTLOG("AAAAAAAAAAAAAAAAAAAAAAAAAA");
    ERRORFMTLOG("CCCCCCCCCCCCCCCCCCCCCCCCC");

    */

    // set private member to public to make this test 
    mcoroutine::MemoryPool test1(128, 60);

    EXPECT_EQ(test1.m_use_counts, 0);
    EXPECT_EQ(test1.m_block_size, 128);
    EXPECT_EQ(test1.m_block_group_size, 128);
    EXPECT_EQ(test1.m_block_groups.size(), 1);
    EXPECT_EQ(test1.m_all_counts, 128);
    EXPECT_EQ(test1.getAllCount(), test1.m_all_counts);
    EXPECT_EQ(test1.getUseCount(), test1.m_use_counts);

    for(auto item : test1.m_block_groups[0].m_use_flags)
        EXPECT_EQ(item, false);

    mcoroutine::MemoryPool test2(128, 312, 100);

    EXPECT_EQ(test2.m_use_counts, 0);
    EXPECT_EQ(test2.m_block_size, 128);
    EXPECT_EQ(test2.m_block_group_size, 100);
    EXPECT_EQ(test2.m_block_groups.size(), 4);
    EXPECT_EQ(test2.m_all_counts, 400);
    EXPECT_EQ(test2.getAllCount(), test1.m_all_counts);
    EXPECT_EQ(test2.getUseCount(), test1.m_use_counts);

    std::vector<uint8_t*> test_vec;
    for(int i = 0; i < 1000; i++) {

        auto m = test1.getBlock();
        EXPECT_EQ(test1.m_use_counts, i);

        auto pair = test1.hasBlockHelp(m);
        EXPECT_EQ(pair.first, true);
        EXPECT_EQ(pair.second, 1000 / test1.m_block_group_size);

        EXPECT_EQ(test1.m_block_groups[pair.second].m_use_flags[std::distance(test1.m_block_groups[pair.second].m_start.get(), m)], true);
        test_vec.push_back(m);
    }

    for(int i = 0; i < 500; i++) {

        test1.backBlock(test_vec[i]);
        EXPECT_EQ(test1.m_use_counts, 1000 - i);

        auto pair = test1.hasBlockHelp(test_vec[i]);
        EXPECT_EQ(pair.first, false);
    }

    EXPECT_EQ(test1.m_use_counts, 500);
    EXPECT_EQ(test1.m_all_counts, 1024);

    test1.recovery();

    EXPECT_EQ(test1.m_use_counts, 500);
    EXPECT_EQ(test1.m_all_counts, 1024 - 384);

    for(int i = 0; i < 500; i++) {

        test1.backBlock(test_vec[i]);
        EXPECT_EQ(test1.m_use_counts, 1000 - i);

        auto pair = test1.hasBlockHelp(test_vec[i]);
        EXPECT_EQ(pair.first, false);
    }

    EXPECT_EQ(test1.m_use_counts, 0);
    EXPECT_EQ(test1.m_all_counts, 1024 - 384);

    test1.recovery();

    EXPECT_EQ(test1.m_use_counts, 0);
    EXPECT_EQ(test1.m_all_counts, 0);

    return 0;

}