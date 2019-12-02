/*
*  myfs.c - Implementacao do sistema de arquivos MyFS
*
*  Autores: SUPER_PROGRAMADORES_C
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "myfs.h"
#include "vfs.h"
#include "inode.h"
#include "util.h"

#define INODE_BEGINSECTOR 2
#define INODE_ENDSECTOR = 5
#define FREE_SPACE_MANAGEMENT_BEGINSECTOR = 5
#define FREE_SPACE_MANAGEMENT_ENDSECTOR = 9
#define ROOT_DIRECTORY_BEGINSECTOR = 10
#define ROOT_DIRECTORY_ENDSECTOR = 13
#define DATA_BEGINSECTOR = 13

//Declaracoes globais
char fsid = 0;  // Identificador do tipo de sistema de arquivos
char *fsname = "myFS"; // Nome do tipo de sistema de arquivos

int myFSslot;
int fileDescriptors [MAX_FDS];

int NUM_SECTOR_PER_BLOCK = 4;

int META_DATA_NUM_ITEM = 6;


typedef struct
{
  int status;
  int type;
  unsigned int fd;
  unsigned int pointer;
  Disk *disk;
  Inode *inode;
  const char path[MAX_FILENAME_LENGTH + 1];
} FileDescriptor;

FileDescriptor files[MAX_FDS];


// Funcao para salvar os caracteres de "from" em "saveTo" a partir de um offset
void saveChar(char *saveTo, char *from, int offset, int arrayLength){
    int lengthOfArray = (int)(sizeof(from) / sizeof(from[0]));
    int j = offset;
    for(int i = 0; i < arrayLength; i++){
        saveTo[j] = from[i];
        j++;
    }
    printf("%s", saveTo);
    
}

void writeMetadata(Disk *d, char *metadataSector){
    char aux[4];                // Vetor para armazenar os valores de unsigned int
    unsigned int teste = 2;
    int offsetCounter = 0;
    ul2char(teste, aux);
    for(int i = 0; i < 4; i++){
        printf("%c", aux[i]);
    }
    saveChar(metadataSector, aux, 0, 4);
    
}

//Funcao para verificacao se o sistema de arquivos está ocioso, ou seja,
//se nao ha quisquer descritores de arquivos em uso atualmente. Retorna
//um positivo se ocioso ou, caso contrario, 0.
int myFSIsIdle (Disk *d) {
    for(int i = 0; i < MAX_FDS; i++){
//        if (files[i] != NULL && diskGetId(d) == diskGetId(files[i].disk)){
//                return 0;
//        }
    }
    return 1;
}

//Funcao para formatacao de um disco com o novo sistema de arquivos
//com tamanho de blocos igual a blockSize. Retorna o numero total de
//blocos disponiveis no disco, se formatado com sucesso. Caso contrario,
//retorna -1.
int myFSFormat (Disk *d, unsigned int blockSize) {
    char metadataSector[DISK_SECTORDATASIZE];
    for(int i = 0; i < DISK_SECTORDATASIZE; i++){
        metadataSector[i] = ' ';
    }
    writeMetadata(d, metadataSector);
    
    diskWriteSector(d, 2, metadataSector);
    
    for(int i = 0; i < MAX_FDS; i ++){
        fileDescriptors[i] = 0;
    }
    return -1;
}

//Funcao para abertura de um arquivo, a partir do caminho especificado
//em path, no disco montado especificado em d, no modo Read/Write,
//criando o arquivo se nao existir. Retorna um descritor de arquivo,
//em caso de sucesso. Retorna -1, caso contrario.
int myFSOpen (Disk *d, const char *path) {
	return -1;
}
	
//Funcao para a leitura de um arquivo, a partir de um descritor de
//arquivo existente. Os dados lidos sao copiados para buf e terao
//tamanho maximo de nbytes. Retorna o numero de bytes efetivamente
//lidos em caso de sucesso ou -1, caso contrario.
int myFSRead (int fd, char *buf, unsigned int nbytes) {
	return -1;
}

//Funcao para a escrita de um arquivo, a partir de um descritor de
//arquivo existente. Os dados de buf serao copiados para o disco e
//terao tamanho maximo de nbytes. Retorna o numero de bytes
//efetivamente escritos em caso de sucesso ou -1, caso contrario
int myFSWrite (int fd, const char *buf, unsigned int nbytes) {
	return -1;
}

//Funcao para fechar um arquivo, a partir de um descritor de arquivo
//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario
int myFSClose (int fd) {
	return -1;
}


//Não sera mais implementado
//Funcao para abertura de um diretorio, a partir do caminho
//especificado em path, no disco indicado por d, no modo Read/Write,
//criando o diretorio se nao existir. Retorna um descritor de arquivo,
//em caso de sucesso. Retorna -1, caso contrario.
int myFSOpenDir (Disk *d, const char *path) {
	return -1;
}

//Funcao para a leitura de um diretorio, identificado por um descritor
//de arquivo existente. Os dados lidos correspondem a uma entrada de
//diretorio na posicao atual do cursor no diretorio. O nome da entrada
//e' copiado para filename, como uma string terminada em \0 (max 255+1).
//O numero do inode correspondente 'a entrada e' copiado para inumber.
//Retorna 1 se uma entrada foi lida, 0 se fim de diretorio ou -1 caso
//mal sucedido
int myFSReadDir (int fd, char *filename, unsigned int *inumber) {
	return -1;
}

//Funcao para adicionar uma entrada a um diretorio, identificado por um
//descritor de arquivo existente. A nova entrada tera' o nome indicado
//por filename e apontara' para o numero de i-node indicado por inumber.
//Retorna 0 caso bem sucedido, ou -1 caso contrario.
int myFSLink (int fd, const char *filename, unsigned int inumber) {
	return -1;
}

//Funcao para remover uma entrada existente em um diretorio, 
//identificado por um descritor de arquivo existente. A entrada e'
//identificada pelo nome indicado em filename. Retorna 0 caso bem
//sucedido, ou -1 caso contrario.
int myFSUnlink (int fd, const char *filename) {
	return -1;
}

//Funcao para fechar um diretorio, identificado por um descritor de
//arquivo existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.	
int myFSCloseDir (int fd) {
	return -1;
}


//Funcao para instalar seu sistema de arquivos no S.O., registrando-o junto
//ao virtual FS (vfs). Retorna um identificador unico (slot), caso
//o sistema de arquivos tenha sido registrado com sucesso.
//Caso contrario, retorna -1
int installMyFS (void) {
    FSInfo *fs_info = (FSInfo*)malloc(sizeof(FSInfo));
    fs_info->fsname = fsname;
    fs_info->fsid = fsid;
    fs_info->closeFn = myFSClose;
    fs_info->closedirFn = myFSCloseDir;
    fs_info->formatFn = myFSFormat;
    fs_info->isidleFn = myFSIsIdle;
    fs_info->linkFn = myFSLink;
    fs_info->openFn = myFSOpen;
    fs_info->opendirFn = myFSOpenDir;
    fs_info->readFn = myFSRead;
    fs_info->readdirFn = myFSReadDir;
    fs_info->unlinkFn = myFSUnlink;
    fs_info->writeFn = myFSWrite;
    myFSslot = vfsRegisterFS(fs_info);
    return myFSslot;
}



