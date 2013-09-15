#if defined(__INANITY_WINDOWS)

#pragma once

#define SCOPE_PROFILER( NAME ) ScopeProfiler scopeProfiler( #NAME , __FILE__ , __LINE__ );

class ScopeProfiler
{
public:
	ScopeProfiler(const char* codeAlias, const char* fileName, unsigned long lineNumber);
	~ScopeProfiler();

protected:
	virtual void processResult(float dt);

private:
	const char* _codeAlias;
	const char* _fileName;
	unsigned long _lineNumber;

	float _secondsPerCount;

	__int64 _start;
};

#else

#define SCOPE_PROFILER( NAME )

#endif