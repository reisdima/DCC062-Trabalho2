/*
*  disk.c - Definicao das informacoes e operacoes sobre discos fisicos
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
#include <stdio.h>
#include "disk.h"

#define DISK_SEEKDELAY 10

#define DISK_SECTORSPERTRACK 64
#define DISK_SECTORDATAOFFSET 3
#define DISK_SECTORTOTALSIZE (2*DISK_SECTORDATAOFFSET+DISK_SECTORDATASIZE)

#define DISK_SECTORPREAMBLE " [["
#define DISK_SECTORECC "]] "

//Estrutura para a representação de um disco fisico.
//Seus membros etao protegidos, portanto use o tipo Disk e as funcoes externalizadas por disk.h.
struct disk {
	int id;				//Identificador do disco no sistema
	FILE* fp;			//Arquivo que implementa o disco
	unsigned long numCylinders;	//Numero de cilindros
	unsigned long numSectors;	//Numero de setores
	unsigned long size;		//Espaco util total para dados no disco
	unsigned long currCylinder;	//Cilindro atual 
};


//Funcao interna, privada, para realizar o posicionamento
//da cabeca sobre o setor desejado para leitura ou escrita
//Insere um atraso a cada cilindro deslocado no percurso
void __diskSeek(Disk *d, unsigned long addr) {
	unsigned long reqCyl, cylOffset;
	unsigned long sectorPos = addr * DISK_SECTORTOTALSIZE;
	unsigned long dataPos = sectorPos + DISK_SECTORDATAOFFSET;

 	diskAddrToCylinder (d, addr, &reqCyl);
	cylOffset = (reqCyl < d->currCylinder 
                     ? d->currCylinder - reqCyl
		     : reqCyl - d->currCylinder);

	for (unsigned long i=1; i <= cylOffset; i++)
		SLEEP (DISK_SEEKDELAY);

	fseek (d->fp, dataPos, 0);
	d->currCylinder = reqCyl;
}

//Funcao que conecta um disco fisico ao sistema operacional.
//Um disco fisico eh implementado por meio de um arquivo regular, 
//cujo caminho eh dado por rawDiskPath.
//O parametro id eh um identificador unico para o disco, controlado
//pelo sistema operacional. Se o disco existir, retorna um ponteiro para Disk.
//Caso contrario, retorna NULL
Disk* diskConnect(int id, char* rawDiskPath) {
	Disk* d = NULL;
	FILE *fp = fopen(rawDiskPath,"r+");
	if (fp!=NULL) {
		d = malloc(sizeof (Disk));
		d->id = id;
		d->fp = fp;
		fseek (fp, 0, SEEK_END);
		d->numSectors = ftell (fp) / DISK_SECTORTOTALSIZE;
		d->numCylinders = d->numSectors / DISK_SECTORSPERTRACK;
		d->size = d->numSectors * DISK_SECTORDATASIZE;
		d->currCylinder = 0;
	}
	return d;
}

//Funcao que disconecta um disco fisico do sistema operacional
int diskDisconnect(Disk* d) {
	int result = fclose (d->fp);
	free(d);
	return result;
}

//Funcao que retorna o identificador de um disco fisico, conforme atribuido
//pelo sistema operacional no momento da conexao
int diskGetId (Disk* d) {
	return d->id;
}

//Funcao que retorna o numero total de setores de um disco fisico
unsigned long diskGetNumSectors (Disk* d) {
	return d->numSectors;
}

//Funcao que retorna o numero total de cilindros de um disco fisico
unsigned long diskGetNumCylinders (Disk* d) {
	return d->numCylinders;
}

//Funcao que retorna o tamanho do espaco util de dados de um disco fisico
//em bytes
unsigned long diskGetSize (Disk* d) {
	return d->size;
}

//Funcao que retorna o cilindro sobre o qual as cabecas estao atualmente
//posicionadas em um disco
unsigned long diskGetCurrentCylinder (Disk* d) {
	return d->currCylinder;
}

//Funcao que escreve em *cyl o numero do cilindro correspondente a um endereco
//(addr) LBA de setor de um disco. Retorna 0 se o endereco for valido e -1
//caso contrario
int diskAddrToCylinder (Disk* d, unsigned long addr, unsigned long *cyl) {
	*cyl = addr / DISK_SECTORSPERTRACK;
	return (addr < d->numSectors ? 0 : -1);
}

//Funcao para realizar a leitura de um setor identificado pelo endereco LBA
//(addr). Os dados sao transferidos para *data. Retorna 0 se a leitura ocorreu
//sem erros e -1 caso contrario
int diskReadSector (Disk* d, unsigned long addr, unsigned char *data) {
	if (addr >= d->numSectors) return -1;
	__diskSeek (d,addr);
	if (fread (data, 1, DISK_SECTORDATASIZE, d->fp) != DISK_SECTORDATASIZE)
		return -1;
	return 0;
}

//Funcao para realzar a escrita de um setor identificado pelo endereco LBA
//(addr). Os dados sao transferidos a partir de *data. Retorna 0 se a leitura
//ocorreu sem erros e -1 caso contrario
int diskWriteSector (Disk* d, unsigned long addr, unsigned char* data) {
	if (addr >= d->numSectors) return -1;
	__diskSeek (d,addr);
	if (fwrite (data, 1, DISK_SECTORDATASIZE, d->fp) != DISK_SECTORDATASIZE)
		return -1;
	return 0;
}

//Funcao para a criacao de um disco fisico, a ser representado pelo arquivo
//regular indicado por rawDiskPath e com numero total de cilindros indicado
//por numCylinders. Retorna 0 se o disco fisico for criado com sucesso e -1
//caso contrario. O disco fisico ja eh criado com formatacao de baixo nivel
int diskCreateRawDisk (char* rawDiskPath, unsigned long numCylinders) {
	FILE* fp;
	if (numCylinders == 0) return -1;
	fp = fopen (rawDiskPath, "w+");
	if (fp == NULL) return -1;
	for (unsigned long i = 0; i < numCylinders; i++) {
		for (int j = 0; j < DISK_SECTORSPERTRACK; j++) {
			fwrite(DISK_SECTORPREAMBLE,1,DISK_SECTORDATAOFFSET,fp);
			for (int k = 0; k < DISK_SECTORDATASIZE; k++)
				fwrite(" ",1,1,fp);
			fwrite(DISK_SECTORECC,1,DISK_SECTORDATAOFFSET,fp);
		}
//		fflush(stdout);
	}
	fclose(fp);
	return 0;
}
