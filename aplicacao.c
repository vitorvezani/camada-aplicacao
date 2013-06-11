//
//  aplicacao.c
//
//  Guilherme Sividal    - 09054512
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 06/05/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include "headers/globals.h"

void *iniciarAplicacao() {

    int i;

    for (i = 0; i < MAX_PS; i++)
        ps[i] = -1;

    for (i = 0; i < MAX_IC; i++) {
        ic[i].ps = -1;
        ic[i].conectado = -1;
    }

    int te, tr;
    pthread_t threadReceberPacotes;

    //Inicia a thread receberPacotes
    tr = pthread_create(&threadReceberPacotes, NULL, receberPacotes, NULL);

    if (tr) {
        printf("ERRO: impossivel criar a thread : receberPacotes\n");
        exit(-1);
    }

    //Espera as threads terminarem
    pthread_join(threadReceberPacotes, NULL);

}

void *receberPacotes() {

    while (TRUE) {

        struct pacote pacote_rcv;

        int apsRet, icRet;
        int tam_arq;
        FILE *fp;
        struct stat st;
        int nblidos;

        char *data;

        //Trava mutex de sincronismo
        pthread_mutex_lock(&mutex_apli_trans_rcv2);

        retirarPacotesBufferApliTransRcv(&pacote_rcv);

        if (pacote_rcv.tipo == BAIXAR && strlen(pacote_rcv.buffer) >= pacote_rcv.tam_buffer) {

#ifdef DEBBUG_APLI_BAIXAR
            printf("[APLIC - RCV] Pedido para baixar o arquivo '%s'\n", pacote_rcv.buffer);
#endif

            fp = fopen(pacote_rcv.buffer, "r");
            if (!fp)
                perror("Fopen()");

            stat(pacote_rcv.buffer, &st);

            tam_arq = st.st_size;

#ifdef DEBBUG_APLI_BAIXAR
            printf("Tamanho do arquivo: '%d' bytes\n", tam_arq);
#endif

            data = (char *) malloc(sizeof (tam_arq));

            fread(data, 1, tam_arq, fp);

            apsRet = aps();

            printf("PS: '%d' criado!\n", apsRet);

            icRet = conectar(pacote_rcv.num_no, apsRet);

            printf("IC: '%d' criado!\n", icRet);

            enviar(icRet, pacote_rcv.buffer, data, tam_arq);

            fclose(fp);
            free(data);

            sleep(50);

        } else if (pacote_rcv.tipo == DADOS && strlen(pacote_rcv.buffer) >= pacote_rcv.tam_buffer) {

            FILE *fp;
            char str[256];

            printf("vou criar arquivo '%s'\n", pacote_rcv.nome_arq);

            mkdir("downloads", S_IRWXU | S_IRGRP | S_IXGRP);

            strcat(str, "downloads/");
            strcat(str, pacote_rcv.nome_arq);

            printf("Criando arquivo '%s'\n", str);

            fp = fopen(str, "w");

            fwrite(pacote_rcv.buffer, sizeof (char), pacote_rcv.tam_buffer, fp);

            //fwrite(pacote_rcv.buffer, sizeof(char), pacote_rcv.tam_buffer, stdout);

            fclose(fp);

        }

        //Destrava mutex de sinconismo
        pthread_mutex_unlock(&mutex_apli_trans_rcv1);
    }

}

int aps() {

    int i;

    for (i = 0; i < MAX_PS; ++i) {
        if (ps[i] == -1) {
            ps[i] = 1;
            return i;
        }
    }
    return -1;
}

int fps(int num_ps) {
    int i;

    for (i = 0; i < MAX_IC; i++)
        if (ic[i].conectado == 1 && ic[i].ps == num_ps)
            return -1;

    if (ps[num_ps] != -1)
        ps[num_ps] = -1;
    else
        return 0;

    return 1;
}

int conectar(int env_no, int num_ps) {

    int i;
    int flag_existe = 0;
    int flag_alocado = 0;
    int index = -1;

    struct pacote pacote_env;

#ifdef DEBBUG_APLI
    printf("[APLIC - CON]Recebi pedido para conectar no nó : '%d' & ps '%d'\n", env_no, num_ps);
#endif

    for (i = 0; i < MAX_PS; i++)
        if (ps[num_ps] == 1)
            flag_existe = 1;

    for (i = 0; i < MAX_IC; i++)
        if (ic[i].conectado == -1) {
            index = i;
            break;
        }

    for (i = 0; i < MAX_IC; i++)
        if (ic[i].ps == num_ps) {
            flag_alocado = 1;
            break;
        }

    // Se ps existe e há ic livre
    if (flag_existe == 1 && index != -1 && flag_alocado == 0) {
        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_lock(&mutex_apli_trans_env1);

        pacote_env.tipo = CONECTAR;

        ic[index].conectado = 1;
        ic[index].env_no = env_no;
        ic[index].ps = num_ps;
        ic[index].num_no = file_info.num_no;

        colocarPacotesBufferApliTransEnv(pacote_env, ic[index]);

        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_unlock(&mutex_apli_trans_env2);

        /* Consome resposta da camada de enlace */
        pthread_mutex_lock(&mutex_apli_trans_env1);

        retornoTransporte(pacote_env);

        /* Consome resposta da camada de enlace */
        pthread_mutex_unlock(&mutex_apli_trans_env1);

        ic[index].end_buffer = buffer_apli_trans_env.retorno;

        return index;

    } else {

        return -1;

    }
}

