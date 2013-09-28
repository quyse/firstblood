#ifndef __FBE_POOL_ALLOCATOR_HPP__
#define __FBE_POOL_ALLOCATOR_HPP__

#include <cstdlib>
#include <vector>
#include <assert.h>

// supports maximum of 65536 objects
// first two bytes of each unused chunk serve as a pointer to the next free chunk
// dtors of the objects allocated via pool won't be called
class PoolAllocator
{
public:
	PoolAllocator(size_t chunkSize, size_t chunksCount) : _chunkSize(chunkSize), _chunksCount(chunksCount), _firstFreeChunkOffset(0), _allocatedCount(0)
	{
		assert(chunkSize >= 2);
		assert(chunksCount > 0);
		_memory = static_cast<unsigned char*>(malloc(_chunksCount * _chunkSize));
		for (size_t i = 0; i < _chunksCount; ++i)
		{
			size_t memoryOffset = i * _chunkSize;
			*(_memory + memoryOffset) = (i + 1) & 255;
			*(_memory + memoryOffset + 1) = ((i + 1) >> 8) & 255;
		}
	}

	~PoolAllocator()
	{
		free(static_cast<void*>(_memory));
		for (size_t i = 0; i < _overflowAllocations.size(); ++i)
			free(_overflowAllocations[i]);
		_overflowAllocations.clear();
	}

	inline void* allocMemory(size_t size)
	{
		assert(size <= _chunkSize);
		if (_allocatedCount < _chunksCount)
		{
			++_allocatedCount;
			unsigned char* chunk = _memory + _firstFreeChunkOffset * _chunkSize;
			_firstFreeChunkOffset = *chunk + (*(chunk + 1) << 8);
			return static_cast<void*>(chunk);
		}
		else
		{	// we don't want to crash our customer in case of pool overflow
			// but still, it should be treated as a bug
			assert(false);
			void* overflowMemory = malloc(size);
			_overflowAllocations.push_back(overflowMemory);
			return overflowMemory;
		}
	}

	template<class T>
	inline T* alloc()
	{
		void* memory = allocMemory(sizeof(T));
		return new (memory) T;
	}

	inline void dealloc(void* memory)
	{
		unsigned char* freedMemory = static_cast<unsigned char*>(memory);
		if (freedMemory >= _memory && freedMemory <= _memory + _chunkSize * (_chunksCount - 1))
		{
			assert((size_t)(freedMemory - _memory) % _chunkSize == 0);
			assert(_allocatedCount > 0);
			--_allocatedCount;
			*freedMemory = _firstFreeChunkOffset & 255;
			*(freedMemory + 1) = (_firstFreeChunkOffset >> 8) & 255;
			_firstFreeChunkOffset = (freedMemory - _memory) / _chunkSize;
		}
		else
		{
			assert(false);
			size_t overflowAllocationsCount = _overflowAllocations.size();
			for (size_t i = 0; i < overflowAllocationsCount; ++i)
			{
				if (_overflowAllocations[i] == memory)
				{
					free(_overflowAllocations[i]);
					_overflowAllocations[i] = _overflowAllocations[overflowAllocationsCount - 1];
					_overflowAllocations.pop_back();
					break;
				}
			}
		}
	}

private:
	unsigned int _firstFreeChunkOffset;
	unsigned char* _memory;
	size_t _allocatedCount;
	size_t _chunksCount;
	size_t _chunkSize;
	std::vector<void*> _overflowAllocations;
};

#endif