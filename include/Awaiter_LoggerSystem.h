#pragma once
#include <coroutine>

class Awaiter_LoggerSystem
{
public:
	bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle) noexcept;

	void await_resume() noexcept;

private:

};

