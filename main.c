/*
* This file is part of a project licensed under the GNU GPL v3.0.
 * This file uses the function custom_srbk() from the project SO2 by tomekjaworski,
 * which is licensed under the GNU General Public License v3.0.
 */


#include <stdio.h>
#include <string.h>
#include "heap.h"

int main(void){
    int res = heap_setup();

    if(res == -1) {
        printf("Memory not allocated from OS, returning...");
        return -1;
    }

    int *tab_int = heap_malloc(sizeof(int) * 3);
    if(tab_int == NULL) {
        printf("Memory not allocated");
        return -1;
    }

    int x = 2;
    int y = 2;
    *tab_int = x;
    *(tab_int+1) = y;
    *(tab_int+2) = *tab_int * *(tab_int+1);

    char tab_copy[20] = "This is test";
    char *some_letters = heap_malloc(sizeof(char) * 20);
    if(some_letters == NULL) {
        printf("Memory not allocated");
        return -1;
    }

    for(size_t i = 0;i < strlen(tab_copy);i++) {
        *(some_letters+i) = *(tab_copy+i);
    }
    *(some_letters+strlen(tab_copy)) = '\0';

    for(size_t i = 0;i < 3;i++) {
        printf("%d ",*(tab_int+i));
    }
    printf("\n%s\n",some_letters);

    heap_free(tab_int);
    heap_free(some_letters);
    return 0;
}





