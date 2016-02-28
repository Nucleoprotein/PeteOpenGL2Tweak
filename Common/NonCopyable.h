#pragma once

// An inheritable class to disallow the copy constructor and operator= functions
class NonCopyable
{
protected:
	NonCopyable() {}
	~NonCopyable() {}

	NonCopyable(const NonCopyable&) = delete;
	const NonCopyable& operator=(NonCopyable& other) = delete;
};
