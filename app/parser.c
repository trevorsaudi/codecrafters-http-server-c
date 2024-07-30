#include <stdio.h>
#include <string.h>

int main(void) {
    const char* lineConst = "/echo/something"; // The input string
    char line[256]; // Copy of the input
    char* subString; // Result

    strcpy(line, lineConst);
    subString = strtok(line, "/"); // Find the first double quote
    subString = strtok(NULL, ""); // Find the second double quote

    char *checker = NULL;

    checker = strstr(lineConst, "/echo/");
    if(checker == lineConst)
    {
    printf("You found a match!");

}
}