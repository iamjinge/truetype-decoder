#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

int main(int argc, char const *argv[])
{
    wchar_t *w = (wchar_t *)argv[1];
    setlocale(LC_CTYPE, "");
    printf("%lc\n", w[0]);
    return 0;
}
