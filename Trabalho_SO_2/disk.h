/*
*  disk.h - Definicao de informacoes e operacoes sobre um disco
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#ifndef DISK_H
#define DISK_H

//Escolha pela funcao de sleep suportada pelo sistema operacional hospedeiro
//Windows: Sleep()
//Unix: nanosleep()
#ifdef _WIN32
#   include <windows.h>
#   define SLEEP(msecs) Sleep(msecs)
#else
#   include <time.h>
#   define SLEEP(msecs) do {            \
        struct timespec ts;             \
        ts.tv_sec = msecs/1000;         \
        ts.tv_nsec = msecs%1000*1000000L;\
        nanosleep(&ts, NULL);           \
        } while (0)
#endif

//Tamanho padrao do setor de qualquer disco, em bytes
#define DISK_SECTORDATASIZE 512

//Tipo de dados para a representacao de discos fisicos
typedef struct disk Disk;

//Funcao que conecta um disco fisico ao sistema operacional.
//Um disco fisico eh implementado por meio de um arquivo regular, 
//cujo caminho eh dado por rawDiskPath.
//O parametro id eh um identificador unico para o disco, controlado
//pelo sistema operacional. Se o disco existir, retorna um ponteiro para Disk.
//Caso contrario, retorna NULL
Disk* diskConnect(int id, char* diskFilePath);

//Funcao que disconecta um disco fisico do sistema operacional
int diskDisconnect(Disk* d);

//Funcao que retorna o identificador de um disco fisico, conforme atribuido
//pelo sistema operacional no momento da conexao
int diskGetId (Disk* d);

//Funcao que retorna o numero total de setores de um disco fisico
unsigned long diskGetNumSectors (Disk* d);

//Funcao que retorna o numero total de cilindros de um disco fisico
unsigned long diskGetNumCylinders (Disk* d);

//Funcao que retorna o tamanho do espaco util de dados de um disco fisico
//em bytes
unsigned long diskGetSize (Disk* d);

//Funcao que retorna o cilindro sobre o qual as cabecas estao atualmente
//posicionadas em um disco
unsigned long diskGetCurrentCylinder (Disk* d);

//Funcao que escreve em *cyl o numero do cilindro correspondente a um endereco
//(addr) LBA de setor de um disco. Retorna 0 se o endereco for valido e -1
//caso contrario
int diskAddrToCylinder (Disk* d, unsigned long addr, unsigned long *cyl);

//Funcao para realizar a leitura de um setor identificado pelo endereco LBA
//(addr). Os dados sao transferidos para *data. Retorna 0 se a leitura ocorreu
//sem erros e -1 caso contrario
int diskReadSector (Disk* d, unsigned long addr, unsigned char* data);

//Funcao para realzar a escrita de um setor identificado pelo endereco LBA
//(addr). Os dados sao transferidos a partir de *data. Retorna 0 se a leitura
//ocorreu sem erros e -1 caso contrario
int diskWriteSector (Disk* d, unsigned long int addr, unsigned char* data);

//Funcao para a criacao de um disco fisico, a ser representado pelo arquivo
//regular indicado por rawDiskPath e com numero total de cilindros indicado
//por numCylinders. Retorna 0 se o disco fisico for criado com sucesso e -1
//caso contrario. O disco fisico ja eh criado com formatacao de baixo nivel
int diskCreateRawDisk (char* rawDiskPath, unsigned long numCylinders);

#endif
