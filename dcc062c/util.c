/*
*  util.c - Funcoes uteis para manipulacao de dados
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#include <stdlib.h>
#include "util.h"

//Funcao para a conversao de unsigned int para um array de bytes (char[])
//O array c deve possuir numero de elementos suficiente para abrigar um 
//unsigned int como sequencia de bytes. Ex.: Em plataformas de 64 bits testadas
//unsigned int possui 4 bytes, neste caso c deve possuir 4 elementos
void ul2char (unsigned int ui, unsigned char *c) {
	for (int i = 0; i < sizeof (unsigned int); i++) 
		c[i] = (ui >> (i*8)) & 0xFF;
}

//Funcao para a conversao de um array de bytes (char[]) em um unsigned int
//Apenas os primeiros bytes do array c serao considerados, tantos quanto forem
//necessarios para construir o valor de ui. Ex.: Em plataformas de 64 bits
//testadas unsigned int possui 4 bytes, neste caso apenas os 4 primeiros
//elementos de c serao considerados
void char2ul (unsigned char *c, unsigned int *ui) {
	*ui = 0;
	for (int i = 0; i < sizeof (unsigned int); i++)
		*ui = *ui + (c[i] << (i*8));
}
