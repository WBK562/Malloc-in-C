# Malloc-in-C
This is my project, which was required by my university (the Polytechnic) to pass the course.
The only files written by me are heap.c, heap.h, and main.c.
All other files were either generated automatically by the IDE or come from the repository: https://github.com/tomekjaworski/SO2/tree/master/heap_sbrk-sim
# What's in it?
This project is a simple custom implementation of malloc, providing the ability to allocate memory dynamically.
It also includes implementations of calloc and realloc, as well as a custom free function.
The heap is organized as a linked list, and the allocator supports coalescing — it can merge up to three neighboring adjacent free blocks (the previous, the current, and the next one) into a single larger block.
Each memory block contains boundary markers (“fences”) and its own metadata structure, which stores information about the block’s state and size.
Several helper functions are also included to support internal heap operations and memory management.
