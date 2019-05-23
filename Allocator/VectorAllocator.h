//请一定要加上 -std=c++17编译！
#pragma once
#include <new>
#define _ALIGN 8
#define _MAX_SIZE 128
#define _LIST_LENGTH (_MAX_SIZE/_ALIGN)
#define _BLOCKNUM_PER_ALLOC 20
typedef struct tagNODE {
	tagNODE* Next;
}NODE;
static NODE* FreeList[_LIST_LENGTH] = { NULL };
//mempool
static void* StartAddr = NULL;
static void* EndAddr = NULL;
static size_t AllocSize = 0;
template<typename T>
class VectorAllocator {
public:
	typedef T* pointer;
	typedef const pointer const_pointer;
	typedef T value_type;
	typedef size_t size_type;
	typedef T& reference;
	typedef const reference const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type is_always_equal;

	template <typename _other> 
	struct rebind {
		typedef  std::allocator<_other> other;
	};

	//allocate n bytes
	pointer allocate (size_type n) {
		size_type blockSize = ROUND (n);
		if (n > (size_type)_MAX_SIZE) {
			//没有分配失败(X
			//if(!ptr)
			return malloc(n);
		} else if(FreeList[INDEX (n)]){
			//链表里有空余块
			pointer ptr = (pointer)FreeList[INDEX (n)];
			FreeList[INDEX (n)] = FreeList[INDEX (n)]->Next;
		} else {
			//链表里无空余块,从mempool里取出一些块放到freelist上
			ptrdiff_t size = EndAddr - StartAddr;
			if (blockSize <= size) {
				//至少能分出一块，最多分配k块到list上
				size_type blockNum = size / blockSize < _BLOCKNUM_PER_ALLOC ? size / blockSize : _BLOCKNUM_PER_ALLOC;
				for (int i = 0; i < blockNum; i++) {
					((NODE*)StartAddr)->Next = FreeList[INDEX (n)];
					FreeList[INDEX[n]] = StartAddr;
					StartAddr += blockSize;
				}
			} else {
				//最后一块不够分
				//先把这块分配掉
				((NODE*)StartAddr)->Next = FreeList[INDEX (size)];
				FreeList[INDEX (size)] = (NODE*)StartAddr;
				//再次获取内存池
				//分配大小为目前需求量的两倍+目前已分配空间的1/4
				size_type size = 2 * n + ROUND (AllocSize >> 2);
				StartAddr = malloc (size);
				if (StartAddr) {
					EndAddr = StartAddr + size;
					AllocSize += size;
				} else {
					//malloc fail
					int i = INDEX (n) + 1;
					for (; i < _LIST_LENGTH; i++) {
						if (FreeList[i]) {
							StartAddr = FreeList[i];
							EndAddr = StartAddr + i;
							FreeList[i] = FreeList[i]->Next;
							while (EndAddr - StartAddr >= blockSize) {
								((NODE*)StartAddr)->Next = FreeList[INDEX (n)];
								FreeList[INDEX (n)] = (NODE*)StartAddr;
								StartAddr += blockSize;
							}
							if (EndAddr - StartAddr) {
								size_type tmp = EndAddr - StartAddr;
								((NODE*)StartAddr)->Next = FreeList[INDEX (tmp)];
								FreeList[INDEX (tmp)] = (NODE*)StartAddr;
								StartAddr += blockSize;
							}
							break;
						}
					}
					if (i == _LIST_LENGTH) {
						//找不到需求的空间，GG
						throw std::bad_alloc;
					}
				}
			}
			//分配完了，重新调用函数
			return allocate (n);
		}
	}

	void deallocate (pointer ptr, size_t n) {
		if (n > (size_type)_MAX_SIZE) {
			free(ptr);
		} else {
			//mempool
			((NODE*)ptr)->Next = FreeList[INDEX (n)];
			FreeList[INDEX (n)] = (NODE*)ptr;
		}
	}
	
	//ptr称为布置参数，这个new用于在已alloc的内存上初始化成员（布置new）
	void construct (pointer ptr, const_reference val) {
		::new(ptr) T (val);
	}
	void destroy (pointer ptr) {
		ptr->~T ( );
	}
	pointer address (reference val) {
		return (pointer)&val;
	}
	const_pointer const_Address (const_reference val) {
		return (const_pointer)&val;
	}
private:
	inline size_type ROUND (size_type n) {
		return (n + _ALIGN - 1) & ~(_ALIGN - 1);
	}

	inline size_type INDEX (size_type n) {
		return ROUND (n) / _ALIGN - 1;
	}
};
