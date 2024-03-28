#ifndef MCOROUTINE_COROUTINE_COROUTINE_H
#define MCOROUTINE_COROUTINE_COROUTINE_H

#include "coroutine/coctx.h"
#include "coroutine/utils.h"
#include <cstdint>
#include <functional>
#include <memory>

MCOROUTINE_NAMESPACE_BEGIN

class Coroutine {

public:
    using u_ptr = std::unique_ptr<Coroutine>;
    using s_ptr = std::shared_ptr<Coroutine>;

private:
public:
	static void Yield();
	static void Resume(Coroutine* cor);

	static Coroutine* GetCurrentCoroutine();
	static Coroutine* GetMainCoroutine();
	static bool IsMainCoroutine();

private:
	uint32_t m_cor_id{0};       // coroutine ID
	coctx m_coctx;              // coroutine regs
	uint32_t m_stack_size{0};   // coroutine stack size
	uint8_t* m_stack_sp{nullptr};   // coroutine stack pointer 
	bool m_is_in_cofunc{false}; // flase -> the callback dunc not completed

	// std::string m_msg_no;       // message no
	// RunTime m_run_time;
    
    bool m_can_resume{true};
    int32_t m_index{-1};

public:
	std::function<void()> m_call_back;
};

MCOROUTINE_NAMESPACE_END

#endif