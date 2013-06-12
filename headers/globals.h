//
//  globals.h
//
//  Guilherme Sividal    - 09054512 
//  Vitor Rodrigo Vezani - 10159861
//
//  Created by Vitor Vezani on 19/03/13.
//  Copyright (c) 2013 Vitor Vezani. All rights reserved.
//

#include <stdio.h>		    /* para printf() */
#include <stdlib.h>			/* para exit() */
#include <string.h>			/* para manipulação de string */
#include <pthread.h>     	/* para manipulação de threads */
#include <sys/socket.h>     /* para sockets */
#include <netinet/in.h>     /* para protocols */
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Defines
#define MAX_NO              6
#define MAX_TAM_NOME_ARQ    25

#define CONECTAR            0
#define DESCONECTAR         1
#define DADOS               2
#define BAIXAR              3

#define TRUE 	            1
#define FALSE	            0

#define TAM_MAX_BUFFER      2000

#define TAM_SEGMENT         50
#define TAM_JANELA          TAM_SEGMENT * 4
#define TAM_BUFFER_TRANS    2000

#define NOS                 1
#define ENLACES             2

#define MAX_BUFFERS_DESFRAG 5

#define MAX_PS              10
#define MAX_IC              10

#define INFINITO            999999

//#define DEBBUG_ARQUIVO
//#define DEBBUG_ENLACE
//#define DEBBUG_REDE
//#define DEBBUG_REDE_FRAGMENTAR
//#define DEBBUG_REDE_DESFRAGMENTAR
//#define DEBBUG_MONTAR_TABELA
//#define DEBBUG_ROTEAMENTO
#define DEBBUG_TRANSPORTE
//#define DEBBUG_APLI
#define DEBBUG_APLI_BAIXAR
//#define DEBBUG

// Variaveis Globais

int nos_vizinhos[6]; 	// Nós vizinhos
int flag_id; 			// Inicializa ID em 1
int flag_iniciei; 		// Enviar tabela de rotas à vizinhos
int flag_saida; 		// Qual nó enviar
int ps[MAX_PS];             // Estrutura do PS
int ack;                // Ack
int syn;                // Sync

pthread_mutex_t mutex_rede_enlace_env1, mutex_rede_enlace_env2;
pthread_mutex_t mutex_rede_enlace_rcv1, mutex_rede_enlace_rcv2;
pthread_mutex_t mutex_rede_rede_atualizei1, mutex_rede_rede_atualizei2;
pthread_mutex_t mutex_rede_rede_receberotas2;
pthread_mutex_t mutex_rede_rede_env1, mutex_rede_rede_env2;
pthread_mutex_t mutex_rede_rede_rcv1, mutex_rede_rede_rcv2;
pthread_mutex_t mutex_trans_rede_env1, mutex_trans_rede_env2;
pthread_mutex_t mutex_trans_rede_rcv1, mutex_trans_rede_rcv2;
pthread_mutex_t mutex_trans_trans_env1, mutex_trans_trans_env2;
pthread_mutex_t mutex_trans_trans_rcv1, mutex_trans_trans_rcv2;
pthread_mutex_t mutex_apli_trans_env1, mutex_apli_trans_env2;
pthread_mutex_t mutex_apli_trans_rcv1, mutex_apli_trans_rcv2;
pthread_mutex_t env_seg_rcv_seg_timer_2;
pthread_mutex_t mutex_trans_acess_exc_timer;

/* Estrutura do ic */
struct ic {
    int conectado;
    int env_no;
    int ps;
    int num_no;
    char *end_buffer;
};

struct ic ic[MAX_IC];       // Estrutura contendo os IC do nó

/* Estrutura do pacote */
struct pacote {
    char nome_arq[MAX_TAM_NOME_ARQ];
    int tipo;
    int tam_buffer;
    int mp;
    int num_no;
    char *retorno;
    char buffer[TAM_MAX_BUFFER];
};

/* Estrutura do buffer entre aplicacao e transporte */
struct buffer_apli_trans {
    struct ic ic;
    int tam_buffer;
    int tipo;
    int env_no;
    char *retorno;
    struct pacote data;

};

/* Estrutura do buffer interno à camada de transporte */
struct buffer_trans_trans {
    int tam_buffer;
    struct ic ic;
    char *data;
};

/* Estrutura do segmento */
struct segmento {
    struct ic ic;
    int flag_ack;
    int flag_connect;
    int flag_syn;
    int flag_push;
    int flag_baixar;
    int ack;
    int seqnum;
    int num_no;
    int env_no;
    int tam_buffer;
    struct pacote data;
};

/* Estrutura do buffer entre transporte e rede */
struct buffer_trans_rede {
    int tam_buffer;
    int env_no;
    int retorno;
    struct segmento data;
};

/* Estrutura da tabela de rotas */
struct tabela_rotas {
    int quem_enviou;
    int destino;
    int custo;
    int saida;
};

/* uniao do segmento ou tabela de rotas */
union segmento_tabela { 
    struct segmento segmento;
    struct tabela_rotas tabela_rotas[6];
};

/* Estrutura do datagrama */
struct datagrama {
    int tam_buffer;
    int offset;
    int id;
    int mf;
    int type;
    int env_no;
    int num_no;
    int retorno;
    union segmento_tabela data;
};

