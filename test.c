#include <stdlib.h>
#include <stdio.h>
int main(int argc, char* argv[]) {
    int cnt = 0;
    printf("100\n[");
    for (int i = 0; i < 99; i ++) {
        for (int j = i+1; j < 99; j ++) {
            if (i != j) {
                cnt ++;
                printf("[%d, %d, %d], ", i, j, 1);
            }
        }
    }
    printf("]\n");
    printf("1\n");
    printf("100\n");
    printf("99\n");
    return 0;
}