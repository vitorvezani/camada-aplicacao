// 
//  transporte.c
// 
//  Guilherme Sividal    - 09054512
//  Vitor Rodrigo Vezani - 10159861
// 
//  Created by Vitor Vezani on 07/04/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
// 

#include "headers/globals.h"

void *iniciarTransporte() {

    int tes, trs, tea, tra, tt;
    pthread_t threadEnviarSegmentos, threadReceberSegmentos, threadEnviarPacote, threadReceberPacote, threadTimer;


    buffer_trans_trans_rcv.data = (char *) malloc(sizeof (TAM_BUFFER_TRANS));

    // Inicia a thread enviarSegmentos
    tes = pthread_create(&threadEnviarSegmentos, NULL, enviarSegmentos, NULL);

    if (tes) {
        printf("ERRO: impossivel criar a thread : enviarSegmentos\n");
        exit(-1);
    }

    // Inicia a thread receberSegmentos
    trs = pthread_create(&threadReceberSegmentos, NULL, receberSegmentos, NULL);

    if (trs) {
        printf("ERRO: impossivel criar a thread : receberSegmentos\n");
        exit(-1);
    }

    // Inicia a thread enviarPacote
    tea = pthread_create(&threadEnviarPacote, NULL, enviarPacote, NULL);

    if (tea) {
        printf("ERRO: impossivel criar a thread : enviarPacote\n");
        exit(-1);
    }

    // Inicia a thread receberPacote
    tra = pthread_create(&threadReceberPacote, NULL, receberPacote, NULL);

    if (tra) {
        printf("ERRO: impossivel criar a thread : receberPacote\n");
        exit(-1);
    }

    // Espera as threads terminarem
    pthread_join(threadEnviarSegmentos, NULL);
    pthread_join(threadReceberSegmentos, NULL);
    pthread_join(threadEnviarPacote, NULL);
    pthread_join(threadReceberPacote, NULL);

}

void *enviarPacote() {

    while (TRUE) {

        struct pacote pacote_rcv;

        // Consumir buffer_trans_trans_rcv
        pthread_mutex_lock(&mutex_trans_trans_rcv2);

            retirarPacoteBufferTransTransRcv(&pacote_rcv);

        // Consumir buffer_trans_trans_rcv
        pthread_mutex_unlock(&mutex_trans_trans_rcv1);

        // Produzir buffer_apli_trans_rcv
        pthread_mutex_lock(&mutex_apli_trans_rcv1);

        colocarPacoteBufferApliTransRcv(pacote_rcv);

        // Produzir buffer_apli_trans_rcv
        pthread_mutex_unlock(&mutex_apli_trans_rcv2);

    }
}

void *receberPacote() {

    while (TRUE) {

        struct pacote pacote_env;
        struct ic ic;

        // Consumir buffer_apli_trans_env
        pthread_mutex_lock(&mutex_apli_trans_env2);

        retirarPacoteBufferApliTransEnv(&pacote_env, &ic);

        /*  Se for CONECTAR, aloca o buffer, faz o ponteiro do buffer alocar para o buffer alocado,
        // envia pacote de connect para o receptor
        */

        if (buffer_apli_trans_env.tipo == CONECTAR) {

            buffer_trans_trans_env.data = (char *) malloc(sizeof (TAM_BUFFER_TRANS));

            buffer_apli_trans_env.retorno = buffer_trans_trans_env.data;

            // Produzir buffer_trans_trans_env
            pthread_mutex_lock(&mutex_trans_trans_env1);

               colocarPacoteBufferTransTransEnv(pacote_env, ic);

            // Produzir buffer_trans_trans_env
             pthread_mutex_unlock(&mutex_trans_trans_env2);

        /* Caso seja DESCONECTAR desaloca o buffer */

        } else if (buffer_apli_trans_env.tipo == DESCONECTAR) {
            char *aux;

            // LIBERAR BUFFER

            aux = buffer_apli_trans_env.ic.end_buffer;

            free(ic.end_buffer);

            buffer_apli_trans_env.retorno = aux;

       /* Caso seja DADOS, envia pacote contendo os dados e o ic */

        } else if (buffer_apli_trans_env.tipo == DADOS) {

            // Produzir buffer_trans_trans_env
            pthread_mutex_lock(&mutex_trans_trans_env1);

                colocarPacoteBufferTransTransEnv(pacote_env, ic);

            // Produzir buffer_trans_trans_env
            pthread_mutex_unlock(&mutex_trans_trans_env2);

            //Unlock ao termino talvez?

        }

    // Consumir buffer_apli_trans_env
    pthread_mutex_unlock(&mutex_apli_trans_env1);

    }

}

    /*  Essa thread tem a função de ficar dormente por um tempo e caso ela eventualmente acorde, 
    // terá a função de desbloquear a thread enviarSegmentos() fazendo que ela reenvie toda a janela.
    */

