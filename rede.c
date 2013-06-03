//
//  rede.c
//
//  Guilherme Sividal    - 09054512
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 19/03/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include "headers/globals.h"

void *iniciarRede() {

    int te, tr, tetr, trtr, ted, tes, i;
    pthread_t threadReceberSegmento, threadReceberDatagramas, threadEnviarTabelaRotas, threadReceberTabelaRotas, threadEnviarDatagrama, threadEnviarSegmento;

    /* Inicializar Mutex Buffer_Rede de acesso exclusivo*/
    pthread_mutex_init(&mutex_buffer_rede_env, NULL);
    pthread_mutex_init(&mutex_buffer_rede_rcv, NULL);

    /* inicializacao do buffer */
    for (i = 0; i < MAX_BUFFERS_DESFRAG; i++)
        buffers_fragmentacao[i].id = -1;

    /* Inicializa topologia da rede*/
    montarTabelaRotasInicial();

    /* Inicia a thread threadReceberSegmento */
    te = pthread_create(&threadReceberSegmento, NULL, receberSegmento, NULL);

    if (te) {
        printf("ERRO: impossivel criar a thread : receberSegmento\n");
        exit(-1);
    }

    /* Inicia a thread threadReceberDatagramas */
    tr = pthread_create(&threadReceberDatagramas, NULL, receberDatagramas, NULL);

    if (tr) {
        printf("ERRO: impossivel criar a thread : receberDatagramas\n");
        exit(-1);
    }

    /* Inicia a thread threadReceberTabelaRotas */
    trtr = pthread_create(&threadReceberTabelaRotas, NULL, receberTabelaRotas, NULL);

    if (trtr) {
        printf("ERRO: impossivel criar a thread : receberTabelaRotas\n");
        exit(-1);
    }

    /* Inicia a thread threadEnviarDatagrama */
    tes = pthread_create(&threadEnviarSegmento, NULL, enviarSegmento, NULL);

    if (tes) {
        printf("ERRO: impossivel criar a thread : enviarSegmento\n");
        exit(-1);
    }

    /* Inicia a thread threadEnviarDatagrama */
    ted = pthread_create(&threadEnviarDatagrama, NULL, enviarDatagrama, NULL);

    if (ted) {
        printf("ERRO: impossivel criar a thread : enviarDatagrama\n");
        exit(-1);
    }

    /* Inicia a thread EnviarTabelaRotas */
    tetr = pthread_create(&threadEnviarTabelaRotas, NULL, enviarTabelaRotas, NULL);

    if (tetr) {
        printf("ERRO: impossivel criar a thread : enviarTabelaRotas\n");
        exit(-1);
    }

    /* Destroi Mutex */
    pthread_mutex_destroy(&mutex_buffer_rede_env);
    pthread_mutex_destroy(&mutex_buffer_rede_rcv);

    /* Espera as threads terminarem */
    pthread_join(threadEnviarTabelaRotas , NULL);
    pthread_join(threadReceberTabelaRotas, NULL);
    pthread_join(threadReceberSegmento   , NULL);
    pthread_join(threadReceberDatagramas , NULL);
    pthread_join(threadEnviarDatagrama   , NULL);
    pthread_join(threadEnviarSegmento    , NULL);
}

void *receberSegmento() {

    while (1) {

        struct datagrama datagrama_env;

        /* Consumir buffer_trans_rede_env */
        pthread_mutex_lock(&mutex_trans_rede_env2);

        retirarSegmentoBufferTransRedeEnv(&datagrama_env);

        /* Consumir buffer_trans_rede_env */
        pthread_mutex_unlock(&mutex_trans_rede_env1);

        /* Produzir buffer_rede_rede_env */
        pthread_mutex_lock(&mutex_rede_rede_env1);

        colocarDatagramaBufferRedeRedeEnv(datagrama_env);

        /* Produzir buffer_rede_rede_env */
        pthread_mutex_unlock(&mutex_rede_rede_env2);
    }
}

