#
#	makefile.GNUmakefile
#
#  Guilherme Sividal - 09054512 
#  Vitor Rodrigo Vezani - 10159861
#
#  Created by Vitor Vezani on 19/03/13.
#  Copyright (c) 2013 Vitor Vezani. All rights reserved.
#

all: meuprog.o garbler.o enlace.o rede.o transporte.o aplicacao.o inicializador.o arquivo.o 
	gcc -o meuprog.exe meuprog.o garbler.o enlace.o rede.o transporte.o aplicacao.o inicializador.o arquivo.o

arquivo: arquivo.c
	gcc -c arquivo.c

garbler: garbler.c
	gcc -c garbler.c

enlace: enlace.c
	gcc -c enlace.c

rede: rede.c
	gcc -c rede.c

transporte: transporte.c
	gcc -c transporte.c

inicializador: inicializador.c
	gcc -c inicializador.c

aplicacao: aplicacao.c
	gcc -c aplicacao.c

meuprog: meuprog.c
	gcc -c meuprog.c

clean:
	rm -f *.o *.exe
