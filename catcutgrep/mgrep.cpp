#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH];

    // Kontrola, zda byl zadán hledaný řetězec
    if (argc < 2) {
        fprintf(stderr, "Chyba: Nebyl zadán hledaný řetězec.\n");
        return 1; // Vrátí chybový kód
    }

    // Čtení řádků ze standardního vstupu
    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        // Hledání podřetězce v řádku
        if (strstr(line, argv[1])) {
            printf("%s", line); // Vypíše řádek, pokud obsahuje hledaný řetězec
        }
    }

    return 0; // Úspěšné ukončení programu
}

