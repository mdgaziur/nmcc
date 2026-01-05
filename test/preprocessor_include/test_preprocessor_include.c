#include "print_hello.h"
#define PI 3.14159265358979323846
#define X(a ,b) a \
    + b \
    + c

int main() {
    print_hello();
    printf("PI: %lf\n", PI);
    int c = 1;
    int z = X(1, 2);
    printf("z: %i\n", z);
}