void *enviarDatagrama() {

    int ret = 0;

    while (TRUE) {

        struct datagrama datagrama_env;

        /* Consumir buffer_rede_rede_env */
        pthread_mutex_lock(&mutex_rede_rede_env2);

        retirarDatagramaBufferRedeRedeEnv(&datagrama_env);

        /* Consumir buffer_rede_rede_env */
        pthread_mutex_unlock(&mutex_rede_rede_env1);

        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_lock(&mutex_rede_enlace_env1);

        colocarDatagramaBufferRedeEnlaceEnv(datagrama_env);

        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_unlock(&mutex_rede_enlace_env2);

        /* Consome resposta da camada de enlace */
        pthread_mutex_lock(&mutex_rede_enlace_env1);

        ret = retornoEnlace(datagrama_env);

        /* Consome resposta da camada de enlace */
        pthread_mutex_unlock(&mutex_rede_enlace_env1);

        /* É necessario fragmentar o datagrama? */
        if (ret > 0){
            fragmentarDatagramaEnv(datagrama_env);
        /* É um datagrama IP? */
        }else if (datagrama_env.type == 1) {
            /* Não é nó vizinho? */
            if (ret < 0) {
                enviarDatagramaNoNaoV(datagrama_env);
            }
        }
    }
}

void enviarDatagramaNoNaoV(struct datagrama datagram) {

    int i, MTU = 0;

    /* Procura por saida para nó não vizinho*/
    for (i = 0; i < 6; i++) {
        if (tabela_rotas[i].destino == datagram.env_no && tabela_rotas[i].saida != 0) {

            /* Produzir buffer_trans_rede_env */
            pthread_mutex_lock(&mutex_rede_enlace_env1);

            /* Coloca Datagrama no buffer_trans_rede_env */
            colocarDatagramaBufferRedeEnlaceEnv(datagram);

            /* Reescrever o env_no */
            buffer_rede_enlace_env.env_no = tabela_rotas[i].saida;
            flag_saida = tabela_rotas[i].saida;


#ifdef DEBBUG_ROTEAMENTO

            printf("[REDE - ROT]Após analizar tabela de rotas sei que para enviar datagrama para o '%d' preciso enviar para o nó  '%d'\n",datagram.env_no, 
                tabela_rotas[i].saida);

#endif
            /* Produzir buffer_trans_rede_env */
            pthread_mutex_unlock(&mutex_rede_enlace_env2);

            /* Consome resposta da camada de enlace */
            pthread_mutex_lock(&mutex_rede_enlace_env1);

            /* Testa retorno */
            MTU = retornoEnlace(datagram);

            /* Consome resposta da camada de enlace */
            pthread_mutex_unlock(&mutex_rede_enlace_env1);

            /* Se precisa fragmentar */
            if (MTU > 0)
                fragmentarDatagramaEnv(datagram);

            flag_saida = 0;

            break;
        }
    }
}

void *receberDatagramas() {

    int i = 0;
    int index;

    while (TRUE) {

        struct datagrama datagrama_rcv;

        /* Consumir buffer_rede_enlace_rcv */
        pthread_mutex_lock(&mutex_rede_enlace_rcv2);

        /* Retira datagrama do buffer */
        retirarDatagramaBufferRedeEnlaceRcv(&datagrama_rcv);

        /* Consumir buffer_rede_enlace_rcv */
        pthread_mutex_unlock(&mutex_rede_enlace_rcv1);

        /* Datagrama é para mim? */
        if (datagrama_rcv.env_no == file_info.num_no) {

            /* É um datagrama IP? */
            if (datagrama_rcv.type == 1) {

                /* É necessario desfragmentar o datagrama? */
                if (datagrama_rcv.mf >= 0)
                    desfragmentarDatagramaRcv(datagrama_rcv, &index);

                if (datagrama_rcv.mf == -1 || datagrama_rcv.mf == 0) {
                    /* Produzir buffer_rede_rede_rcv */
                    pthread_mutex_lock(&mutex_rede_rede_rcv1);

                    /* É um datagrama unico? */
                    if (datagrama_rcv.mf == -1) {

                        colocarDatagramaBufferRedeRedeRcv(datagrama_rcv);

                        /* Foi o ultimo datagrama a ser framentado? */
                    } else if (datagrama_rcv.mf == 0) {

                        colocarDatagramaBufferRedeRedeRcv(buffers_fragmentacao[index]);
                        resetarBuffer(&buffers_fragmentacao[index]);
                    }

                    /* Produzir buffer_rede_rede_rcv */
                    pthread_mutex_unlock(&mutex_rede_rede_rcv2);
                }

                /* É um datagrama de roteamento? */
            } else if (datagrama_rcv.type == 2) {

                /* Produzir buffer_rede_rede_rcv */
                pthread_mutex_lock(&mutex_rede_rede_rcv1);

                colocarDatagramaBufferRedeRedeRcv(datagrama_rcv);

                /* Produzir buffer_rede_rede_rcv */
                pthread_mutex_unlock(&mutex_rede_rede_receberotas2);

            }
        }// Datagrama não é para mim, repassar datagrama
        else {

            /* Produzir buffer_rede_rede_env */
            pthread_mutex_lock(&mutex_rede_rede_env1);

#ifdef DEBBUG
            printf("[REDE - RCV]Repassando datagrama nó dts: '%d', nó inicial: '%d'!\n", datagrama_rcv.env_no, datagrama_rcv.num_no);
#endif
            colocarDatagramaBufferRedeRedeEnv(datagrama_rcv);

            /* Produzir buffer_rede_rede_env */
            pthread_mutex_unlock(&mutex_rede_rede_env2);
        }
    }
}