/* Estrutura do buffer entre rede e enlace */
struct buffer_rede_enlace {
    int tam_buffer;
    int env_no;
    int retorno;
    struct datagrama data;
};

/* Estrutura do Frame */
struct frame {
    int tam_buffer;
    int ecc;
    struct datagrama data;
};

struct ligacoes {
    char nos[6][3][35];
    int enlaces[18][3];
};

struct file {
    char file_name[20];
    int num_no;
};

struct tabela_rotas tabela_rotas[6]; 		// Tabela de Rotas Interna do nó 

struct datagrama buffers_fragmentacao[MAX_BUFFERS_DESFRAG]; // Buffer interno de fragmentos
struct datagrama buffer_rede_rede_env, buffer_rede_rede_rcv; // Buffer interno entre threads

struct buffer_apli_trans buffer_apli_trans_env, buffer_apli_trans_rcv;
struct buffer_trans_trans buffer_trans_trans_env, buffer_trans_trans_rcv;
struct buffer_rede_enlace buffer_rede_enlace_env, buffer_rede_enlace_rcv;
struct buffer_trans_rede buffer_trans_rede_env, buffer_trans_rede_rcv;

struct file file_info;

struct ligacoes ligacao;

//Variaveis Globais à Camada de Rede

pthread_mutex_t mutex_buffer_rede_env, mutex_buffer_rede_rcv;

//Threads Inicializador
void *inicializarCamadas();
void *iniciarEnlace();
void *iniciarRede();
void *iniciarTransporte();
void *iniciarAplicacao();

// Threads da Camada de Enlace
void *enviarFrames();
void *receberFrames();

//Threads da Camada de Rede
void *enviarTabelaRotas();
void *enviarTabelaRotasJob();
void *receberTabelaRotas();
void *receberDatagramas();
void *receberSegmento();
void *enviarDatagrama();
void *enviarSegmento();

// Threads da Camada de Transporte
void *enviarSegmentos();
void *receberSegmentos();
void *enviarPacote();
void *receberPacote();
void *timer();

//Threads da Camada de Aplicação 
void *receberPacotes();

//Funções Arquivos
void delete_espace(char* input);

//Funções da Camada de Enlace
void colocarArquivoStruct(FILE * fp);
void retirarEspaco(char * string);
void colocarBufferFrame(struct frame *frame);
void colocarDatagramaBuffer(struct frame frame);
int checkSum(struct datagrama datagram);

//Funções da Camada de Rede
void atualizarTabelaRotas(struct datagrama datagram);
void fragmentarDatagramaEnv(struct datagrama datagram);
void desfragmentarDatagramaRcv(struct datagrama datagram, int *index);
void enviarDatagramaNoNaoV(struct datagrama datagram);

void retirarDatagramaBufferRedeRedeRcv(struct datagrama *datagram);
void retirarDatagramaBufferRedeRedeEnv(struct datagrama *datagram);
void retirarDatagramaBufferRedeEnlaceRcv(struct datagrama *datagram);
void retirarSegmentoBufferTransRedeEnv(struct datagrama *datagram);

void colocarDatagramaBufferRedeRedeEnv(struct datagrama datagrama);
void colocarDatagramaBufferRedeRedeRcv(struct datagrama datagrama);
void colocarDatagramaBufferTransRedeEnv(struct datagrama datagrama);
void colocarDatagramaBufferTransRedeRcv(struct datagrama datagrama);

void colocarDatagramaBufferRedeEnlaceEnv(struct datagrama datagrama);
int retornoEnlace();

void resetarBuffer(struct datagrama *datagram);

void montarTabelaRotasInicial();
void montarDatagramaTabelaRotas(struct datagrama *datagram);
void enviarTabelaRotasVizinhos(struct datagrama *datagram);

//Funções da Camada de Transporte
void colocarPacoteBufferApliTransRcv(struct pacote pacote);
void retirarPacoteBufferApliTransEnv(struct pacote *pacote, struct ic *ic);

void colocarPacoteBufferTransTransEnv(struct pacote package, struct ic ic);
void retirarPacoteBufferTransTransRcv(struct pacote *pacote);

void colocarSegmentoBufferTransTransRcv(struct segmento segment, int seqnum);
void retirarSegmentoBufferTransTransEnv(struct segmento *segment, int nextseqnum);

void colocarSegmentoBufferTransRedeEnv(struct segmento segment);
void retirarSegmentoBufferTransRedeRcv(struct segmento *segment);

void montarSegmentoAck(struct segmento *segment, struct segmento segment_rcv, int expectedseqnum);

//Funções da Camada de Aplicacao
void colocarPacotesBufferApliTransEnv(struct pacote pacote, struct ic ic);
void retirarPacotesBufferApliTransRcv(struct pacote *pacote);
void retornoTransporte();

//Funções da Camada de Aplicacao
int aps();
int fps(int num_ps);
int conectar(int env_no, int ps);
int desconectar(int index);
int baixar(int index, char *data);
int enviar(int index, char nome_arq[MAX_TAM_NOME_ARQ],char *data, int tam_arq);
void sobre();
void howto();