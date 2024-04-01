#include "coroutine/coroutine.h"
#include "coroutine/utils.h"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>

MCOROUTINE_NAMESPACE_BEGIN

static thread_local Coroutine::u_ptr t_main_coroutine = nullptr; // main coroutine
static thread_local std::once_flag t_singleton_main_coroutine; // main coroutine once flag

static thread_local Coroutine::ptr t_cur_coroutine = nullptr; // current coroutine(working coroutine)

static std::atomic<uint32_t> t_cur_coroutine_id{1}; // 0 -> main coroutine (for every thread have a 0 coroutine)

void runCallBack(Coroutine::ptr co) { /* NOLINT */

	if (unlikely(co == nullptr)) {
		ERRORFMTLOG("the coroutine can't be nullptr!");
		exit(1);
	}

	co->setCallBackRunningFlag(true);
	co->getCallBack()(); // runing calllbcak function
	co->setCallBackRunningFlag(false);

	Coroutine::Yield();
}

Coroutine::Coroutine(uint32_t stack_size, uint8_t* stack_ptr)
    : m_cor_id(t_cur_coroutine_id++) 
	, m_stack_size(stack_size)
    , m_stack_sp(stack_ptr) { /* NOLINT */

	if (unlikely(stack_ptr == nullptr)) {
		ERRORFMTLOG("the stack pointer can't be nullptr!");
		exit(1);
	}
	// memset(&m_coctx, 0, sizeof(m_coctx));	// use when set callback
}

Coroutine::Coroutine(uint32_t stack_size, uint8_t* stack_ptr,
                     const std::function<void()>& cb)
    : m_cor_id(t_cur_coroutine_id++) 
	, m_stack_size(stack_size)
    , m_stack_sp(stack_ptr) { /* NOLINT */

	if (unlikely(stack_ptr == nullptr)) {
		ERRORFMTLOG("the stack pointer can't be nullptr!");
		exit(1);
	}

	setCallBack(cb);		// set callbackl function
}

bool Coroutine::getCallBackRunningFlag() const { return m_cbfunc_running_flag; }

void Coroutine::setCallBackRunningFlag(bool cofunc_runing_flag) {
	m_cbfunc_running_flag = cofunc_runing_flag;
}

bool Coroutine::getResumeFlag() const { return m_resume_flag; }
void Coroutine::setResumeFlag(bool resume_flag) { m_resume_flag = resume_flag; }


const std::function<void()>& Coroutine::getCallBack() const {
	return m_call_back;
}
bool Coroutine::setCallBack(const std::function<void()>& cb) {

	if (unlikely(this == Coroutine::GetMainCoroutine())) {
		ERRORFMTLOG("main coroutine can't set callback!");
		return false;
	}

	if (unlikely(m_cbfunc_running_flag == true)) {
		ERRORFMTLOG("this coroutine callback is running!");
		return false;
	}

	m_call_back = cb;
	uint8_t* top = m_stack_sp + m_stack_size;

	top = reinterpret_cast<uint8_t*>((reinterpret_cast<unsigned long>(top)) &
	                                 -16LL); /* NOLINT */

	memset(&m_coctx, 0, sizeof(m_coctx));

	m_coctx.regs[kRSP] = top;
	m_coctx.regs[kRBP] = top;
	m_coctx.regs[kRETAddr] = reinterpret_cast<uint8_t*>(runCallBack);
	m_coctx.regs[kRDI] = reinterpret_cast<uint8_t*>(this);

	m_resume_flag = true;

	return true;
}

// static function begin

void Coroutine::Yield() {

	if (unlikely(GetCurrentCoroutine() == GetMainCoroutine())) {
		ERRORFMTLOG("current coroutine is main coroutine!");
		exit(1);
	}

	Coroutine::ptr cor = GetCurrentCoroutine();
	t_cur_coroutine = GetMainCoroutine();
	coctx_swap(&(cor->m_coctx), &(GetMainCoroutine()->m_coctx));
}

void Coroutine::Resume(Coroutine::ptr cor) {

	if (unlikely(GetCurrentCoroutine() != GetMainCoroutine())) {
		ERRORFMTLOG("swap error, current coroutine must be main coroutine!");
		exit(1);
	}

	if (unlikely(cor == nullptr || cor->m_resume_flag == false)) {
		ERRORFMTLOG("pending coroutine is nullptr or resume flag is false!");
		exit(1);
	}

	if (unlikely(GetCurrentCoroutine() == cor)) {
		DEBUGFMTLOG("current coroutine is pending cor, need't swap!");
		return;
	}

	t_cur_coroutine = cor;
	coctx_swap(&(t_main_coroutine->m_coctx), &(cor->m_coctx));
}

Coroutine::ptr Coroutine::GetMainCoroutine() {

	std::call_once(t_singleton_main_coroutine, []() {
		t_main_coroutine = MAKE_UNIQUE(Coroutine);
		DEBUGFMTLOG("create main coroutine in this thread!");

		// memset(&(t_main_coroutine->m_coctx), 0, sizeof(t_main_coroutine->m_coctx));
	});

	return t_main_coroutine.get();
}

Coroutine::ptr Coroutine::GetCurrentCoroutine() {

	if (unlikely(t_cur_coroutine == nullptr)) {
		DEBUGFMTLOG(
		    "the current coroutine is nullptr, set it as main coroutine!");
		t_cur_coroutine = Coroutine::GetMainCoroutine();
	}

	return t_cur_coroutine;
}

bool Coroutine::IsMainCoroutine() {

	return GetMainCoroutine() == GetCurrentCoroutine();
}

// static function end

MCOROUTINE_NAMESPACE_END