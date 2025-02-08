#include <stdio.h>
#include <stdlib.h>

extern "C" {

// https://ckcat.github.io/2021/02/22/%E5%8A%A8%E6%80%81%E8%B0%83%E8%AF%95so/
void _init(void) {
    printf("_init\n");
}
}

__attribute__((constructor))
void test_constructor() {
    printf("test_constructor\n");
}

int main(void) 
{
    printf("hello world!\n");
    exit(0);
}