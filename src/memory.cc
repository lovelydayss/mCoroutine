#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <mutex>
#include <utility>

MCOROUTINE_NAMESPACE_BEGIN

MemoryBlockGroup::MemoryBlockGroup(uint32_t block_size,
                                   uint32_t block_group_size) noexcept {
	m_start = std::make_unique<uint8_t>(block_group_size * block_size);
	INFOFMTLOG("succ mmap {0} bytes memory", block_group_size * block_size);

	m_use_flags.resize(block_group_size, false);
	// std::fill(m_use_flags.begin(), m_use_flags.end(), false);
}

MemoryPool::MemoryPool(uint32_t block_size, uint32_t block_count,
                       uint32_t block_group_size /* = 128*/)
    : m_block_size(block_size)
    , m_block_group_size(block_group_size) {

	uint32_t group_count = block_count / m_block_group_size + 1;

	m_all_counts = group_count * block_group_size;
	m_block_groups.resize(
	    group_count,
	    MemoryBlockGroup(block_size,
	                     m_block_group_size)); // 直接使用 resize 进行元素构造
}

uint8_t* MemoryPool::getBlock() {

	std::lock_guard<std::mutex> lk(m_mutex);

	for (auto& block_group : m_block_groups) {

		auto& use_flags = block_group.m_use_flags;
		auto search_block =
		    std::find(use_flags.begin(), use_flags.end(), false);

		// 找到可分配内存块，直接返回即可
		if (search_block != use_flags.end()) {
			*search_block = true;
			m_use_counts++; // 分配成功
			return block_group.m_start.get() +
			       distance(use_flags.begin(), search_block);
		}
	}

	// 没有找到可用块，重新分配数组
	m_block_groups.emplace_back(m_block_size, m_block_group_size);
	m_use_counts++;
	m_all_counts += m_block_group_size;

	m_block_groups.back().m_use_flags[0] = true;
	return m_block_groups.back().m_start.get();
}

void MemoryPool::backBlock(uint8_t* addr) {

	auto temp = hasBlockHelp(addr);
	if (temp.first) {

		m_use_counts--;

		{
			std::lock_guard<std::mutex> lk(m_mutex);

			auto& start = m_block_groups[temp.second];
			start.m_use_flags[std::distance(start.m_start.get(), addr)];
			return;
		}
	}

	ERRORFMTLOG("error, this block is not belong to this Memory%");
}

bool MemoryPool::hasBlock(const uint8_t* addr) {
	return hasBlockHelp(addr).first;
}

void MemoryPool::recovery() {

	std::lock_guard<std::mutex> lk(m_mutex);

	for (auto& block_group : m_block_groups) {

		auto& use_flags = block_group.m_use_flags;
		auto search_block = std::find(use_flags.begin(), use_flags.end(), true);

		if (search_block == use_flags.end()) { // 组内全部块均未使用，可以释放掉该空间
			std::swap(block_group, m_block_groups.back());
			m_block_groups.pop_back();
		}
	}
}

std::pair<bool, uint32_t> MemoryPool::hasBlockHelp(const uint8_t* addr) {

	std::lock_guard<std::mutex> lk(m_mutex);

	for (uint32_t i = 0; i < m_block_groups.size(); i++) {

		auto& group_start = m_block_groups[i].m_start;
		if (addr >= group_start.get() &&
		    addr <= group_start.get() + m_block_group_size)
			return std::make_pair(true, i);
	}

	return std::make_pair(false, UINT32_MAX);
}

MCOROUTINE_NAMESPACE_END
