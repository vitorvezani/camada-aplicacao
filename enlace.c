//
//  enlace.c
//
//  Guilherme Sividal - 09054512
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 19/03/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include "headers/globals.h"

void *iniciarEnlace() {

    int te, tr;
    int result;
    int i, j;

    pthread_t threadEnviarFrames, threadReceberFrames;

    FILE * fp;
    fp = fopen(file_info.file_name, "r");

    if (!fp) {
        perror("Fopen()");
        exit(1);
    }

    /*Funcão que coloca o arquivo na estrutura ligacao */
    colocarArquivoStruct(fp);

    /*Inicia a thread enviarFrames */
    te = pthread_create(&threadEnviarFrames, NULL, enviarFrames, NULL);

    if (te) {
        printf("ERRO: impossivel criar a thread : Enviar Pacote\n");
        exit(-1);
    }

    /*Inicia a thread ReceberFrames */
    tr = pthread_create(&threadReceberFrames, NULL, receberFrames, NULL);

    if (tr) {
        printf("ERRO: impossivel criar a thread : Receber Pacote\n");
        exit(-1);
    }

    /*Espera as threads terminarem */
    pthread_join(threadEnviarFrames, NULL);
    pthread_join(threadReceberFrames, NULL);
}

void *enviarFrames() {

    int i, j;



#ifdef DEBBUG_ENLACE
    printf("\n");
#endif

    while (1) {

        struct frame frame_env;
        struct sockaddr_in to;
        /*Variavel para setar no fim das iteracoes a variavel buffer_rede_enlace_env.retorno */
        int flag = 0;
        int atoi_result = -1;
        int s, mtu;

        /*trava mutex de sincronismo */
        pthread_mutex_lock(&mutex_rede_enlace_env2);

        /*Loop no ligacao enlaces */
        for (i = 0; i < 18; i++) {
            /*Verificar se existe ligacao entre seu nó e o nó destino */
            if ((ligacao.enlaces[i][0] == file_info.num_no) && (buffer_rede_enlace_env.env_no == ligacao.enlaces[i][1]) ||
                    (ligacao.enlaces[i][1] == file_info.num_no) && (buffer_rede_enlace_env.env_no == ligacao.enlaces[i][0])) {

#ifdef DEBBUG_ENLACE
                printf("[ENLACE] Existe Ligacao nos [Enlaces]\n");
#endif
                /*seta MTU do enlace encontrado */
                mtu = ligacao.enlaces[i][2];

                /*Loop nos nós para encontrar IP e porta do nó destino */
                for (i = 0; i < 6; i++) {
                    atoi_result = atoi(ligacao.nos[i][0]);

                    /*Verificar o IP e Porta do nó destino */
                    if (atoi_result == buffer_rede_enlace_env.env_no) {
                        /*Cria o socket */
                        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                            perror("socket()");
                            exit(1);
                        }

#ifdef DEBBUG_ENLACE
                        printf("[ENLACE] Existe -> [Nó]: '%d',possui IP: '%s' , Porta: '%d'\n", buffer_rede_enlace_env.env_no, ligacao.nos[i][1], atoi(ligacao.nos[i][2]));
#endif

                        /*seta o IP e Porta do nó destino no sockaddr_in 'to' */

                        to.sin_family = AF_INET;
                        to.sin_port = htons(atoi(ligacao.nos[i][2])); /* Porta do nó */
                        to.sin_addr.s_addr = inet_addr(ligacao.nos[i][1]); /* Endereço IP do nó */

#ifdef DEBBUG_ENLACE
                        printf("[ENLACE] Nó Configurado\n");
#endif

                        /*Função que monta o frame */
                        colocarBufferFrame(&frame_env);

#ifdef DEBBUG_ENLACE
                        printf("[ENLACE] Frame Montado! tam_buffer: '%d', tam_data: '%lu', tam_frame: '%lu'\n", frame_env.tam_buffer,
                                sizeof (frame_env.data), sizeof (frame_env));
#endif

                        /*Testa tam dos dados vs MTU do nó */
                        if (frame_env.tam_buffer > mtu) {
                            #ifdef DEBBUG
                            printf("[ENLACE] ERRO de MTU Tamanho: '%d', MTU: '%d'\n", frame_env.tam_buffer, mtu);
                            #endif
                            buffer_rede_enlace_env.retorno = mtu;
                            flag = 2;
                            break;
                        }

#ifdef DEBBUG_ENLACE
                        printf("[ENLACE] frame_env.tam_buffer: '%d', MTU: '%d'\n", frame_env.tam_buffer, ligacao.enlaces[i][2]);
#endif

                        /*Funcão do calculo de checksum */
                        frame_env.ecc = checkSum(frame_env.data);

#ifdef DEBBUG_ENLACE
                        printf("[ENLACE] ECC Calculado! ecc: '%d'\n", frame_env.ecc);
#endif

                        /*Setar as variaveis L,C,D do garbler */
                        set_garbler(3, 3, 3);

                        /*Funcão que envia para o nó destino o Frame */
                        if (sendto_garbled(s, &frame_env, sizeof (frame_env), 0, (struct sockaddr *) &to, sizeof (to)) < 0) {
                            printf("[ENLACE] Dados não enviados!\n");
                            perror("sendto()");
                        } else {
                            #ifdef DEBBUG
                            printf("[ENLACE] Dados enviados!\n");
                            #endif
                            flag = 1;
                            break;
                        }

                    }
                }
            }
        }

        /*Teste da variavel flag para setar a variavel buffer_rede_enlace_env.retorno*/

        if (flag == 0) {
            buffer_rede_enlace_env.retorno = -1;
        } else if (flag == 1) {
            buffer_rede_enlace_env.retorno = 0;
        }

        /*Libera mutex para sincronismo */
        pthread_mutex_unlock(&mutex_rede_enlace_env1);
    }
}

