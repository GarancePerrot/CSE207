#include <stdio.h>
#include <string.h>
#include "fancy-hello-world.h"

int main(void) {
    char name[50];  //initializes chars of size big enough for the input/output
    char output[100];  
    hello_string(name, output); //calls the function below
    return 0;
}

void hello_string(char* name, char* output){
    fgets(name, 50, stdin);   //accepts input from keyboard into char *name
    char dest[100] = "Hello World, hello ";  //initialize the first part of string
    strcat(dest, name);   //concatenates the name at the end of dest
    strcpy(output, dest);  //copy the content of dest into output
    printf("%s\n", output);  //prints the result
}