void *timer() {

#ifdef DEBBUG_TRANSPORTE
    printf("[TRANS - TIMER] Iniciei a Thread Timer!!\n");
#endif

    sleep(1);

    pthread_mutex_unlock(&env_seg_rcv_seg_timer_2);

}

    /*  Essa thread tem como principal função enviar o segmentos do buffer interno à camada de transporte 
    // para a camada de rede. É aonde está implementado o algoritmo da janela deslizante. 
    // Recebe também alem do sinal de estouro de timer, os syn e acks da thread receberSegmentos.
    */

void *enviarSegmentos() {

    int first_pkg, tt;
    pthread_t threadTimer;

    int tipo;

    int base = 0;
    int nextseqnum = 0;
    ack = -1;
    syn = -1;

    // Consumir buffer_trans_trans_env
    pthread_mutex_lock(&mutex_trans_trans_env2);

    while (TRUE){

        // Dados a ser considerado como fim do envio
        if (base > (sizeof (buffer_trans_trans_env) + TAM_BUFFER_TRANS) - (TAM_BUFFER_TRANS - buffer_trans_trans_env.tam_buffer) ||
            syn == 1)
        {

        #ifdef DEBBUG_TRANSPORTE
        printf("[TRANS - ENV FIM DA JANELA] sizeof(buffer): '%zd', base final: '%d', nextseqnum final: '%d'\n", (sizeof (buffer_trans_trans_env) + TAM_BUFFER_TRANS) - (TAM_BUFFER_TRANS - buffer_trans_trans_env.tam_buffer), base, nextseqnum);
        #endif

        // Reinicialização das variaveis
        base = 0;
        nextseqnum = 0;
        ack = -1;
        syn = -1;

        // Consumir buffer_trans_trans_env
        pthread_mutex_lock(&mutex_trans_trans_env2);

        }

        // Dados ou Conectar?
        tipo = buffer_apli_trans_env.tipo;

        #ifdef DEBBUG_TRANSPORTE
            printf("----------------------------------------------------------------------\n");
            printf("[TRANS - ENV] Vou enviar a Janela\n");
        #endif

        // Inicia a thread timer
        tt = pthread_create(&threadTimer, NULL, timer, NULL);

        // Dispara Thread timer
        if (tt) {
            printf("ERRO: impossivel criar a thread : timer\n");
            exit(-1);
        }

        while (nextseqnum < base + TAM_JANELA) {
            
            #ifdef DEBBUG_TRANSPORTE
            printf("[TRANS - ENV] base: '%d', nextseqnum: '%d'\n", base, nextseqnum);
            #endif

            struct segmento segmento_env;

            retirarSegmentoBufferTransTransEnv(&segmento_env, nextseqnum);

            // Consumir buffer_trans_trans_env
            pthread_mutex_unlock(&mutex_trans_trans_env1);

            // Produzir buffer_trans_rede_env
            pthread_mutex_lock(&mutex_trans_rede_env1);

            // Envia os segmentos da janela

            switch(tipo){
                case CONECTAR:
                    segmento_env.tam_buffer = sizeof (segmento_env) - TAM_MAX_BUFFER; // Tamanho de segmentos - dados
                    segmento_env.flag_connect = 1;
                    segmento_env.flag_ack = -1;
                    segmento_env.flag_syn = -1;
                    segmento_env.flag_push = -1;
                    break;
                case DADOS:
                    segmento_env.flag_push = 1;
                    segmento_env.flag_ack = -1;
                    segmento_env.flag_connect = -1;
                    segmento_env.flag_syn = -1;
                    break;
                default:
                    printf("erro fatal\n");
                    exit(1);
                    break;
            }

            segmento_env.num_no = file_info.num_no;
            segmento_env.env_no = buffer_trans_trans_env.ic.env_no;

            colocarSegmentoBufferTransRedeEnv(segmento_env);

#ifdef DEBBUG_TRANSPORTE
            printf("[TRANS - ENV] Mandei um Segmento\n");
#endif

#ifdef DEBBUG_TRANSPORTE_FLAGS
            printf("[TRANS - ENV] flag_push = '%d'\n", segmento_env.flag_push);
            printf("[TRANS - ENV] flag_connect = '%d'\n", segmento_env.flag_connect);
            printf("[TRANS - ENV] env_no = '%d'\n", segmento_env.env_no);
            printf("[TRANS - ENV] tam_buffer = '%d'\n", buffer_trans_rede_env.tam_buffer);
#endif

            // Produzir buffer_trans_rede_env
            pthread_mutex_unlock(&mutex_trans_rede_env2);

            // Anda com o nextseqnum
            nextseqnum += TAM_SEGMENT;

        }

            // Recebi um sinal de ack ou de syn
            if (ack != -1 || syn == 1)
            {

                printf("[TRANS - ENV] ack: '%d', syn: '%d'\n", ack, syn);

                pthread_cancel(threadTimer);

                // Produzir buffer_trans_trans_rcv
                pthread_mutex_unlock(&env_seg_rcv_seg_timer_2);
            }

            // Produzir buffer_trans_rede_env
            pthread_mutex_lock(&env_seg_rcv_seg_timer_2);

            // Se o timer nao estourou, anda com a janela
            if (ack != -1) {

            // Anda com a janela
            base = ack;

            #ifdef DEBBUG
            printf("[TRANS - ENV] Recebi um pacote de ack: '%d'\n", ack);
            #endif

            #ifdef DEBBUG_TRANSPORTE
            printf("[TRANS - ENV] Andei com a janela: '%d'\n", base);
            #endif

            ack = -1;

        } else if (syn == 1) {

#ifdef DEBBUG
            printf("[TRANS - ENV] Recebi um pacote de syn\n");
#endif
        printf("Conexão estabelecida com sucesso!\n");

        } else {

#ifdef DEBBUG
            printf("[TRANS - ENV] Thread de timer estourou. Reenviando Janela\n");
#endif
            // Reenvia Janela
            nextseqnum = base;

        }
    }
}