void *receberFrames() {

    int i, atoi_result, s, from_address_size;
    struct sockaddr_in from, server;

    /*Cria socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }

    /*Loop nos nós para achar o IP e Porta de seu nó */

    for (i = 0; i < 6; i++) {

        /*Transforma char para int */
        atoi_result = atoi(ligacao.nos[i][0]);

        /*Achei o nó? */
        if (atoi_result == file_info.num_no) {

            /* seta o IP e Porta de seu nó no sockaddr_in 'server' */

            server.sin_family = AF_INET;
            server.sin_port = htons(atoi(ligacao.nos[i][2])); /* Porta do servidor */
            server.sin_addr.s_addr = inet_addr(ligacao.nos[i][1]); /* Endereço IP do servidor */
        }
    }

    /* Bind no IP e Porta */
    if (bind(s, (struct sockaddr *) &server, sizeof (server)) < 0) {
        perror("bind()");
        exit(1);
    }

#ifdef DEBBUG_ENLACE
    printf("\nBind no IP: %s e Porta %d\n\n",inet_ntoa(server.sin_addr),ntohs(server.sin_port));
#endif

    while (TRUE) {

        struct frame frame_rcv;
        int sum = 0;

        /* Trava o mutex de sincronismo */
        pthread_mutex_lock(&mutex_rede_enlace_rcv1);

        /* Fica esperando receber Frames */
        from_address_size = sizeof (from);
        if (recvfrom(s, &frame_rcv, sizeof (frame_rcv), 0, (struct sockaddr *) &from, &from_address_size) < 0) {
            perror("recvfrom()");
            exit(1);
        }

#ifdef DEBBUG_ENLACE
        printf("\n[ENLACE - SERVER] Frame Recebido! tam_buffer: '%d', ecc: '%d', tam_datagrama: '%lu', tam_frame: '%lu'\n", frame_rcv.tam_buffer,
                frame_rcv.ecc, sizeof (frame_rcv.data), sizeof (frame_rcv));
#endif

        /* Coloar Frame recebido no Buffer_rede_enlace_rcv */
        colocarDatagramaBuffer(frame_rcv);

#ifdef DEBBUG_ENLACE
        printf("[ENLACE - SERVER] Datagrama Montado!\n");
#endif

        /* Recalculo do ECC do Datagrama montado */
        sum = checkSum(frame_rcv.data);

#ifdef DEBBUG_ENLACE
        printf("[ENLACE - SERVER] ECC Recalculado -> frame_rcv.ECC:'%d', ECC recalculado: '%d'\n", frame_rcv.ecc, sum);
#endif

        /* Teste do ECC e retorno para cadamada de rede */
        if (frame_rcv.ecc == sum)
            buffer_rede_enlace_rcv.retorno = 0;
        else
            buffer_rede_enlace_rcv.retorno = -1;

        /* Libera mutex de sincronismo */
        pthread_mutex_unlock(&mutex_rede_enlace_rcv2);

    }
}

void colocarDatagramaBuffer(struct frame frame) {

    /* Monta o buffer */
    memcpy(&buffer_rede_enlace_rcv.data, &frame.data, sizeof (frame.data));

    buffer_rede_enlace_rcv.tam_buffer = frame.tam_buffer;

}

void colocarBufferFrame(struct frame *frame) {

    /* Monta o Frame */

    frame->ecc = 0;

    frame->tam_buffer = buffer_rede_enlace_env.tam_buffer;

    memcpy(&frame->data, &buffer_rede_enlace_env.data, sizeof (buffer_rede_enlace_env.data));

    buffer_rede_enlace_env.env_no = -1;
    buffer_rede_enlace_env.retorno = 0;

}

/* Função de CheckSum */
int checkSum(struct datagrama datagram) {

    int sum = 0;
    int aux = 0;
    int i;
    void *ptr = &datagram;

    /* Percorre o datagrama com um ponteiro de void, somando em inteiros */
    for (i = 0; i < sizeof (datagram); i += sizeof (int)) {
        memcpy(&aux, ptr, sizeof (int));
        ptr += sizeof (int);
        sum += aux;
    }
    /* Retorna a soma total */
    return sum;
}