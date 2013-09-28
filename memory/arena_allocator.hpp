#ifndef __FBE_ARENA_ALLOCATOR_HPP__
#define __FBE_ARENA_ALLOCATOR_HPP__

#include <cstdlib>
#include <assert.h>
#include <vector>

// dtors of allocated created via arena won't be called
class ArenaAllocator
{
public:
	ArenaAllocator(size_t totalSize) : _allocatedSize(0), _totalSize(totalSize)
	{
		_memory = static_cast<unsigned char*>(malloc(totalSize));
	}

	~ArenaAllocator()
	{
		free(static_cast<void*>(_memory));
		clearOverflowAllocations();
	}

	inline void* allocMemory(size_t size)
	{
		if (_allocatedSize + size <= _totalSize)
		{
			unsigned char* address = _memory + _allocatedSize;
			_allocatedSize += size;
			return static_cast<void*>(address);
		}
		else
		{
			// we don't want to crash our customer in case of arena overflow
			// but still, it should be treated as a bug
			assert(false);
			void* overflowAllocation = malloc(size);
			_overflowAllocations.push_back(overflowAllocation);
			return overflowAllocation;
		}
	}

	template<class T>
	inline T* alloc()
	{
		void* memory = allocMemory(sizeof(T));
		return new (memory) T;
	}

	inline void purge()
	{
		_allocatedSize = 0;
		clearOverflowAllocations();
	}

private:
	inline void clearOverflowAllocations()
	{
		for (size_t i = 0; i < _overflowAllocations.size(); ++i)
			free(_overflowAllocations[i]);
		_overflowAllocations.clear();
	}

private:
	size_t _totalSize;
	size_t _allocatedSize;
	unsigned char* _memory;
	std::vector<void*> _overflowAllocations;
};


#endif