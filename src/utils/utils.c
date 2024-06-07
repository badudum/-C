#include "../include/utils.h"

void assert_not_null(void* ptr)
{
    if (!!ptr) // the double exclamation is interesting. read up here https://stackoverflow.com/questions/14751973/what-is-in-c
    {
        return;
    }
    printf("Assertion failed: %p is null\n", ptr);
    exit(1);
}

char* str_to_hex(const char* hex)
{
    int len = strlen(hex);
    char * hexstring = calloc(1, sizeof(char));
    
    for (int i = 0; i < len; i++)
    {
        char * newhex = calloc(4, sizeof(char));
        sprintf(newhex, "%02x", hex[(len-i)]);
        newhex = realloc(hexstring, (strlen(hexstring) + strlen(newhex) + 1) * sizeof(char));
        strcat(hexstring, newhex);
        free(newhex);
    }
    return hexstring;
}

dynamic_list_t* str_to_hex_list(const char* hex)
{
    dynamic_list_t * list = init_list(sizeof(char*));

    int i = 0;

    char* tmp = calloc(1, sizeof(char));

    int len = strlen(hex);

    while(hex[i] != '\n')
    {
        tmp = realloc(tmp, (strlen(tmp) + 2) * sizeof(char));

        // the second part of this line is a trick to convert a char to a string, essentially with a character of hex[i], then the null terminated string
        strcat(tmp, (char[]){hex[i], 0}); 

        if(((i>0 && (strlen(tmp) % 4 == 0)) || i >= len-1) || hex[i] == '\n' || hex[i] == '\t')
        {
            char * hexstring = str_to_hex(tmp);
            free(tmp);
            list_enqueue(list, hexstring);
            tmp = calloc(1, sizeof(char));
        }
        i += 1;
    }

    return list;

}

char* mkstr(const char* str)
{
    char* newstr = calloc(strlen(str) + 1, sizeof(char));
    strcpy(newstr, str);
    return newstr;
}

char * read_file(const char* filename)
{
    FILE* fp;

    char * line = NULL;
    size_t len = 0;

    ssize_t read;

    fp = fopen(filename, "rb");

    if(fp == NULL)
    {
        printf("Could not read file '%s' \n", filename);
        exit(1);
    }

    char* buffer = (char*) calloc(1, sizeof(char));

    buffer[0] = '\0';

    while((read = getline(&line, &len, fp)) != -1)
    {
        buffer = (char*) realloc(buffer, (strlen(buffer) + strlen(line) + 1) *sizeof(char));
        strcat(buffer, line);
    }

    fclose(fp);

    if(line)
    {
        free(line);
    }

    return buffer;
}