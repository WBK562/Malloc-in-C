/*
* This file is part of a project licensed under the GNU GPL v3.0.
 * This file uses the function custom_srbk() from the project SO2 by tomekjaworski,
 * which is licensed under the GNU General Public License v3.0.
 */


#include "heap.h"
#include <stdio.h>
#include "custom_unistd.h"

#define FENCES 8

struct memory_manager_t memory_manager;

int check_and_set_suma_struct(struct memory_chunk_t *ptr) {
    int suma = 0;
    char *ptr_temp = (char*)ptr;
    for(size_t i = sizeof(int);i < sizeof(struct memory_chunk_t);i++){
        suma += *(ptr_temp+i);
    }
    return suma;
}

void set_zeros(char *ptr, size_t size) {
    if(ptr == NULL) {
        return;
    }
    for(size_t i = 0;i < size;i++) {
        *(ptr+i) = 0;
    }
}

void set_fences(char *ptr, size_t size) {
    for (size_t i = 0; i < FENCES; i++) {
        *(ptr+i) = '#';
        *(ptr+size+FENCES+i) = '#';
    }
}

int check_fences(const char *ptr,size_t size) {
    for (size_t i = 0; i < FENCES; i++) {
        if (*(ptr+i) != '#' || *(ptr+size+FENCES+i) != '#') {
            return 1;
        }
    }
    return 0;
}

int heap_setup(void) {
    memory_manager.memory_start = custom_sbrk(0);
    if(memory_manager.memory_start == (void*)-1) {
        return -1;
    }
    memory_manager.memory_size = 0;
    memory_manager.first_memory_chunk = NULL;
    return 0;
}

void heap_clean(void) {
    if (memory_manager.memory_start == NULL) {
        memory_manager.memory_size = 0;
        memory_manager.first_memory_chunk = NULL;
        return;
    }
    custom_sbrk((uintptr_t)memory_manager.memory_start - (uintptr_t)custom_sbrk(0));
    memory_manager.memory_start = NULL;
    memory_manager.memory_size = 0;
    memory_manager.first_memory_chunk = NULL;
}

void* heap_malloc(size_t size) {
    if(size < 1 || heap_validate() == 1) {
        return NULL;
    }
    struct memory_chunk_t *ptr = memory_manager.first_memory_chunk;

    if(ptr == NULL) {
        void *sbrk_blok = custom_sbrk(size + sizeof(struct memory_chunk_t) + FENCES * 2);
        if(sbrk_blok == (void*)-1){return NULL;}
        struct memory_chunk_t *new_block = sbrk_blok;
        if (!new_block) {
            return NULL;
        }
        new_block->free = 0;
        new_block->prev = NULL;
        new_block->next = NULL;
        new_block->size = size;
        new_block->full_block_size = size + FENCES * 2 + sizeof(struct memory_chunk_t);
        memory_manager.first_memory_chunk = new_block;
        memory_manager.memory_size += new_block->size + sizeof(struct memory_chunk_t) + FENCES * 2;
        char *ptr1 = (char*)new_block + sizeof(struct memory_chunk_t);
        new_block->sum_struct = check_and_set_suma_struct(new_block);
        set_fences(ptr1,size);
        char *adres_pocz = (char*)new_block + sizeof(struct memory_chunk_t) + FENCES;
        return adres_pocz;
    }
    struct memory_chunk_t *ptr_temp = NULL;
    while(ptr) {
        if(ptr->free == 1 && ptr->full_block_size - sizeof(struct memory_chunk_t) - FENCES * 2 >= size) {
            ptr->size = size;
            ptr->free = 0;
            char *ptr1 = (char*)ptr + sizeof(struct memory_chunk_t);
            ptr->sum_struct = check_and_set_suma_struct(ptr);
            set_fences(ptr1,size);
            char *new_block = (char*)ptr + sizeof(struct memory_chunk_t) + FENCES;
            return new_block;
        }
        ptr_temp = ptr;
        ptr = ptr->next;
    }

    void *sbrk_block = custom_sbrk(size + sizeof(struct memory_chunk_t) + FENCES * 2);
    if(sbrk_block == (void*)-1) {
        return NULL;
    }
    struct memory_chunk_t *new_block = sbrk_block;
    if (!new_block){return NULL;}
    new_block->free = 0;
    new_block->size = size;
    new_block->prev = ptr_temp;
    new_block->next = NULL;
    new_block->full_block_size = size + FENCES * 2 + sizeof(struct memory_chunk_t);
    memory_manager.memory_size += new_block->size + sizeof(struct memory_chunk_t) + FENCES * 2;
    if (ptr_temp != NULL) {
        ptr_temp->next = new_block;
        ptr_temp->sum_struct = check_and_set_suma_struct(ptr_temp);
    }
    char *ptr1 = (char*)new_block + sizeof(struct memory_chunk_t);
    new_block->sum_struct = check_and_set_suma_struct(new_block);
    set_fences(ptr1,size);
    char *adres_pocz = (char*)new_block + sizeof(struct memory_chunk_t) + FENCES;
    return adres_pocz;
}

