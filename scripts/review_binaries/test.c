#include <stdio.h>
#include <string.h>

int foo() {
    int i, x = 0;

    // exit-2
    //exit(1);

    for (i = 0; i < 200; i++)
        x += i * 2;
    
    // exit-3
    //exit(1);

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

    const char* test = "a";
    //const char* test2 = "f";

    if (strcmp(test, "a") == 0) {

        for (int i = 0; i < 400; i++) {
            y += foo();
        }

        z = fii();

        printf("%d \n", y);
        printf("%d \n", z);
    } else {
        printf("%d \n", 0);
    }
}