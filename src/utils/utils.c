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
    unsigned int len = strlen(hex);
    char * hexstring = calloc(1, sizeof(char));

    for (unsigned int i = 0; i < len+1; i++)
    {
        char * newhex = calloc(4, sizeof(char));
        sprintf(newhex, "%x", hex[(len-i)]);
        hexstring = realloc(hexstring, (strlen(hexstring) + strlen(newhex) + 1) * sizeof(char));
        strcat(hexstring, newhex);
        free(newhex);
    }
    return hexstring;
}


dynamic_list_t* str_to_hex_list(const char* hex)
{
    dynamic_list_t * list = init_list(sizeof(char*));

    unsigned int i = 0;

    char* tmp = calloc(1, sizeof(char));

    unsigned int len = strlen(hex);

    while(hex[i] != '\0')
    {
        tmp = realloc(tmp, (strlen(tmp) + 2) * sizeof(char));

        // the second part of this line is a trick to convert a char to a string, essentially with a character of hex[i], then the null terminated string
        strcat(tmp, (char[]){hex[i], 0}); 

        if(((i>0 && (strlen(tmp) % 8 == 0)) || i >= len-1) || hex[i] == '\n' || hex[i] == '\t')
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
    char* newstr = (char*)calloc(strlen(str) + 1, sizeof(char));
    strcpy(newstr, str);
    return newstr;
}

char * read_file(const char* filename)
{
    FILE* fp;

    char * line = NULL;
    size_t len = 0;

    size_t read;

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

void write_file(const char * filename, const char * output_buffer)
{
    FILE* fp = fopen(filename, "w");

    if(fp == NULL)
    {
        printf("Could not write to file '%s' \n", filename);
        exit(1);
    }

    fputs(output_buffer, fp);
    fclose(fp);
}

char * command(const char* cmd)
{
    char * output = (char*) calloc(1, sizeof(char));
    output[0] = '\0';

    FILE* fp = popen(cmd, "r");
    char path[1035];

    if(fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }

    while(fgets(path, sizeof(path), fp) != NULL)
    {
        output = (char*) realloc(output, (strlen(output) + strlen(path) + 1) * sizeof(char));
        strcat(output, path);
    }

    pclose(fp);
    return output;
}



int compare (const void * a, const void * b) {
   return ( *(char*)a - *(char*)b );
}

int is_anagram(const char* str1, const char* str2)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if(len1 != len2)
    {
        return 0;
    }

    char* sorted1 = calloc(len1 + 1, sizeof(char));
    char* sorted2 = calloc(len2 + 1, sizeof(char));

    strcpy(sorted1, str1);
    strcpy(sorted2, str2);

    qsort(sorted1, len1, sizeof(char), compare);
    qsort(sorted2, len2, sizeof(char), compare);

    if(strcmp(sorted1, sorted2) == 0)
    {
        return 1;
    }

    return 0;
}

char * str_format(char* instr)
{
    char* newstr = calloc(1, sizeof(char));
    unsigned int len = strlen(instr);
    unsigned int i = 0;

    while(instr[i] != '\0' && i < len)
    {
        newstr = realloc(newstr, (strlen(newstr) + 2) * sizeof(char));

        if(instr[i] == '\\')
        {
            char escape = str_to_escape((char[]) {instr[i], instr[MIN(i+1, len)], 0});

            strcat(newstr , (char[]) {escape, 0});

            i += 2;
        }
        else 
        {
            strcat(newstr, (char[]) {instr[i], 0});
            i += 1;
        }

    }
    return newstr;
}

char str_to_escape(const char* instr)
{
    if ( strlen(instr) <= 1) return 0;
    if (instr[0] != '\\') return 0;
    char in_c = instr[1];

    switch (in_c)
    {
        case 'n' : return '\n';
        case 't' : return '\t';
        case 'r' : return '\r';
        case '\\': return in_c;
        default : return in_c;
    }
    return in_c;
}