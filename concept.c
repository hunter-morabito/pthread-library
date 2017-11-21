#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

struct tess {
    int field;
};

static char memblock[1000];

int main(){
    struct tess* poc = (struct tess*)memblock;
    struct tess* poc2 = (struct tess*)(memblock + sizeof(struct tess));
    poc->field = 5;
    poc2->field = 10;

    int * a = (int *)(memblock + 12);
    *a = 100;
    int * b = (int *)(memblock + 16);

    printf("size: %lu\n", sizeof(struct tess));
    for (int i = 0; i < 13;i++){
        printf("array[%d]: %d\n", i, memblock[i]);
    }
    uintptr_t someadr = (uintptr_t)a;
    uintptr_t someotheradr = (uintptr_t)b;

    uintptr_t diff = someotheradr - someadr;
    //int * d = (int *)diff;
    //*d = 7;
    //printf("array[%d]: %d\n", 4, *d);
    printf("space between addresses: %" PRIuPTR "\n", diff);
}
