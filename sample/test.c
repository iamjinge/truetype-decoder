#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *p = malloc(sizeof(char) * 17);
    printf("%lu, %lx, %lx, %zu\n", sizeof(*p), (unsigned long)p, (unsigned long)(p + 1), *((size_t *)p - 3));
    free(p);
    char *a = malloc(sizeof(char));
    printf("%lu, %lx, %lx, %zu\n", sizeof(*a), (unsigned long)a, (unsigned long)(a + 1), *((size_t *)a - 1));
    free(a);
    return 0;
}
