//
//  misc.c
//
//  Guilherme Sividal - 09054512 
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 19/03/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include "headers/globals.h"

//FUNCOES DE ARQUIVO DA CAMADA DE ENLACE

void colocarArquivoStruct(FILE * fp) {

    size_t len = 100;
    char *linha = malloc(len);
    char *pch;
    int j, i, k = 0;
    int troca_i;
    int lendo = 0;
    int numbers[6][2];

    for (i = 0; i < MAX_NO; i++) {
        numbers[i][0] = 0;
        numbers[i][1] = 0;
    }

    while (getline(&linha, &len, fp) > 0) {
        j = 0;
        troca_i = 0;

        pch = strtok(linha, ">,:");

        if (strlen(linha) != 1) { //enter somente com o '\n'

            while (pch != NULL) {
                //Deleta Espacos
                delete_espace(pch);

                if (strcmp(pch, "[Nos]") == 0) {
#ifdef DEBBUG_ARQUIVO
                    printf("\nTabela de nós\n");
#endif
                    lendo = NOS;
                    i = 0;
                }

                if (strcmp(pch, "[Enlaces]") == 0) {
#ifdef DEBBUG_ARQUIVO
                    printf("\nTabela de enlaces\n");
#endif
                    lendo = ENLACES;
                    i = 0;
                }

                if (strcmp(pch, "[Nos]") != 0 && strcmp(pch, "[Enlaces]") != 0) {
                    if (lendo == NOS) {

                        strcpy(ligacao.nos[i][j], pch);
#ifdef DEBBUG_ARQUIVO
                        printf("nos[%d][%d] '%s' | ", i, j, ligacao.nos[i][j]);
#endif

                        //Achar MAX de 6 nós
                        if (i >= 6) {
                            printf("\nLimite de 6 nós atingidos\n");
                            exit(1);
                        }
                        //Matriz auxiliar para descobrir Maximo de 3 enlaces
                        if (j == 0) {
                            numbers[i][0] = atoi(pch);
                        }

                        troca_i++;
                    } else if (lendo == ENLACES) {

                        ligacao.enlaces[i][j] = atoi(pch);

                        //Achar + de 3 enlaces
                        if (j == 0) {
                            for (k = 0; k < MAX_NO; k++) {
                                if (numbers[k][0] == ligacao.enlaces[i][0]) {
                                    numbers[k][1] += 1;
                                    if (numbers[k][1] > 3) {
                                        printf("\nNó '%d' possue '%d' enlaces!\n", numbers[k][0], numbers[k][1]);
                                        exit(1);
                                    }
                                }
                            }
                        }

#ifdef DEBBUG_ARQUIVO
                        printf("enlace[%d][%d] '%d' | ", i, j, ligacao.enlaces[i][j]);
#endif
                        troca_i++;
                    }
                    if (troca_i == 3) {
                        i++;
#ifdef DEBBUG_ARQUIVO
                        printf("\n");
#endif
                    }
                }
                j++;
                pch = strtok(NULL, ">,:");
            }
        }
    }

    if (linha)
        free(linha);

    fclose(fp);
}

void delete_espace(char* input) {
    int i, j;
    size_t len = 100;

    char *output = malloc(len);
    strcpy(output, input);

    for (i = 0, j = 0; i < strlen(input); i++, j++) {
        if (input[i] != ' ')
            output[j] = input[i];
        else
            j--;
    }
    output[j] = '\0';

    if (output[strlen(output) - 1] == '\n')
        output[strlen(output) - 1] = '\0';

    strcpy(input, output);
}
