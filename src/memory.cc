#include "coroutine/memory.h"
#include "coroutine/utils.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <mutex>
#include <utility>

MCOROUTINE_NAMESPACE_BEGIN

MemoryBlockGroup::MemoryBlockGroup(uint32_t block_size,
                                   uint32_t block_group_size) noexcept {
	
	m_start = std::unique_ptr<uint8_t>(static_cast<uint8_t*>(malloc(block_group_size * block_size)));	
	m_use_flags.resize(block_group_size, false);
	// std::fill(m_use_flags.begin(), m_use_flags.end(), false);

	INFOFMTLOG("succ mmap {} bytes in memory pool, group_size = {}, block_size = {}, start address = {}",
	           block_group_size * block_size, block_group_size, block_size, (void*)m_start.get());
}

MemoryPool::MemoryPool(uint32_t block_size, uint32_t block_count,
                       uint32_t block_group_size /* = 128 */)
    : m_block_size(block_size)
    , m_block_group_size(block_group_size) {

	if (unlikely(block_size == 0))
		ERRORFMTLOG("error, block_size == 0 may caused undefined behavior!!");

	uint32_t group_count = block_count / m_block_group_size + 1;
	m_all_counts = group_count * block_group_size;

	for (int i = 0; i < group_count; i++)
		m_block_groups.emplace_back(m_block_size, m_block_group_size);
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
			       distance(use_flags.begin(), search_block) * m_block_size;
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

		std::lock_guard<std::mutex> lk(m_mutex);

		auto& start = m_block_groups[temp.second];
		uint32_t index = (addr - start.m_start.get()) / m_block_size;
		start.m_use_flags[index] = false;
		return;
	}

	ERRORFMTLOG("error, this block is not belong to this memory pool");
}

bool MemoryPool::hasBlock(const uint8_t* addr) {
	return hasBlockHelp(addr).first;
}

void MemoryPool::recovery() {

	std::lock_guard<std::mutex> lk(m_mutex);
	uint32_t count = 0;

	// 这里数组动态长度在变，所以不能用 range 写法
	for (int i = 0; i < m_block_groups.size();) {

		auto& use_flags = m_block_groups[i].m_use_flags;
		auto search_block = std::find(use_flags.begin(), use_flags.end(), true);

		if (search_block ==
		    use_flags.end()) { // 组内全部块均未使用，可以释放掉该空间

			std::swap(m_block_groups[i], m_block_groups.back());
			m_block_groups.pop_back();

			m_all_counts -= m_block_group_size;
		}
		else {
			i++;
		}
	}
}

std::pair<bool, uint32_t> MemoryPool::hasBlockHelp(const uint8_t* addr) {

	std::lock_guard<std::mutex> lk(m_mutex);

	for (uint32_t i = 0; i < m_block_groups.size(); i++) {

		auto& group_start = m_block_groups[i].m_start;
		if (addr >= group_start.get() &&
		    (addr < group_start.get() + m_block_group_size * m_block_size))
			return std::make_pair(true, i);
	}

	return std::make_pair(false, UINT32_MAX);
}

bool MemoryPool::usedBlockHelp(const uint8_t* addr) {

	std::lock_guard<std::mutex> lk(m_mutex);

	for (auto& m_block_group : m_block_groups) {

		auto& group_start = m_block_group.m_start;

		if (addr >= group_start.get() &&
		    (addr < group_start.get() + m_block_group_size * m_block_size) &&
		    m_block_group
		        .m_use_flags[(addr - group_start.get()) / m_block_size])
			return true;
	}
	return false;
}

MCOROUTINE_NAMESPACE_END