void* heap_calloc(size_t number, size_t size) {
    if(number < 1 || size < 1 || memory_manager.memory_start == NULL || heap_validate() == 1) {
        return NULL;
    }
    size_t total_size = number * size;
    void *user_mem = heap_malloc(total_size);
    char *ptr = user_mem;
    set_zeros(ptr,total_size);
    return user_mem;
}

void* heap_realloc(void* memblock, size_t size) {
    if (memory_manager.memory_start == NULL) {
        return NULL;
    }
    if (get_pointer_type(memblock) != 6 && memblock != NULL) {
        return NULL;
    }
    if (size == 0 && memblock != NULL) {
        heap_free(memblock);
        return NULL;
    }
    if(memblock == NULL) {
        return heap_malloc(size);
    }
    if(heap_validate()) {
        return NULL;
    }

    struct memory_chunk_t *temp_old = memory_manager.first_memory_chunk;
    int found = 0;
    while (temp_old) {
        if ((char*)temp_old + sizeof(struct memory_chunk_t) + FENCES == (char*)memblock) {
            found = 1;
            break;
        }
        temp_old = temp_old->next;
    }
    if(!found) {
        return NULL;
    }

    struct memory_chunk_t *temp = (struct memory_chunk_t*)((char*)memblock - sizeof(struct memory_chunk_t) - FENCES);
    if(temp->size == size) {
        return memblock;
    }
    if(temp->size > size) {
        temp->size = size;
        char *ptr = (char*)temp + sizeof(struct memory_chunk_t);
        set_fences(ptr,temp->size);
        temp->sum_struct = check_and_set_suma_struct(temp);
        return memblock;
    }
    if(temp->size < size) {
        size_t full_rozmiar = 0;
        if (temp->next){full_rozmiar = temp->full_block_size - sizeof(struct memory_chunk_t) - FENCES * 2 + temp->next->size + FENCES * 2 + sizeof(struct memory_chunk_t);}
        if(temp->next && temp->next->free && size <= full_rozmiar) {
            int smaller_size = size - temp->full_block_size;
            int old_next_size = temp->next->size;
            int new_block_struct = 0;
            if (old_next_size - smaller_size < 0) {
                new_block_struct = !new_block_struct;
            }
            if(new_block_struct == 0) {
                struct memory_chunk_t *old_next = temp->next;
                temp->next = old_next->next;
                if (temp->next != NULL) {
                    temp->next->prev = temp;
                    temp->next->sum_struct = check_and_set_suma_struct(temp->next);
                }
            }
            else {
                struct memory_chunk_t *new_block = (struct memory_chunk_t*)((char*)temp + sizeof(struct memory_chunk_t) + FENCES * 2 + size);
                new_block->free = 1;
                new_block->next = temp->next;
                new_block->prev = temp;
                new_block->size = old_next_size - smaller_size;
                new_block->full_block_size = new_block->size + sizeof(struct memory_chunk_t) + FENCES * 2;
                temp->next = new_block;
                new_block->sum_struct = check_and_set_suma_struct(new_block);
            }
            temp->size = size;
            temp->full_block_size = size + sizeof(struct memory_chunk_t) + FENCES * 2;
            char *ptr = (char*)temp + sizeof(struct memory_chunk_t);
            set_fences(ptr,size);
            temp->sum_struct = check_and_set_suma_struct(temp);
            return memblock;
        }
        if(temp->next == NULL) {
            size_t add_size = size - temp->size;
            if(custom_sbrk(add_size) == (void*)-1) {
                return NULL;
            }
            temp->size = size;
            char *ptr = (char*)temp + sizeof(struct memory_chunk_t);
            set_fences(ptr,size);
            temp->sum_struct = check_and_set_suma_struct(temp);
            memory_manager.memory_size += add_size;
            return memblock;
        }
        void *nowy_blok = heap_malloc(size);
        if (nowy_blok == NULL) {
            return NULL;
        }
        char *ptr1 = nowy_blok;
        char *ptr2 = (char*)temp + sizeof(struct memory_chunk_t) + FENCES;
        size_t copy_size = size < temp->size ? size : temp->size;
        for (size_t i = 0;i < copy_size;i++) {
            *(ptr1+i) = *(ptr2+i);
        }
        heap_free(memblock);
        return nowy_blok;
    }
    return NULL;
}

