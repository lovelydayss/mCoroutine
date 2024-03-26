#ifndef MCOROUTINE_COMMON_COROUTINE_MEMORY_H
#define MCOROUTINE_COMMON_COROUTINE_MEMORY_H

#include "coroutine/utils.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

MCOROUTINE_NAMESPACE_BEGIN

class Memory {
public:
	using u_ptr = std::unique_ptr<Memory>;
	using s_ptr = std::shared_ptr<Memory>;

public:
	Memory(uint32_t block_size, uint32_t block_count);
	~Memory();

	uint32_t getRefCount() { return m_ref_counts; }

	uint8_t* getStart() { return m_start.get(); }
	uint8_t* getEnd() { return m_start.get() + m_size - 1; }

	uint8_t* getBlock();
	void backBlock(const uint8_t* addr);
	bool hasBlock(const uint8_t* addr);

private:
	uint32_t m_block_size{0};  // 内存块字节数
	uint32_t m_block_count{0}; // 内存块数目

	uint32_t m_size{0}; // 内存字节数
	std::unique_ptr<uint8_t> m_start; // 内存池内部空间起始位置，持有内存所有权

	std::atomic<uint32_t> m_ref_counts{0}; // 引用内存块数
	std::vector<bool> m_blocks; // 块引用标志，改用 bitset 节约空间
	std::mutex m_mutex;         // 内存池锁
};

MCOROUTINE_NAMESPACE_END


#endif