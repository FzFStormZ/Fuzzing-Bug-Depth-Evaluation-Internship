#include <stdio.h>

int foo() {
    int i, x = 0;

    // exit-2
    //exit(1);

    for (i = 0; i < 10; i++)
        x += i * 2;
    
    // exit-3
    exit(1);

    return x;
}

int fii() {

    // exit-1
    //exit(1);

    return 1;
}



int main()
{
    int y, z = 0;

    for (int i = 0; i < 10; i++) {
        y += foo();
    }

    z = fii();

    printf("%d \n", y);
    printf("%d \n", z);
}