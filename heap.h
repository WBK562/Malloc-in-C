/*
 * This file is part of a project licensed under the GNU GPL v3.0.
 * This file uses the function custom_srbk() from the project SO2 by tomekjaworski,
 * which is licensed under the GNU General Public License v3.0.
 */


#include <stdlib.h>
#ifndef HEAP_H
#define HEAP_H

enum pointer_type_t
{
    pointer_null = 0,
    pointer_heap_corrupted = 1,
    pointer_control_block = 2,
    pointer_inside_fences = 3,
    pointer_inside_data_block = 4,
    pointer_unallocated = 5,
    pointer_valid = 6
};


struct memory_manager_t {
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
};

struct memory_chunk_t {
    int sum_struct;
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    size_t full_block_size;
    int free;
};

int heap_setup(void);
void heap_clean(void);
void* heap_malloc(size_t size);
void* heap_calloc(size_t number, size_t size);
void* heap_realloc(void* memblock, size_t count);
void  heap_free(void* memblock);
size_t heap_get_largest_used_block_size(void);
enum pointer_type_t get_pointer_type(const void* const pointer);
int heap_validate(void);
void set_fences(char *ptr, size_t size);
int check_fences(const char *ptr,size_t size);
int check_and_set_suma_struct(struct memory_chunk_t *ptr);

#endif //HEAP_H
