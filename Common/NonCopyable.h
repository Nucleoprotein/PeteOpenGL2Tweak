#pragma once

// An inheritable class to disallow the copy constructor and operator= functions
class NonCopyable
{
protected:
	NonCopyable() {}
	~NonCopyable() {}
private:
	NonCopyable( const NonCopyable& );
	const NonCopyable& operator=( NonCopyable& other );
};
