#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

int main(int argc, char const *argv[])
{
    int* dir=(int*)malloc(sizeof(int));
    dir[0]=0;
    printf("%i",dir[0]);
    free(dir);
    return 0;
}
