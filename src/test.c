#include <stdio.h>

struct foo
{
    char a;
    int b;
} myfoo;

int main()
{
    int c = 1;
    int *d;
    
    d = &myfoo.a;
    *d = c;
}