void *enviarSegmento() {

    while (TRUE) {

        struct datagrama datagrama_rcv;

        /* Consumir buffer_rede_rede_rcv */
        pthread_mutex_lock(&mutex_rede_rede_rcv2);

        retirarDatagramaBufferRedeRedeRcv(&datagrama_rcv);

        /* Consumir buffer_rede_rede_rcv */
        pthread_mutex_unlock(&mutex_rede_rede_rcv1);

        /* Produzir buffer_trans_rede_rcv */
        pthread_mutex_lock(&mutex_trans_rede_rcv1);

        colocarDatagramaBufferTransRedeRcv(datagrama_rcv);

        /* Produzir buffer_trans_rede_rcv */
        pthread_mutex_unlock(&mutex_trans_rede_rcv2);

    }
}

void *enviarTabelaRotasJob() {

    while (TRUE){

        struct datagrama datagrama_env;

        sleep(rand() % 15 + 25);

        montarDatagramaTabelaRotas(&datagrama_env);

        enviarTabelaRotasVizinhos(&datagrama_env);

    }
}

void *enviarTabelaRotas() {

    int i, tetrj;
    pthread_t threadEnviarTabelaRotasJob;

    while (TRUE) {

        struct datagrama datagrama_env_inicial, datagrama_env;

        /* Executando pela primeira vez */
        if (flag_iniciei == 1) {

            /* Inicia a thread threadEnviarTabelaRotasJob */
            tetrj = pthread_create(&threadEnviarTabelaRotasJob, NULL, enviarTabelaRotasJob, NULL);

            if (tetrj) {
                printf("ERRO: impossivel criar a thread : enviarTabelaRotasJob\n");
                exit(-1);
            }

            pthread_detach(threadEnviarTabelaRotasJob);

            montarDatagramaTabelaRotas(&datagrama_env_inicial);

            enviarTabelaRotasVizinhos(&datagrama_env_inicial);

            flag_iniciei = 0;

        }

        /* Demais vezes, destravado pela thread receberTabelaRotas */
        pthread_mutex_lock(&mutex_rede_rede_atualizei2);

        montarDatagramaTabelaRotas(&datagrama_env);

        pthread_mutex_unlock(&mutex_rede_rede_atualizei1);

        enviarTabelaRotasVizinhos(&datagrama_env);

    }
}

void *receberTabelaRotas() {

    while (TRUE) {

        struct datagrama datagrama_rcv;

        /* Consumir buffer_rede_rede_rcv */
        pthread_mutex_lock(&mutex_rede_rede_receberotas2);

        retirarDatagramaBufferRedeRedeRcv(&datagrama_rcv);

        /* Consumir buffer_rede_rede_rcv */
        pthread_mutex_unlock(&mutex_rede_rede_rcv1);

        /* Travar e Desbloquear a threadEnviarTabelaRotas */
        pthread_mutex_lock(&mutex_rede_rede_atualizei1);

        atualizarTabelaRotas(datagrama_rcv);

        pthread_mutex_unlock(&mutex_rede_rede_atualizei2);

    }

}

