#ifndef MCOROUTINE_COROUTINE_MEMORY_H
#define MCOROUTINE_COROUTINE_MEMORY_H

#include "coroutine/utils.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

MCOROUTINE_NAMESPACE_BEGIN

class MemoryBlockGroup {

public:
	struct free_delete {
		void operator()(void* x) { free(x); }
	};

public:
	MemoryBlockGroup(uint32_t block_size, uint32_t block_group_size) noexcept;

public:
	std::unique_ptr<uint8_t, free_delete> m_start; // 内部空间起始位置，持有内存所有权
	std::vector<bool> m_use_flags{}; // 块引用标志，改用 bitset 节约空间
};

class MemoryPool {
public:
	using u_ptr = std::unique_ptr<MemoryPool>;
	using s_ptr = std::shared_ptr<MemoryPool>;

public:
	MemoryPool(uint32_t block_size, uint32_t block_count,
	           uint32_t block_group_size = 128);
	~MemoryPool() = default;

	uint32_t getUseCount() const { return m_use_counts; } // 获取已经使用块数目
	uint32_t getAllCount() const { return m_all_counts; } // 获取全部块数目

	uint8_t* getBlock();                // 读取内存块
	void backBlock(uint8_t* addr);      // 归还块
	bool hasBlock(const uint8_t* addr); // 判断是否存在块

	void recovery(); // 回收资源

private:
	std::pair<bool, uint32_t> hasBlockHelp(const uint8_t* addr);
	bool usedBlockHelp(const uint8_t* addr);

private:
	std::atomic<uint32_t> m_all_counts{0}; // 内存块总数
	std::atomic<uint32_t> m_use_counts{0}; // 已使用内存块数

	std::vector<MemoryBlockGroup>
	    m_block_groups{}; // 内存组数组，实现动态扩张且避免迭代器失效
	std::mutex m_mutex; // 内存池锁

	const uint32_t m_block_size;       // 内存块字节数
	const uint32_t m_block_group_size; // 单个内存组内内存块个数
};

MCOROUTINE_NAMESPACE_END

#endif