#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "header.h"
#include "grid.h"

// Implementazioni

// Importa la griglia da un file
int grid_import(cell instance[GS][GS], char * filePath)
{
    FILE * fIn;
    char giLine[MAX_LINE];
    unsigned int importInd = 0;
    unsigned int i, i2;

    fIn = fopen(filePath, "r");
    if (fIn == NULL) return 0;

    for (i = 0; i < GS; i ++)
    {
        for (i2 = 0; i2 < GS; i2 ++)
            instance[i][i2].visited = 0;
    }

    while (fgets(giLine, GS + 2, fIn) != NULL)
    {
        // Elimino il carattere di nuova linea per i test successivi
        for (i = 0; giLine[i] != '\n' && giLine[i] != '\0'; i ++) ;
        giLine[i] = '\0';

        // Ignoro le linee vuote
        if (!i) continue;

        // Verifico che le linee siano nel giusto formato
        if (i != GS)
        {
            fclose(fIn);
            return 0;
        }

        // Sto importando le lettere
        if (importInd < GS)
        {
            for (i = 0; i < GS; i ++)
                instance[importInd][i].character = tolower(giLine[i]);
        }

        // Sto importando i punti
        else if (importInd < (GS * 2))
        {
            for (i = 0; i < GS; i ++)
                instance[importInd - GS][i].points = giLine[i] - '0';
        }

        // Sto importando i bonus
        else if (importInd < (GS * 3))
        {
            for (i = 0; i < GS; i ++)
                instance[importInd - (2 * GS)][i].bonus = giLine[i] - '0';
        }

        else
        {
            // troppe linee: errore di formato
            fclose(fIn);
            return 0;
        }

        importInd ++;
    }

    fclose(fIn);

    // Verifico di non aver letto meno dati del necessario
    if (importInd != 3 * GS) return 0;

    return 1;
}