void atualizarTabelaRotas(struct datagrama datagram) {
    int i;

    /* Salva quem enviou para não enviar para esse nó*/
    tabela_rotas[1].quem_enviou = datagram.num_no;

    #ifdef DEBBUG
    printf("[REDE - ROT]Recebi tabela de rotas do nó '%d'\n", tabela_rotas[1].quem_enviou);
    #endif

    for (i = 0; i < 6; i++) {

        /* Se nó destino -1 */
        if (tabela_rotas[i].destino == -1 && datagram.data.tabela_rotas[i].destino != -1)
        {
            tabela_rotas[i].destino = datagram.data.tabela_rotas[i].destino;
            tabela_rotas[i].custo = datagram.data.tabela_rotas[i].custo + 1;
            tabela_rotas[i].saida = datagram.num_no;
        }

        /* atualiza custos */
        if (tabela_rotas[i].custo + 1 > datagram.data.tabela_rotas[i].custo) {
            tabela_rotas[i].custo = datagram.data.tabela_rotas[i].custo + 1;
            tabela_rotas[i].saida = datagram.num_no;
        }
    }
}

void enviarTabelaRotasVizinhos(struct datagrama *datagram) {

    int i;

    /* Procura vizinhos para enviar tabela de rotas*/
    for (i = 0; i < 6; i++) {

        /* Se for vizinho, não for ele mesmo e não foi quem enviou*/
        if (nos_vizinhos[i] != file_info.num_no &&
            nos_vizinhos[i] != -1               &&
            nos_vizinhos[i] != tabela_rotas[1].quem_enviou) 
        {
            // Produzir buffer_rede_rede_env
            pthread_mutex_lock(&mutex_rede_rede_env1);

            memcpy(&(datagram->data.tabela_rotas), &tabela_rotas, sizeof (tabela_rotas));

            /* Seta variaveis para envio */
            datagram->env_no = nos_vizinhos[i];
            datagram->num_no = file_info.num_no;

            #ifdef DEBBUG
            printf("[REDE - ROT]Enviei Tabela de Rotas para o nó '%d'\n", nos_vizinhos[i]);
            #endif

            colocarDatagramaBufferRedeRedeEnv(*datagram);

            // Produzir buffer_rede_rede_env
            pthread_mutex_unlock(&mutex_rede_rede_env2);
        }
    }
    tabela_rotas[1].quem_enviou = -1;
}

void retirarDatagramaBufferRedeRedeRcv(struct datagrama *datagram) {

        /* Lock buffer_rede_rcv acesso exclusivo */
        pthread_mutex_lock(&mutex_buffer_rede_rcv);

        memcpy(datagram, &buffer_rede_rede_rcv, sizeof (buffer_rede_rede_rcv));

        /* Unlock buffer_rede_rcv acesso exclusivo */
        pthread_mutex_unlock(&mutex_buffer_rede_rcv);
}

void desfragmentarDatagramaRcv(struct datagrama datagram, int *index) {
    int i;
    int flag = -1;
    void *aux;

    /* Implementar MUTEX buffers_fragmentacao[i] */

    for (i = 0; i < MAX_BUFFERS_DESFRAG; i++) {

        /* Há datagramas anteriores com o mesmo id? */
        if (buffers_fragmentacao[i].id == datagram.id) {
            flag = i;
            break;
        }
    }
    if (flag == -1) {
        for (i = 0; i < MAX_BUFFERS_DESFRAG; i++) {

            /* Não há datagramas anteriores, ache a primeira posicao vazia */
            if (buffers_fragmentacao[i].id == -1) {
                flag = i;
                break;
            }
        }
    }

    /* index = flag */
    *index = flag;
    /* ponteiro recebe posicao de memoria */
    aux = &buffers_fragmentacao[i].data;
    /* pronteiro soma offset */
    aux += datagram.offset;
    /* copia conteudo do datagrama para posicao de memoria apontado por aux, tamanho de tam_buffer */
    memcpy(aux, &datagram.data, datagram.tam_buffer);
}

