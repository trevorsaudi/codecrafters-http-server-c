#include <string.h>
#include <stdio.h>

void Tokenizer(char str[]){
    char *pch;
    char *target;
    pch = strtok(str, " ");
    target = pch = strtok(NULL, " ");
    printf("The second item in the string is: %s", target);
}


int main(){
    char str[] = "This is a sample string";
    Tokenizer(str);
    return 0;
}