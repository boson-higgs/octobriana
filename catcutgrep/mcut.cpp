#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int limit, count;
    char ch;

    // Kontrola, zda byl zadán argument s počtem znaků
    if (argc < 2) {
        fprintf(stderr, "Chyba: Nebyl zadán počet znaků.\n");
        return 1; // Vrátí chybový kód
    }

    // Převedení argumentu na číslo
    limit = atoi(argv[1]);

    // Kontrola, zda je limit kladné číslo
    if (limit <= 0) {
        fprintf(stderr, "Chyba: Limit musí být kladné číslo.\n");
        return 1; // Vrátí chybový kód
    }

    // Čtení a ořezávání řádků ze standardního vstupu
    count = 0;
    while ((ch = getchar()) != EOF) {
        if (ch == '\n') {
            putchar(ch); // Vypíše nový řádek
            count = 0;   // Resetuje počítadlo pro nový řádek
        } else if (count < limit) {
            putchar(ch);
            count++;
        }
    }

    return 0; // Úspěšné ukončení programu
}

