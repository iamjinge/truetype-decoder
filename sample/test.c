#include <stdio.h>
#include <stdlib.h>

typedef struct A_
{
    int a;
    int b;
} A;

typedef struct A_ *B;

int main(int argc, char const *argv[])
{
    int a[] = {1, 2, 3, 4, 5};
    printf("%d, %d, %d\n", *a, (int)(*a + 1), (int)(&a + 1));
    return 0;
}