void heap_free(void* memblock) {
    if(memblock == NULL || heap_validate()) {
        return;
    }
    struct memory_chunk_t *temp = memory_manager.first_memory_chunk;
    if(temp == NULL) {
        return;
    }
    int found = 0;

    while (temp) {
        if ((char*)temp + sizeof(struct memory_chunk_t) + FENCES == (char*)memblock) {
            found = 1;
            break;
        }
        temp = temp->next;
    }
    if(!found) {
        return;
    }

    struct memory_chunk_t *first_address;
    first_address = (struct memory_chunk_t*)((char*)memblock - sizeof(struct memory_chunk_t) - FENCES);

    first_address->free = 1;

    if (first_address->next != NULL && first_address->next->free) {
        first_address->size += sizeof(struct memory_chunk_t) + first_address->next->size + FENCES * 2;
        first_address->full_block_size = first_address->size + sizeof(struct memory_chunk_t) + FENCES * 2;
        first_address->next = first_address->next->next;
        if (first_address->next != NULL) {
            first_address->next->prev = first_address;
        }
    }

    if (first_address->prev != NULL && first_address->prev->free) {
        first_address->prev->size += sizeof(struct memory_chunk_t) + first_address->size + FENCES * 2;
        first_address->prev->full_block_size = first_address->prev->size + sizeof(struct memory_chunk_t) + FENCES * 2;
        first_address->prev->next = first_address->next;
        if (first_address->next != NULL) {
            first_address->next->prev = first_address->prev;
        }
        first_address = first_address->prev;
    }
    if (first_address->next) first_address->next->sum_struct = check_and_set_suma_struct(first_address->next);
    if (first_address->prev) first_address->prev->sum_struct = check_and_set_suma_struct(first_address->prev);
    first_address->sum_struct = check_and_set_suma_struct(first_address);
}

size_t heap_get_largest_used_block_size(void) {
    if(heap_validate() || memory_manager.first_memory_chunk == NULL) {
        return 0;
    }
    size_t max_size = 0;
    struct memory_chunk_t *temp = memory_manager.first_memory_chunk;
    while(temp) {
        if(temp->free == 0) {
            if(max_size < temp->size) {
                max_size = temp->size;
            }
        }
        temp = temp->next;
    }
    return max_size;
}

enum pointer_type_t get_pointer_type(const void* const pointer) {
    if(pointer == NULL) {
        return pointer_null;
    }
    if(heap_validate()) {
        return pointer_heap_corrupted;
    }
    struct memory_chunk_t *temp = memory_manager.memory_start;
    while(temp) {
        if(temp->free == 1) {
            if((char*)pointer >= (char*)temp + sizeof(struct memory_chunk_t)
                && (char*)pointer < (char*)temp + sizeof(struct memory_chunk_t) + temp->size + FENCES * 2) {
                return pointer_unallocated;
            }
        }
        if((char*)pointer == (char*)temp + sizeof(struct memory_chunk_t) + FENCES){return pointer_valid;}
        if((char*)pointer >= (char*)temp  && (char*)temp + sizeof(struct memory_chunk_t) > (char*)pointer) {
            return pointer_control_block;
        }

        if((char*)pointer >= (char*)temp + sizeof(struct memory_chunk_t) &&
                (char*)pointer < (char*)temp + sizeof(struct memory_chunk_t) + FENCES) {
            return pointer_inside_fences;
        }

        if((char*)pointer > (char*)temp + sizeof(struct memory_chunk_t) + FENCES &&
                (char*)pointer < (char*)temp + sizeof(struct memory_chunk_t) + temp->size + FENCES) {
            return pointer_inside_data_block;
        }

        if((char*)pointer >= (char*)temp + sizeof(struct memory_chunk_t) + temp->size &&
                (char*)pointer < (char*)temp + sizeof(struct memory_chunk_t) + temp->size + FENCES * 2) {
            return pointer_inside_fences;
        }
        temp = temp->next;
    }
    return pointer_unallocated;
}

int heap_validate(void) {
    if(memory_manager.memory_start == NULL) {
        return 2;
    }
    struct memory_chunk_t *temp = memory_manager.first_memory_chunk;
    while(temp) {
        if(temp->sum_struct != check_and_set_suma_struct(temp)) {
            return 3;
        }
        if(temp->free == 0) {
            char *ptr = (char*)temp + sizeof(struct memory_chunk_t);
            if(check_fences(ptr,temp->size)) {
                return 1;
            }
        }
        temp = temp->next;
    }
    return 0;
}


