#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
    FILE *file;
    int ch;

    if (argc < 2) 
    {
        fprintf(stderr, "Chyba: Nebyl zadán soubor.\n");
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) 
    {
        perror("Chyba při otevírání souboru");
        return 1;
    }

    while ((ch = fgetc(file)) != EOF) //fgetc returns int 
    {
        putchar(ch);
    }

    fclose(file);

    return 0;
}

