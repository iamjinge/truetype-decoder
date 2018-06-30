#include <stdio.h>
#include <stdlib.h>

typedef struct A_
{
    int a;
    int b;
} A;

typedef struct A_ *B;

int binarySearch(int target, int *array, int length)
{
    int first, last;
    first = 0;
    last = length - 1;

    while (first != last)
    {
        int middle = (first + last) / 2;
        printf("%d, %d, %d, %d\n", first, last, array[middle], array[middle + 1]);
        if (array[middle] <= target)
            first = middle + 1;
        else
            last = middle;
    }
    printf("%d\n", first);
    if (array[first] >= target)
        return first;
    else
        return -1;
}

int main(int argc, char const *argv[])
{
    int a[] = {1, 3, 15, 16, 21};
    printf("search result %d\n", binarySearch(-1, a, 5));
    return 0;
}
