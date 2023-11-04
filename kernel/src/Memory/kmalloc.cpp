/*
Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "kmalloc.hpp"
#include "newdelete.hpp"

#include <stdint.h>
#include <stdio.h>
#include <util.h>
#include <assert.h>
#include <math.h>

#include <Memory/PageManager.hpp>

/*
How the allocator works:
It uses the mrvn LinkedListBucketHeapImplementation by Goswin von Brederlow <goswin-v-b@web.de>
Found at: https://wiki.osdev.org/User:Mrvn/LinkedListBucketHeapImplementation
*/

/*
START MRVN source
*/

typedef struct DList DList;
struct DList {
    DList *next;
    DList *prev;
};
 
// initialize a one element DList
static inline void dlist_init(DList *dlist) {
	//printf("%s(%p)\n", __FUNCTION__, dlist);
    dlist->next = dlist;
    dlist->prev = dlist;
}
 
// insert d2 after d1
static inline void dlist_insert_after(DList *d1, DList *d2) {
	//printf("%s(%p, %p)\n", __FUNCTION__, d1, d2);
    DList *n1 = d1->next;
    DList *e2 = d2->prev;
 
    d1->next = d2;
    d2->prev = d1;
    e2->next = n1;
    n1->prev = e2;
}
 
// insert d2 before d1
static inline void dlist_insert_before(DList *d1, DList *d2) {
	//printf("%s(%p, %p)\n", __FUNCTION__, d1, d2);
    DList *e1 = d1->prev;
    DList *e2 = d2->prev;
 
    e1->next = d2;
    d2->prev = e1;
    e2->next = d1;
    d1->prev = e2;
}
 
// remove d from the list
static inline void dlist_remove(DList *d) {
	//printf("%s(%p)\n", __FUNCTION__, d);
    d->prev->next = d->next;
    d->next->prev = d->prev;
    d->next = d;
    d->prev = d;    
}
 
// push d2 to the front of the d1p list
static inline void dlist_push(DList **d1p, DList *d2) {
	//printf("%s(%p, %p)\n", __FUNCTION__, d1p, d2);
    if (*d1p != NULL) {
	dlist_insert_before(*d1p, d2);
    }
    *d1p = d2;
}
 
// pop the front of the dp list
static inline DList * dlist_pop(DList **dp) {
	//printf("%s(%p)\n", __FUNCTION__, dp);
    DList *d1 = *dp;
    DList *d2 = d1->next;
    dlist_remove(d1);
    if (d1 == d2) {
	*dp = NULL;
    } else {
	*dp = d2;
    }
    return d1;
}
 
// remove d2 from the list, advancing d1p if needed
static inline void dlist_remove_from(DList **d1p, DList *d2) {
	//printf("%s(%p, %p)\n", __FUNCTION__, d1p, d2);
    if (*d1p == d2) {
	dlist_pop(d1p);
    } else {
	dlist_remove(d2);
    }
}
 
#define CONTAINER(C, l, v) ((C*)(((char*)v) - (intptr_t)&(((C*)0)->l)))
#define OFFSETOF(TYPE, MEMBER)  __builtin_offsetof (TYPE, MEMBER)
 
#define DLIST_INIT(v, l) dlist_init(&v->l)
 
#define DLIST_REMOVE_FROM(h, d, l)					\
    {									\
	__typeof__(**h) **h_ = h, *d_ = d;					\
	DList *head = &(*h_)->l;					\
	dlist_remove_from(&head, &d_->l);					\
	if (head == NULL) {						\
	    *h_ = NULL;							\
	} else {							\
	    *h_ = CONTAINER(__typeof__(**h), l, head);			\
	}								\
    }
 
#define DLIST_PUSH(h, v, l)						\
    {									\
	__typeof__(*v) **h_ = h, *v_ = v;					\
	DList *head = &(*h_)->l;					\
	if (*h_ == NULL) head = NULL;					\
	dlist_push(&head, &v_->l);					\
	*h_ = CONTAINER(__typeof__(*v), l, head);				\
    }
 
#define DLIST_POP(h, l)							\
    ({									\
	__typeof__(**h) **h_ = h;						\
	DList *head = &(*h_)->l;					\
	DList *res = dlist_pop(&head);					\
	if (head == NULL) {						\
	    *h_ = NULL;							\
	} else {							\
	    *h_ = CONTAINER(__typeof__(**h), l, head);			\
	}								\
	CONTAINER(__typeof__(**h), l, res);					\
    })
 
