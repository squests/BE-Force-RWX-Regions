#include <stdio.h>

void vuln() {
    char buffer[20];

    puts("Give me the input");

    gets(buffer);
}

int main() {
    vuln();

    return 0;
}
