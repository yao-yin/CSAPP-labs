#include <stdlib.h>
extern char** environ;
int main(int argc, char* argv[]) {
    for (int i = 0; environ[i] != NULL; i++) {
        printf("env var: %s\n", environ[i]);
    }
    return 0;
}