int desconectar(index) {

    struct pacote pacote_env;
    int i;

    for (i = 0; i < MAX_IC; i++)
        if (ic[index].conectado == -1)
            return 0;

#ifdef DEBBUG_APLI
    printf("[APLIC - DESC]Recebi pedido para desconectar o ic que liga env_no: '%d' e ps: '%d'\n", ic[index].env_no, ic[index].ps);
#endif

    /* Produzir buffer_rede_enlace_env */
    pthread_mutex_lock(&mutex_apli_trans_env1);

    pacote_env.tipo = DESCONECTAR;

    colocarPacotesBufferApliTransEnv(pacote_env, ic[index]);

    /* Produzir buffer_rede_enlace_env */
    pthread_mutex_unlock(&mutex_apli_trans_env2);

    /* Consome resposta da camada de enlace */
    pthread_mutex_lock(&mutex_apli_trans_env1);

    retornoTransporte(pacote_env);

    ic[index].conectado = -1;

    /* Consome resposta da camada de enlace */
    pthread_mutex_unlock(&mutex_apli_trans_env1);

    return 1;
}

int baixar(int index, char *data) {

    struct pacote pacote_env;
    int i, flag_existe = 0;

#ifdef DEBBUG_APLI
    printf("[APLIC - BAIXAR] Data: '%s'\n", data);
    printf("[APLIC - BAIXAR] Tam_buffer: '%zd'\n", strlen(data));
#endif

    for (i = 0; i < MAX_IC; i++) {
        if (ic[index].conectado == 1) {
            flag_existe = 1;

            //Produz no buffer apli_trans
            pthread_mutex_lock(&mutex_apli_trans_env1);

            pacote_env.tipo = BAIXAR;
            pacote_env.num_no = file_info.num_no;

            pacote_env.tam_buffer = strlen(data);
            strncpy(pacote_env.buffer, data, strlen(data));

            pacote_env.buffer[strlen(data)] = '\0';

            colocarPacotesBufferApliTransEnv(pacote_env, ic[index]);

            //Produz no buffer apli_trans
            pthread_mutex_unlock(&mutex_apli_trans_env2);

            break;
        }
    }
    if (flag_existe) {
        return pacote_env.tam_buffer = strlen(data);
    } else
        return -1;
}

int enviar(int index, char *nome_arq, char *data, int tam_arq) {

    struct pacote pacote_env;
    int i, flag_existe = 0;

    for (i = 0; i < MAX_IC; i++) {
        if (ic[index].conectado == 1) {
            flag_existe = 1;

            //Produz no buffer apli_trans
            pthread_mutex_lock(&mutex_apli_trans_env1);

            pacote_env.tipo = DADOS;
            pacote_env.tam_buffer = tam_arq;
            strncpy(pacote_env.nome_arq, nome_arq, MAX_TAM_NOME_ARQ);
            strncpy(pacote_env.buffer, data, tam_arq);

            colocarPacotesBufferApliTransEnv(pacote_env, ic[index]);

            //Produz no buffer apli_trans
            pthread_mutex_unlock(&mutex_apli_trans_env2);

            break;
        }
    }
    if (flag_existe) {
        return pacote_env.tam_buffer = tam_arq;
    } else
        return -1;
}

void colocarPacotesBufferApliTransEnv(struct pacote pacote, struct ic ic) {

    // Colocar no Buffer
    buffer_apli_trans_env.tam_buffer = pacote.tam_buffer;
    buffer_apli_trans_env.tipo = pacote.tipo;

    // Copiando Dados do Pacote para o Buffer
    memcpy(&buffer_apli_trans_env.ic, &ic, sizeof (ic));
    memcpy(&buffer_apli_trans_env.data, &pacote, sizeof (pacote));

}

void retirarPacotesBufferApliTransRcv(struct pacote *pacote) {

    memcpy(pacote, &buffer_apli_trans_rcv.data, sizeof (buffer_apli_trans_rcv.data));

}

void retornoTransporte() {

    // Não houve erro de malloc ou free
    if (buffer_apli_trans_env.retorno != NULL)

        if (buffer_apli_trans_env.tipo == CONECTAR) {
#ifdef DEBBUG_APLI
            printf("[APLIC - RET]Alocado o buffer -> end_buffer: '%p' \n", buffer_apli_trans_env.retorno);
            printf("Esperando syn para estabelecer conexão!\n");
#endif
        } else if (buffer_apli_trans_env.tipo == DESCONECTAR) {
#ifdef DEBBUG_APLI
            printf("[APLIC - RET]Conexão encerrada com sucesso! end_buffer: '%p' \n", buffer_apli_trans_env.retorno);
#endif
        }
}