void *receberSegmentos() {

    int expectedseqnum = 0;

    while (TRUE) {

        struct segmento segmento_rcv;

        // Consumir buffer_trans_rede_rcv
        pthread_mutex_lock(&mutex_trans_rede_rcv2);

        #ifdef DEBBUG_TRANSPORTE

        printf("----------------------------------------------------------------------\n");
        printf("[TRANS - RCV] Novo Segmento\n\n");

        #endif

            // Retira segmento do buffer
            retirarSegmentoBufferTransRedeRcv(&segmento_rcv);

        // Consumir buffer_trans_rede_rcv
        pthread_mutex_unlock(&mutex_trans_rede_rcv1);

        /* Se for Segmento de ACK, envia ACK */
        if (segmento_rcv.flag_ack == 1) {

            ack = segmento_rcv.ack;

        /* Se for Segmento de SYN, envia SYN */
        } else if (segmento_rcv.flag_syn == 1) {

            syn = 1;

        /* Se for Segmento de CONNECT, envia SYN ao nó de origem */
        } else if (segmento_rcv.flag_connect == 1) {

            struct segmento segmento_env_ack;

        #ifdef DEBBUG

                printf("[TRANS - RCV] Recebi um segmento de connect!\n\n");

        #endif

            segmento_env_ack.flag_ack = -1;
            segmento_env_ack.flag_push = -1;
            segmento_env_ack.flag_connect = -1;

            segmento_env_ack.flag_syn = 1;
            segmento_env_ack.num_no = segmento_rcv.env_no;
            segmento_env_ack.env_no = segmento_rcv.num_no;

            // Produzir buffer_trans_rede_env
            pthread_mutex_lock(&mutex_trans_rede_env1);

                #ifdef DEBBUG

                printf("[TRANS - RCV] Enviei um pacote de syn!\n\n");

                #endif

                colocarSegmentoBufferTransRedeEnv(segmento_env_ack);

            // Produzir buffer_trans_rede_env
            pthread_mutex_unlock(&mutex_trans_rede_env2);

        /*  Se for Segmento de Dados, e o expectedseqnum for igual ao numseq recebido, 
        // coloca o segmento no buffer interno e envia ack do prox. numseq ao nó de origem.
        */

        } else if (segmento_rcv.flag_push == 1) {

            if (segmento_rcv.seqnum == expectedseqnum) {

                struct segmento segmento_env_ack;

        #ifdef DEBBUG_TRANSPORTE

                printf("[TRANS - RCV] SeqNum('%d') recebido == ExpectedSeqNum('%d')\n\n", segmento_rcv.seqnum, expectedseqnum);

        #endif

                // Produzir buffer_trans_trans_rcv
                pthread_mutex_lock(&mutex_trans_trans_rcv1);

                    colocarSegmentoBufferTransTransRcv(segmento_rcv, segmento_rcv.seqnum);

                // Produzir buffer_trans_trans_rcv
                pthread_mutex_unlock(&mutex_trans_trans_rcv2);

                // Enviar ACK para nó destino

                expectedseqnum += TAM_SEGMENT;

                montarSegmentoAck(&segmento_env_ack, segmento_rcv, expectedseqnum);

                #ifdef DEBBUG
                    printf("[TRANS - RCV] Enviando Pacote de ACK: '%d'\n", segmento_env_ack.ack);
                #endif

                // Produzir buffer_trans_rede_env
                pthread_mutex_lock(&mutex_trans_rede_env1);

                colocarSegmentoBufferTransRedeEnv(segmento_env_ack);

                // Produzir buffer_trans_rede_env
                pthread_mutex_unlock(&mutex_trans_rede_env2);

            /*  Se for Segmento de Dados, e o expectedseqnum for diferente do numseq recebido.
            // descarta segmento recebido.
            */

            } else {

                #ifdef DEBBUG
                printf("[TRANS - RCV] SeqNum('%d') != ExpectedSeqNum('%d')\n", segmento_rcv.seqnum, expectedseqnum);
                printf("[TRANS - RCV] Pacote Descartado !\n\n");
                #endif

            }
        }
    }
}

