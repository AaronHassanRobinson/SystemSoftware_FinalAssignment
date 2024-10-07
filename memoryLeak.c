// C program causing a memory leak

#include <stdlib.h>

int main()
{
    for(;;) {
        // Allocating memory without freeing it
        int* ptr = (int*)malloc(sizeof(int));
    }

    return 0;
}