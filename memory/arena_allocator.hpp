#ifndef __FBE_ARENA_ALLOCATOR_HPP__
#define __FBE_ARENA_ALLOCATOR_HPP__

#include <cstdlib>
#include <assert.h>

class ArenaAllocator
{
public:
	ArenaAllocator(size_t totalSize) : _allocatedSize(0), _totalSize(totalSize)
	{
		_memory = static_cast<unsigned char*>(malloc(totalSize));
	}


	virtual ~ArenaAllocator()
	{
		free(static_cast<void*>(_memory));
	}


	inline void* alloc(size_t size)
	{
		if (_allocatedSize + size <= _totalSize)
		{
			unsigned char* address = _memory + _allocatedSize;
			_allocatedSize += size;
			return static_cast<void*>(address);
		}
		else
		{
			return nullptr;
		}
	}


	inline void purge()
	{
		_allocatedSize = 0;
	}

private:
	size_t _totalSize;
	size_t _allocatedSize;

	unsigned char* _memory;
};


#endif