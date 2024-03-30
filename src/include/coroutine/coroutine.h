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

public:
	Coroutine() = default;
	Coroutine(uint32_t stack_size, uint8_t* stack_ptr);
	Coroutine(uint32_t stack_size, uint8_t* stack_ptr, const std::function<void()>& cb);

	~Coroutine() = default;

	Coroutine(const Coroutine& cor) = delete;
	Coroutine& operator=(const Coroutine& cor) = delete;

public:

	uint32_t getCorId() const { return m_cor_id; }

	uint8_t* getStackPtr() { return m_stack_sp; }
	uint32_t getStackSize() const { return m_stack_size; }

	bool getCallBackRunningFlag() const;
	void setCallBackRunningFlag(bool cofunc_runing_flag);

	bool getResumeFlag() const;
	void setResumeFlag(bool resume_flag);

	const std::function<void()>& getCallBack() const;
	bool setCallBack(const std::function<void()>& cb);

public:
	static void Yield();               // target coroutine -> main coroutine
	static void Resume(Coroutine* cor); // main coroutine -> target coroutine

	static Coroutine* GetMainCoroutine();
	static Coroutine* GetCurrentCoroutine();

	static bool IsMainCoroutine();

private:
	const uint32_t m_cor_id{0};   // coroutine ID

	coctx m_coctx{};              // coroutine regs
	uint32_t m_stack_size{0};     // coroutine stack size
	uint8_t* m_stack_sp{nullptr}; // coroutine stack pointer

	bool m_cbfunc_running_flag{false}; // true -> the callback is running, flase -> the callback func is completed
	bool m_resume_flag{true}; // true -> the coroutine can resume, false -> can't resume

	std::function<void()> m_call_back; // coroutine callback function

};

MCOROUTINE_NAMESPACE_END

#endif