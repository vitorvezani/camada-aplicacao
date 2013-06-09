//
//  meuprog.c
//
//  Guilherme Sividal    - 09054512
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 07/05/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#define TRUE    1

#include "headers/globals.h"

int main(int argc, char const *argv[]) {

    int tic;
    pthread_t threadinicializarCamadas;

    char cmd[128];
    char *ptrFuncao, *ptrParam1, *ptrParam2;

    /* Testa Parametros */
    if (argc != 3) {
        printf("Use: %s 'file_name.ini' 'numero_nó'\n\n", argv[0]);
        exit(1);
    }

    /* Copia para as Variaveis */
    strcpy(file_info.file_name, argv[1]);
    file_info.num_no = atoi(argv[2]);

    printf("Nome do arquivo: '%s'\n Numero nó: '%d'\n", file_info.file_name, file_info.num_no);

    tic = pthread_create(&threadinicializarCamadas, NULL, inicializarCamadas, NULL);

    if (tic) {
        printf("ERRO: impossivel criar a thread : inicializarCamadas\n");
        exit(-1);
    }

    usleep(20000);

    printf("Funções Disponíveis:\n'aps(),abrir_ponto_serviço()'\n'fps(ps),fechar_ponto_serviço(ps)'\n'conectar(nó,ps),c(nó,ps)'\n'desconectar(ic),d(ic)'\n'baixar(ic,ar.ext),b(ic,arq.ext)'\n'clear()\n");
    printf("Digite a função e os respectivos parametros\n");

while (TRUE){

    printf("$ ");
    fgets(cmd , 127 , stdin);
    cmd[strlen(cmd)-1]='\0';

    ptrFuncao = strtok(cmd," ");

    if(strcasecmp(ptrFuncao,"aps") == 0 || strcasecmp(ptrFuncao,"abrir_ponto_serviço") == 0)
    {
        int ps;

        ps = aps();

        if (ps == -1)
            printf("Impossivel criar ps.\n");
        else
            printf("ps criado '%d'.\n", ps);

    }else if(strcasecmp(ptrFuncao,"fps") == 0 || strcasecmp(ptrFuncao,"fechar_ponto_serviço") == 0)
    {
        int fpsRet = 0;

        ptrParam1 = strtok(NULL," ");

        if (ptrParam1 != NULL)
        {
            fpsRet = fps(atoi(ptrParam1));

            if (fpsRet == 1)
                printf("ps fechado!\n");
            else if (fpsRet == 0)
                printf("Erro ao fechar ps, verifique a existencia do ps.\n");
            else if (fpsRet == -1)
                printf("Erro ao fechar ps, ps conectado por um ic, primeiro desconecte ic.\n");
        }else
            printf("Parametros invalidos, utilize fps 'ps'\n");

    }else if(strcasecmp(ptrFuncao,"conectar") == 0 || strcasecmp(ptrFuncao,"c") == 0)
    {
        int conectarRet;

        ptrParam1 = strtok(NULL," ");
        ptrParam2 = strtok(NULL," ");

        if (ptrParam1 != NULL || ptrParam2 != NULL)
        {
            conectarRet = conectar(atoi(ptrParam1), atoi(ptrParam2));

            if(conectarRet != -1)
                printf("IC conectado: '%d'\n", conectarRet);
            else
                printf("Erro ao conectar, verifique a existencia do ps ou se o ps já está alocado.\n");
        }else
            printf("Parametros invalidos, utilize conectar 'dst_nó' 'ps'\n");

    }else if(strcasecmp(ptrFuncao,"desconectar") == 0 || strcasecmp(ptrFuncao,"d") == 0)
    {
        int desconectarRet;

        ptrParam1 = strtok(NULL," ");

        if (ptrParam1 != NULL)
        {
            desconectarRet = desconectar(atoi(ptrParam1));

            if (!desconectarRet)
                printf("Erro ao desconectar, verifique a existencia do ic.\n");
            else
                printf("IC Desconectado com sucesso!\n");
        }else
            printf("Parametros invalidos, utilize desconectar 'ic'\n");

    }else if(strcasecmp(ptrFuncao,"baixar") == 0 || strcasecmp(ptrFuncao,"b") == 0)
    {
        int baixarRet;

        ptrParam1 = strtok(NULL," ");
        ptrParam2 = strtok(NULL," ");

        if (ptrParam1 != NULL || ptrParam2 != NULL)
        {
            baixarRet = baixar( atoi(ptrParam1) , ptrParam2 );

            if (baixarRet == -1)
                printf("Erro ao baixar, verifique a existencia do ic.\n");
        }else
            printf("Parametros invalidos, utilize baixar 'ic' 'nome_arq.ext'\n");

    }else if(strcasecmp(ptrFuncao,"clear") == 0)
    {
        system("clear");
    }else
        printf("Opção invalida!.\n");

}
    /* Espera a Thread terminar */
    pthread_join(threadinicializarCamadas, NULL);

    return 0;
}