void fragmentarDatagramaEnv(struct datagrama datagram) {

    int i;
    int tam_total_datagrama = sizeof (datagram.data) - (TAM_MAX_BUFFER - datagram.tam_buffer); //Tamanho efetivo para dividir (desconsidera lixo)
    int qtde_divisao = 0; //Quantidade que será dividido os datagramas
    int tam_parte_final = 0; //Tamanho do ultimo datagrama
    int env_no = datagram.env_no; //Env_no
    int retorno = datagram.env_no; //Retorno
    int offset = 0; //Offset para gravar
    int MTU = buffer_rede_enlace_env.retorno; //MTU
    char buffer_interno[tam_total_datagrama]; //Buffer carrega o datagrama

    /* Copia conteudo para o buffer */
    memcpy(buffer_interno, &datagram.data, tam_total_datagrama);

    void *ptr = buffer_interno;

#ifdef DEBBUG
    printf("[REDE - FRAG] MTU excedido dividindo o pacote em max '%d' bytes\n\n", MTU);
#endif
    /* Se o ultimo datagrama tiver resto */
    if (tam_total_datagrama % MTU != 0) {
        qtde_divisao = (tam_total_datagrama) / MTU + 1;
        tam_parte_final = (tam_total_datagrama) % MTU;
        #ifdef DEBBUG_REDE_FRAGMENTAR
        printf("[REDE - FRAG] Tamanho total : '%d'.Dividirei em '%d' partes\n", (tam_total_datagrama), qtde_divisao);
        #endif
    }/* Se o ultimo datagrama não tiver resto */
    else {
        qtde_divisao = tam_total_datagrama / MTU;
        tam_parte_final = 0;

        #ifdef DEBBUG_REDE_FRAGMENTAR
        printf("[REDE - FRAG] Tamanho total : '%d'.Dividirei o datagrama em '%d' partes\n", (tam_total_datagrama), qtde_divisao);
        #endif

    }

    /* Loop para mandar os fragmentos dos datagramas*/
    for (i = 0; i < qtde_divisao; i++) {
        struct datagrama datagrama_env_aux;

        datagrama_env_aux.type = 1;
        datagrama_env_aux.env_no = env_no;
        datagrama_env_aux.offset = offset;
        datagrama_env_aux.retorno = retorno;
        datagrama_env_aux.id = flag_id;

        #ifdef DEBBUG_REDE_FRAGMENTAR
        printf("[REDE - FRAG] Offset: '%d'\n", datagrama_env_aux.offset);
        #endif

        /* Primeiro e Demais pacotes (exceto ultimo) */
        if (i + 1 != qtde_divisao) {
            datagrama_env_aux.tam_buffer = MTU; // Atualiza Tam_buffer
            datagrama_env_aux.mf = 1; // More Fragments

            memcpy(&datagrama_env_aux.data, ptr, MTU); // Copia Dados em Datagrama Auxiliar

#ifdef DEBBUG_REDE_FRAGMENTAR
            write(1, ptr, MTU); //  Printa o que enviou
#endif

            offset += MTU; // Atualiza Offset
            ptr += MTU; // Move Ponteiro
        }/* Se for o ultimo pacote e nao tem sobra */
        else if (i + 1 == qtde_divisao && tam_parte_final == 0) {
            datagrama_env_aux.tam_buffer = MTU; // Atualiza Tam_buffer
            datagrama_env_aux.mf = 0; // More Fragments

            /* Copia Dados em Datagrama Auxiliar */
            memcpy(&datagrama_env_aux.data, ptr, MTU);

#ifdef DEBBUG_REDE_FRAGMENTAR
            write(1, ptr, MTU); //Printa o que enviou
#endif
        }/* Se for o ultimo pacote e tem sobra */
        else if (i + 1 == qtde_divisao && tam_parte_final != 0) {
            datagrama_env_aux.tam_buffer = tam_parte_final; // Atualiza Tam_buffer
            datagrama_env_aux.mf = 0; // More Fragments

            memcpy(&datagrama_env_aux.data, ptr, tam_parte_final); // Copia Dados em Datagrama Auxiliar

#ifdef DEBBUG_REDE_FRAGMENTAR
            write(1, ptr, tam_parte_final); // Printa o que enviou
#endif
        }/* Qualqr outro >erro fatal< */
        else {
            printf("[REDE - FRAG] Erro Fatal(2)\n\n"); //  Estrouro de Excessão
            exit(2);
        }

        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_lock(&mutex_rede_enlace_env1);

        /* Coloca Datagrama no buffer_rede_enlace_env */
        colocarDatagramaBufferRedeEnlaceEnv(datagrama_env_aux);

        /* Se precisa enviar à nó não vizinho */
        if (flag_saida != 0)
            buffer_rede_enlace_env.env_no = flag_saida;

        /* Produzir buffer_rede_enlace_env */
        pthread_mutex_unlock(&mutex_rede_enlace_env2);

        /* Consome resposta da camada de enlace */
        pthread_mutex_lock(&mutex_rede_enlace_env1);

        /* Testa retorno */
        retornoEnlace(datagrama_env_aux);

        /* Consome resposta da camada de enlace */
        pthread_mutex_unlock(&mutex_rede_enlace_env1);
    }
}