void montarSegmentoAck(struct segmento *segment, struct segmento segment_rcv, int expectedseqnum){

        segment->flag_syn = -1;
        segment->flag_push = -1;
        segment->flag_connect = -1;

        segment->flag_ack = 1;
        segment->ack = expectedseqnum;

        segment->num_no = segment_rcv.env_no;
        segment->env_no = segment_rcv.num_no;
}

void colocarPacoteBufferApliTransRcv(struct pacote package) {

    // Colocar no Buffer
    memcpy(&buffer_apli_trans_rcv.data, &package, sizeof (package));

}

void retirarPacoteBufferApliTransEnv(struct pacote *package, struct ic *ic) {

    // Retirar do Buffer
    memcpy(ic, &buffer_apli_trans_env.ic , sizeof (buffer_apli_trans_env.ic));
    memcpy(package, &buffer_apli_trans_env.data , sizeof (buffer_apli_trans_env.data));

}

void colocarPacoteBufferTransTransEnv(struct pacote package, struct ic ic) {

    // Colocar no Buffer
    memcpy(&buffer_trans_trans_env.tam_buffer, &buffer_apli_trans_env.tam_buffer, sizeof (int));
    memcpy(&buffer_trans_trans_env.ic, &ic, sizeof (ic));
    memcpy(buffer_trans_trans_env.data, &package, sizeof (package));

}

void retirarPacoteBufferTransTransRcv(struct pacote *package) {

    // Retirar do Buffer
    memcpy(package, buffer_trans_trans_rcv.data, sizeof (*package));

    // fwrite (&buffer_trans_trans_rcv.data , 1 , TAM_SEGMENT , stdout);

}

void colocarSegmentoBufferTransTransRcv(struct segmento segment, int seqnum) {

    void *ptr;
    ptr = buffer_trans_trans_rcv.data;
    ptr += seqnum;

    // Colocar no Buffer
    memcpy(ptr, &segment.data, TAM_SEGMENT);

    // fwrite (ptr , 1 , TAM_SEGMENT , stdout );

}

void retirarSegmentoBufferTransTransEnv(struct segmento *segment, int nextseqnum) {

    void *ptr;
    ptr = buffer_trans_trans_env.data;
    ptr += nextseqnum;

    segment->seqnum = nextseqnum;
    segment->num_no = file_info.num_no;
    segment->tam_buffer = buffer_trans_trans_env.tam_buffer;
    memcpy(&(segment->ic), &buffer_trans_trans_env.ic, sizeof (buffer_trans_trans_env.ic));
    memcpy(&(segment->data), ptr, TAM_SEGMENT);

}


void colocarSegmentoBufferTransRedeEnv(struct segmento segment){

    // Colocar no Buffer
    buffer_trans_rede_env.env_no = segment.env_no;
    buffer_trans_rede_env.tam_buffer = segment.tam_buffer;
    memcpy(&buffer_trans_rede_env.data, &segment, sizeof (segment));

}

void retirarSegmentoBufferTransRedeRcv(struct segmento *segment) {

    // Retirar do Buffer
    memcpy(segment, &buffer_trans_rede_rcv.data, sizeof (buffer_trans_rede_rcv.data));

    #ifdef DEBBUG_TRANSPORTE_FLAGS

    printf("[TRANS - RCV] segmento_rcv.flag_ack: '%d'\n", segmento_rcv.flag_ack);
    printf("[TRANS - RCV] segmento_rcv.flag_syn: '%d'\n", segmento_rcv.flag_syn);
    printf("[TRANS - RCV] segmento_rcv.flag_connect: '%d'\n", segmento_rcv.flag_connect);
    printf("[TRANS - RCV] segmento_rcv.flag_push: '%d'\n\n", segmento_rcv.flag_push);
    printf("[TRANS - RCV] segmento_rcv.ack: '%d'\n\n", segmento_rcv.ack);
    printf("[TRANS - RCV] segmento_rcv.num_no: '%d'\n", segmento_rcv.num_no);
    printf("[TRANS - RCV] segmento_rcv.env_no: '%d'\n", segmento_rcv.env_no);

    #endif

}