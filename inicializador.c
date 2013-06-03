//
//  inicializador.c
//
//  Guilherme Sividal    - 09054512
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 25/03/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include "headers/globals.h"

void *inicializarCamadas() {

    int i, j;
    int te, tr, ttr, tap;
    pthread_t threadIniciaEnlace, threadIniciaRede, threadIniciaTransporte, threadIniciaAplicacao;

    /* Inicializar Mutex Sincronismo Rede->Enlace Enviar */
    pthread_mutex_init(&mutex_rede_enlace_env1, NULL);
    pthread_mutex_init(&mutex_rede_enlace_env2, NULL);
    /* Inicializar Mutex Sincronismo Rede->Enlace Receber */
    pthread_mutex_init(&mutex_rede_enlace_rcv1, NULL);
    pthread_mutex_init(&mutex_rede_enlace_rcv2, NULL);
    /* Inicializar Mutex Sincronismo Rede->Rede Enviar */
    pthread_mutex_init(&mutex_rede_rede_env1, NULL);
    pthread_mutex_init(&mutex_rede_rede_env2, NULL);
    /* Inicializar Mutex Sincronismo Rede->Rede Receber */
    pthread_mutex_init(&mutex_rede_rede_rcv1, NULL);
    pthread_mutex_init(&mutex_rede_rede_rcv2, NULL);
    /* Inicializar Mutex Sincronismo Recebe Rotas */
    pthread_mutex_init(&mutex_rede_rede_receberotas2, NULL);
    /* Inicializar Mutex Sincronismo Rede->Rede Atualizei */
    pthread_mutex_init(&mutex_rede_rede_atualizei1, NULL);
    pthread_mutex_init(&mutex_rede_rede_atualizei2, NULL);
    /* Inicializar Mutex Sincronismo Trans->Rede Enviar */
    pthread_mutex_init(&mutex_trans_rede_env1, NULL);
    pthread_mutex_init(&mutex_trans_rede_env2, NULL);
    /* Inicializar Mutex Sincronismo Trans->Rede Receber */
    pthread_mutex_init(&mutex_trans_rede_rcv1, NULL);
    pthread_mutex_init(&mutex_trans_rede_rcv2, NULL);
    /* Inicializar Mutex Sincronismo Trans->Trans Enviar */
    pthread_mutex_init(&mutex_trans_trans_env1, NULL);
    pthread_mutex_init(&mutex_trans_trans_env2, NULL);
    /* Inicializar Mutex Sincronismo Trans->Trans Receber */
    pthread_mutex_init(&mutex_trans_trans_rcv1, NULL);
    pthread_mutex_init(&mutex_trans_trans_rcv2, NULL);
    pthread_mutex_init(&mutex_trans_acess_exc_timer, NULL);
    /* Inicializar Mutex Sincronismo Apli_Trans Enviar */
    pthread_mutex_init(&mutex_apli_trans_env1, NULL);
    pthread_mutex_init(&mutex_apli_trans_env2, NULL);
    /* Inicializar Mutex Sincronismo Apli_Trans Receber */
    pthread_mutex_init(&mutex_apli_trans_rcv1, NULL);
    pthread_mutex_init(&mutex_apli_trans_rcv2, NULL);
    /* Inicializar Mutex Sincronismo Apli_Trans Receber */  
    pthread_mutex_init(&env_seg_rcv_seg_timer_2, NULL);

    /* Inicialização de Mutex Consumidores */
    pthread_mutex_lock(&mutex_rede_rede_receberotas2);
    pthread_mutex_lock(&mutex_rede_rede_atualizei2);
    pthread_mutex_lock(&mutex_rede_enlace_rcv2);
    pthread_mutex_lock(&mutex_rede_enlace_env2);
    pthread_mutex_lock(&mutex_rede_rede_rcv2);
    pthread_mutex_lock(&mutex_rede_rede_env2);
    pthread_mutex_lock(&mutex_trans_rede_env2);
    pthread_mutex_lock(&mutex_trans_rede_rcv2);
    pthread_mutex_lock(&mutex_trans_trans_env2);
    pthread_mutex_lock(&mutex_trans_trans_rcv2);
    pthread_mutex_lock(&mutex_apli_trans_env2);
    pthread_mutex_lock(&mutex_apli_trans_rcv2);
    pthread_mutex_lock(&env_seg_rcv_seg_timer_2);

    /* Inicializacao das estrutura ligacao */
    for (i = 0; i < 18; ++i)
        for (j = 0; j < 3; ++j)
            ligacao.enlaces[i][j] = -1;

    for (i = 0; i < 6; ++i)
        for (j = 0; j < 3; ++j)
            strcpy(ligacao.nos[i][j], "-1");

    // Inicialização de variaveis de controle
    flag_id = 0;                                // Inicializa ID em 1
    flag_iniciei = 1;                           // Enviar tabela de rotas à vizinhos
    flag_saida = 0;                             //Qual nó enviar

    /* Inicia a thread iniciarEnlace */
    te = pthread_create(&threadIniciaEnlace, NULL, iniciarEnlace, NULL);

    if (te) {
        printf("ERRO: impossivel criar a thread : iniciarEnlace\n");
        exit(-1);
    }

    usleep(4000);

    /* Inicia a thread iniciarRede */
    tr = pthread_create(&threadIniciaRede, NULL, iniciarRede, NULL);

    if (tr) {
        printf("ERRO: impossivel criar a thread : iniciarRede\n");
        exit(-1);
    }

    usleep(4000);

    /* Inicia a thread iniciarTransporte */
    ttr = pthread_create(&threadIniciaTransporte, NULL, iniciarTransporte, NULL);

    if (ttr) {
        printf("ERRO: impossivel criar a thread : iniciarTransporte\n");
        exit(-1);
    }

    usleep(4000);

    /* Inicia a thread iniciarAplicacao */
    tap = pthread_create(&threadIniciaAplicacao, NULL, iniciarAplicacao, NULL);

    if (tap) {
        printf("ERRO: impossivel criar a thread : iniciarAplicacao\n");
        exit(-1);
    }

    usleep(4000);

    /* Espera as threads terminarem */
    pthread_join(threadIniciaEnlace, NULL);
    pthread_join(threadIniciaRede, NULL);
    pthread_join(threadIniciaTransporte, NULL);
    pthread_join(threadIniciaAplicacao, NULL);

    /* Destroi Mutex */
    pthread_mutex_destroy(&mutex_rede_enlace_env1);
    pthread_mutex_destroy(&mutex_rede_enlace_env2);
    pthread_mutex_destroy(&mutex_rede_enlace_rcv1);
    pthread_mutex_destroy(&mutex_rede_enlace_rcv2);
    pthread_mutex_destroy(&mutex_rede_rede_atualizei1);
    pthread_mutex_destroy(&mutex_rede_rede_atualizei2);
    pthread_mutex_destroy(&mutex_rede_rede_env1);
    pthread_mutex_destroy(&mutex_rede_rede_env2);
    pthread_mutex_destroy(&mutex_rede_rede_receberotas2);
    pthread_mutex_destroy(&mutex_rede_rede_rcv1);
    pthread_mutex_destroy(&mutex_rede_rede_rcv2);
    pthread_mutex_destroy(&mutex_trans_trans_env1);
    pthread_mutex_destroy(&mutex_trans_trans_env2);
    pthread_mutex_destroy(&mutex_trans_trans_rcv1);
    pthread_mutex_destroy(&mutex_trans_trans_rcv2);
    pthread_mutex_destroy(&mutex_trans_rede_env1);
    pthread_mutex_destroy(&mutex_trans_rede_env2);
    pthread_mutex_destroy(&mutex_trans_rede_rcv1);
    pthread_mutex_destroy(&mutex_trans_rede_rcv2);
    pthread_mutex_destroy(&mutex_trans_rede_env1);
    pthread_mutex_destroy(&mutex_trans_rede_env2);
    pthread_mutex_destroy(&mutex_trans_rede_rcv1);
    pthread_mutex_destroy(&mutex_trans_rede_rcv2);
    pthread_mutex_destroy(&env_seg_rcv_seg_timer_2);

    return 0;
}