void colocarDatagramaBufferRedeRedeEnv(struct datagrama datagrama_env) {

        /* Lock buffer_rede_env acesso exclusivo */
        pthread_mutex_lock(&mutex_buffer_rede_env);

                memcpy(&buffer_rede_rede_env, &datagrama_env, sizeof (datagrama_env));

        /* Unlock buffer_rede_env acesso exclusivo */
        pthread_mutex_unlock(&mutex_buffer_rede_env);

}

void colocarDatagramaBufferRedeEnlaceEnv(struct datagrama datagrama_env) {

    buffer_rede_enlace_env.tam_buffer = datagrama_env.tam_buffer;
    buffer_rede_enlace_env.env_no = datagrama_env.env_no;

    memcpy(&buffer_rede_enlace_env.data, &datagrama_env, sizeof (datagrama_env));
}

void colocarDatagramaBufferRedeRedeRcv(struct datagrama datagram) {

        /* Lock buffer_rede_rcv acesso exclusivo */
        pthread_mutex_lock(&mutex_buffer_rede_rcv);

            memcpy(&buffer_rede_rede_rcv, &datagram, sizeof (datagram));

        /* Unlock buffer_rede_rcv acesso exclusivo */
        pthread_mutex_unlock(&mutex_buffer_rede_rcv);

}

void colocarDatagramaBufferTransRedeRcv(struct datagrama datagram) {

    memcpy(&buffer_trans_rede_rcv.data, &datagram.data, sizeof (datagram.data));

}

void retirarDatagramaBufferRedeEnlaceRcv(struct datagrama *datagram) {

    if (buffer_rede_enlace_rcv.retorno != -1) {

        memcpy(datagram, &buffer_rede_enlace_rcv.data, sizeof (buffer_rede_enlace_rcv.data));

#ifdef DEBBUG
        printf("[REDE - RCV] Datagrama Recebido com sucesso!\n");
#endif

#ifdef DEBBUG_REDE
        printf("[REDE - RCV] Type: '%d', Tam_buffer: '%d' Bytes, ID: '%d', offset: '%d', MF: '%d'\n", datagram->type, datagram->tam_buffer,
                datagram->id, datagram->offset, datagram->mf);
#endif

    } else{

    #ifdef DEBBUG
        printf("[REDE - RCV] ECC não correspondente - Datagrama Descartado\n");
    #endif
    }
}

void retirarSegmentoBufferTransRedeEnv(struct datagrama *datagram) {

    memcpy(&(datagram->data), &buffer_trans_rede_env.data, sizeof (buffer_trans_rede_env.data));

    datagram->type        = 1;
    datagram->env_no      = buffer_trans_rede_env.env_no;
    datagram->num_no      = file_info.num_no;
    datagram->tam_buffer  = buffer_trans_rede_env.tam_buffer;
    datagram->offset      = 0;
    datagram->mf          = -1;

}

void retirarDatagramaBufferRedeRedeEnv(struct datagrama *datagram) {

            memcpy(datagram, &buffer_rede_rede_env, sizeof (buffer_rede_rede_env));

    /* Incrementa ID do pacote */
    datagram->id = flag_id;
    flag_id++;

}

