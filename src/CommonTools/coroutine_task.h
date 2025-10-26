#pragma once
#include <coroutine>
#include <atomic>
#include <iostream>
#include "CommonToolsExport.h"

COMMON_TOOLS_EXPORT std::atomic<size_t> g_coroutine_count = 0;

// 协程返回类型
struct COMMON_TOOLS_EXPORT task
{
	struct promise_type
	{
		// 1、协程的返回对象
		task get_return_object() 
		{ 
			g_coroutine_count.fetch_add(1, std::memory_order_relaxed); 
			//std::cout << "创建新的协程，当前协程数为:" << g_coroutine_count << '\n';  
			return task{ std::coroutine_handle<promise_type>::from_promise(*this) }; 
		}

		// 2、初始协程状态，suspend_never为不挂起：立即执行
		std::suspend_never initial_suspend() noexcept { return {}; }

		// 3、协程结束状态：挂起
		std::suspend_always final_suspend() noexcept { return {}; }

		// 4、协程void返回类型
		void return_void() {}

		// 5、协程的异常处理
		void unhandled_exception() noexcept {}
	};
	// 协程句柄，用于控制协程
	std::coroutine_handle<promise_type> m_handle;
	// 1.默认构造
	task() : m_handle(nullptr) {}
	// 2.移动构造
	task(task&& other) noexcept : m_handle(other.m_handle) 
	{
		other.m_handle = nullptr; // 转移所有权
	}
	// 3.移动赋值
	task& operator=(task&& other) noexcept 
	{
		if (this != &other) {
			if (m_handle) m_handle.destroy(); // 释放当前句柄
			m_handle = other.m_handle;
			other.m_handle = nullptr; // 转移所有权
		}
		return *this;
	}
	
	task(const task&) = delete;
	task& operator=(const task&) = delete;
	task(std::coroutine_handle<promise_type> _h) : m_handle(_h) {}
	~task() 
	{
		// 控制协程销毁防止失效
		if (m_handle) 
		{
			m_handle.destroy();
			g_coroutine_count.fetch_sub(1, std::memory_order_relaxed); 
			//std::cout << "释放协程，当前协程数为:" << g_coroutine_count << '\n';
		}
	} 
};