#define DLIST_ITERATOR_BEGIN(h, l, it)					\
    {									\
        __typeof__(*h) *h_ = h;						\
	DList *last_##it = h_->l.prev, *iter_##it = &h_->l, *next_##it;	\
	do {								\
	    if (iter_##it == last_##it) {				\
		next_##it = NULL;					\
	    } else {							\
		next_##it = iter_##it->next;				\
	    }								\
	    __typeof__(*h)* it = CONTAINER(__typeof__(*h), l, iter_##it);
 
#define DLIST_ITERATOR_END(it)						\
	} while((iter_##it = next_##it));				\
    }
 
#define DLIST_ITERATOR_REMOVE_FROM(h, it, l) DLIST_REMOVE_FROM(h, iter_##it, l)
 
typedef struct Chunk Chunk;
struct Chunk {
    DList all;
    int used;
    union {
	    char data[0];
	    DList free;
    };
};
 
enum {
    NUM_SIZES = 32,
    ALIGN = 4,
    MIN_SIZE = sizeof(DList),
    HEADER_SIZE = OFFSETOF(Chunk, data),
};
 
Chunk *free_chunk[NUM_SIZES] = { NULL };
size_t mem_free = 0;
size_t mem_used = 0;
size_t mem_meta = 0;
Chunk *first = NULL;
Chunk *last = NULL;
 
static void memory_chunk_init(Chunk *chunk) {
	//printf("%s(%p)\n", __FUNCTION__, chunk);
    DLIST_INIT(chunk, all);
    chunk->used = 0;
    DLIST_INIT(chunk, free);
}
 
static size_t memory_chunk_size(const Chunk* chunk) {
	//printf("%s(%p)\n", __FUNCTION__, chunk);
    char *end = (char*)(chunk->all.next);
    char *start = (char*)(&chunk->all);
    return (end - start) - HEADER_SIZE;
}
 
static int memory_chunk_slot(size_t size) {
    int n = -1;
    while(size > 0) {
	++n;
	size /= 2;
    }
    return n;
}

unsigned char* g_mem_start = nullptr;
unsigned char* g_mem_end = nullptr;
 
void mrvn_memory_init(void* mem, size_t size) {
    unsigned char* mem_start = (unsigned char*)(((intptr_t)mem + ALIGN - 1) & (~(ALIGN - 1)));
    unsigned char* mem_end = (unsigned char*)(((intptr_t)mem + size) & (~(ALIGN - 1)));
    g_mem_start = mem_start;
    g_mem_end = mem_end;
    first = (Chunk*)mem_start;
    Chunk *second = first + 1;
    last = ((Chunk*)mem_end) - 1;
    memory_chunk_init(first);
    memory_chunk_init(second);
    memory_chunk_init(last);
    dlist_insert_after(&first->all, &second->all);
    dlist_insert_after(&second->all, &last->all);
    // make first/last as used so they never get merged
    first->used = 1;
    last->used = 1;
 
    size_t len = memory_chunk_size(second);
    int n = memory_chunk_slot(len);
    //printf("%s(%p, %#lx) : adding chunk %#lx [%d]\n", __FUNCTION__, mem, size, len, n);
    DLIST_PUSH(&free_chunk[n], second, free);
    mem_free = len - HEADER_SIZE;
    mem_meta = sizeof(Chunk) * 2 + HEADER_SIZE;
}
 
void* mrvn_malloc(size_t size) {
    //dbgprintf("[%s(%lx)] INFO: mem_free = %lu, mem_used = %lu\n", __extension__ __PRETTY_FUNCTION__, size, mem_free, mem_used);
    size = (size + ALIGN - 1) & (~(ALIGN - 1));
 
	if (size < MIN_SIZE) size = MIN_SIZE;
 
	int n = memory_chunk_slot(size - 1) + 1;
 
	if (n >= NUM_SIZES) return NULL;
 
	while(!free_chunk[n]) {
		++n;
		if (n >= NUM_SIZES) return NULL;
    }
 
	Chunk *chunk = DLIST_POP(&free_chunk[n], free);
    size_t size2 = memory_chunk_size(chunk);
	//printf("@ %p [%#lx]\n", chunk, size2);
    size_t len = 0;
 
	if (size + sizeof(Chunk) <= size2) {
		Chunk *chunk2 = (Chunk*)(((char*)chunk) + HEADER_SIZE + size);
		memory_chunk_init(chunk2);
		dlist_insert_after(&chunk->all, &chunk2->all);
		len = memory_chunk_size(chunk2);
		int n = memory_chunk_slot(len);
		//printf("  adding chunk @ %p %#lx [%d]\n", chunk2, len, n);
		DLIST_PUSH(&free_chunk[n], chunk2, free);
		mem_meta += HEADER_SIZE;
		mem_free += len - HEADER_SIZE;
    }
 
	chunk->used = 1;
    //memset(chunk->data, 0xAA, size);
	//printf("AAAA\n");
    mem_free -= size2;
    mem_used += size2 - len - HEADER_SIZE;
    //dbgprintf("  = %lp [%lp]. mem_free = %lx\n", chunk->data, chunk, mem_free);
    return chunk->data;
}
 
static void remove_free(Chunk *chunk) {
    size_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);
    //printf("%s(%p) : removing chunk %#lx [%d]\n", __FUNCTION__, chunk, len, n);
    DLIST_REMOVE_FROM(&free_chunk[n], chunk, free);
    mem_free -= len - HEADER_SIZE;
}
 
static void push_free(Chunk *chunk) {
    size_t len = memory_chunk_size(chunk);
    int n = memory_chunk_slot(len);
    //printf("%s(%p) : adding chunk %#lx [%d]\n", __FUNCTION__, chunk, len, n);
    DLIST_PUSH(&free_chunk[n], chunk, free);
    mem_free += len - HEADER_SIZE;
}
 
void mrvn_free(void *mem) {
    /*if (mem < g_mem_start || mem > g_mem_end) {
        fprintf(VFS_DEBUG, "%s(%p): WARNING: block to free is outside of range. Not freeing...\n", __extension__ __PRETTY_FUNCTION__, mem);
        return;
    }*/
    Chunk *chunk = (Chunk*)((char*)mem - HEADER_SIZE);
    Chunk *next = CONTAINER(Chunk, all, chunk->all.next);
    Chunk *prev = CONTAINER(Chunk, all, chunk->all.prev);

	//fprintf(VFS_DEBUG, "%s(%p): @%p %lx [%d]\n", __extension__ __PRETTY_FUNCTION__, mem, chunk, memory_chunk_size(chunk), memory_chunk_slot(memory_chunk_size(chunk)));
    mem_used -= memory_chunk_size(chunk);

    if (next->used == 0) {
		// merge in next
		remove_free(next);
		dlist_remove(&next->all);
		//memset(next, 0xDD, sizeof(Chunk));
		mem_meta -= HEADER_SIZE;
		mem_free += HEADER_SIZE;
    }
    if (prev->used == 0) {
		// merge to prev
		remove_free(prev);
		dlist_remove(&chunk->all);
		//memset(chunk, 0xDD, sizeof(Chunk));
		push_free(prev);
		mem_meta -= HEADER_SIZE;
		mem_free += HEADER_SIZE;
    } else {
		// make chunk as free
		chunk->used = 0;
		DLIST_INIT(chunk, free);
		push_free(chunk);
    }
}
 
void check(void) {
	int	i;
    Chunk *t = last;
 
	DLIST_ITERATOR_BEGIN(first, all, it) {
		assert(CONTAINER(Chunk, all, it->all.prev) == t);
		t = it;
    } DLIST_ITERATOR_END(it);
 
    for(i = 0; i < NUM_SIZES; ++i) {
		if (free_chunk[i]) {
			t = CONTAINER(Chunk, free, free_chunk[i]->free.prev);
			DLIST_ITERATOR_BEGIN(free_chunk[i], free, it) {
			assert(CONTAINER(Chunk, free, it->free.prev) == t);
			t = it;
			} DLIST_ITERATOR_END(it);
		}
    }
}

/*
END MRVN source
*/

bool g_kmalloc_initialised;

void kmalloc_init() {
    g_kmalloc_initialised = false;
    
    void* pages = g_KPM->AllocatePages(1024); // ~4MiB
    assert(pages != nullptr);
    mrvn_memory_init(pages, PAGE_SIZE * 1024);

    g_kmalloc_initialised = true;
    NewDeleteInit();
}

extern "C" void* kcalloc(size_t num, size_t size) {
    if (!g_kmalloc_initialised)
        return nullptr;
    size = ALIGN_UP((size * num), MIN_SIZE);
    void* mem = mrvn_malloc(size);
    if (mem == nullptr)
        return nullptr;
    fast_memset(mem, 0, size / 8);
    return mem;
}

extern "C" void kfree(void* addr) {
    //dbgputs("kfree\n");
    return mrvn_free(addr);
}

extern "C" void* kmalloc(size_t size) {
    if (size == 0 || !g_kmalloc_initialised)
        return nullptr;
    return mrvn_malloc(ALIGN_UP(size, MIN_SIZE));
}

extern "C" void* krealloc(void* ptr, size_t size) {
    if (size == 0) {
        kfree(ptr);
        return nullptr;
    }
    if (ptr == nullptr)
        return kmalloc(size);
    if (ptr < g_mem_start || ptr > g_mem_end)
        return nullptr;
    size = ALIGN_UP(size, MIN_SIZE);
    size_t old_size = memory_chunk_size((Chunk*)((char*)ptr - HEADER_SIZE));
    void* ptr2 = kmalloc(size);
    if (ptr2 == nullptr)
        return nullptr;
    fast_memcpy(ptr2, ptr, old_size);
    kfree(ptr);
    return ptr2;
}


bool g_kmalloc_eternal_initialised = false;

void* g_kmalloc_eternal_mem = nullptr;
size_t g_kmalloc_eternal_free_mem = 0;
size_t g_kmalloc_eternal_used_mem = 0;

void kmalloc_eternal_init() {
    g_kmalloc_eternal_initialised = false;

    void* pages = g_KPM->AllocatePages(512); // ~2MiB
    if (pages == nullptr)
        return;
    g_kmalloc_eternal_mem = pages;
    g_kmalloc_eternal_free_mem = 512 * PAGE_SIZE;
    g_kmalloc_eternal_used_mem = 0;

    g_kmalloc_eternal_initialised = true;
}

extern "C" void* kmalloc_eternal(size_t size) {
    size = ALIGN_UP(size, 8);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    return mem;
}

extern "C" void* kcalloc_eternal(size_t num, size_t size) {
    size = ALIGN_UP(size * num, 8);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    fast_memset(mem, 0, size / 8);

    return mem;
}