int retornoEnlace() {

        /* Testa o retorno da camada de enlace */
        if (buffer_rede_enlace_env.retorno == 0) {
            #ifdef DEBBUG
            printf("[REDE - ENV] OK\n\n");
            #endif
        } else if (buffer_rede_enlace_env.retorno == -1) {
            #ifdef DEBBUG
            printf("[REDE - ENV] Não há ligacao do nó: '%d'!\n\n", buffer_rede_enlace_env.env_no);
            #endif
            return -1;
        } else if (buffer_rede_enlace_env.retorno > 0) {
            #ifdef DEBBUG
            printf("[REDE - ENV] MTU Excedido MAX '%d'\n", buffer_rede_enlace_env.retorno);
            #endif
            return buffer_rede_enlace_env.retorno;
        } else {
            printf("[REDE - ENV] Erro Fatal(1)\n\n");
            exit(1);
        }
    return 0;
}

void resetarBuffer(struct datagrama *datagram) {

    /* Após desfragmentacao reseta o buffer*/

    char aux[sizeof (datagram->data)] = "";

    datagram->tam_buffer = -1;
    datagram->offset = -1;
    datagram->id = -1;
    datagram->mf = -1;
    datagram->type = -1;
    datagram->env_no = -1;
    datagram->retorno = -1;
    memcpy(&(datagram->data), aux, sizeof (datagram->data));

}

void montarDatagramaTabelaRotas(struct datagrama *datagram) {

    datagram->tam_buffer = sizeof(tabela_rotas);
    datagram->offset = 0;
    datagram->id = flag_id;
    datagram->mf = -1;
    datagram->type = 2;
    datagram->retorno = -1;

}

void montarTabelaRotasInicial() {
    int i, j, k = 0;

    /* Inicializacao da variavel quem enviou */
    tabela_rotas[1].quem_enviou = -1;

    /* Inicializacao da variavel vizinhos */
    for (i = 0; i < 6; i++) {
        nos_vizinhos[i] = -1;
    }

    /* Inicializacao da tabela de rotas */
    for (i = 0; i < 6; i++) {
        tabela_rotas[i].destino = atoi(ligacao.nos[i][0]);
        if (file_info.num_no == atoi(ligacao.nos[i][0])) {
            tabela_rotas[i].custo = 0;
            tabela_rotas[i].saida = atoi(ligacao.nos[i][0]);
        } else {
            tabela_rotas[i].custo = INFINITO;
            tabela_rotas[i].saida = -1;
        }

        if (tabela_rotas[i].destino == -1) {
            tabela_rotas[i].custo = -1;
        }
    }

    for (i = 0; i < 18; i++) {

        /* Procura vizinhos do tipo 1 > 2*/
        if (ligacao.enlaces[i][0] == file_info.num_no) {
            for (j = 0; j < 6; j++) {
                if (ligacao.enlaces[i][1] == tabela_rotas[j].destino) {
                    tabela_rotas[j].custo = 1;
                    tabela_rotas[j].saida = ligacao.enlaces[i][1];

                    nos_vizinhos[k] = ligacao.enlaces[i][1];
                    k++;
                    break;
                }
            }
        }/* Procura vizinhos do tipo 2 > 1*/ 
        else if (ligacao.enlaces[i][1] == file_info.num_no) {
            for (j = 0; j < 6; j++) {
                if (ligacao.enlaces[i][0] == tabela_rotas[j].destino) {
                    tabela_rotas[j].custo = 1;
                    tabela_rotas[j].saida = ligacao.enlaces[i][0];

                    nos_vizinhos[k] = ligacao.enlaces[i][0];
                    k++;
                    break;
                }
            }
        }
    }

#ifdef DEBBUG_MONTAR_TABELA
    for (i = 0; i < 6; i++) {
        printf("[REDE - MONTAR TABELA]tabela_rotas[%d]].destino: '%d', tabela_rotas[%d]].custo: '%d', tabela_rotas[%d]].saida: '%d'\n", i,
                tabela_rotas[i].destino, i, tabela_rotas[i].custo, i, tabela_rotas[i].saida);
    }
#endif
}
