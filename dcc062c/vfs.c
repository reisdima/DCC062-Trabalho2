/*
*  vfs.c - Implementacao da API comum de sistemas de arquivos (virtual FS)
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#include <stdio.h>
#include "vfs.h"
#include "inode.h"

#define MAX_INSTALLED_FS 4

FSInfo* installedFSInfo[MAX_INSTALLED_FS];
Disk* rootDisk;
FSInfo* rootFS;

//Funcao interna para a obtencao do FSInfo correspondente a um fsId
FSInfo* __vfsGetFSInfo (char fsId) {
        FSInfo *fsInfo = NULL;
        for (int i = 0; i < MAX_INSTALLED_FS; i++) {
		if ( !installedFSInfo[i] ) continue;
                if ( fsId == installedFSInfo[i]->fsid ) {
                        fsInfo = installedFSInfo[i];
                        break;
                }
	}
	return fsInfo;
}

//Funcao para inicializacao do sistema de arquivos virtual
void vfsInit ( void ) {
	for (int i=0; i<MAX_INSTALLED_FS; i++)
		installedFSInfo[i] = NULL;
	rootDisk = NULL;
	rootFS = NULL;	
}

//Funcao para a montagem do sistema de arquivos que sera' a raiz da arvore
//unica do sistema (Unix-like). Retorna 0 caso bem sucedido e -1 em contrario
int vfsMountRoot (Disk *d, char fsId) {
	if ( !d ) return -1;
	rootFS = __vfsGetFSInfo (fsId);
	if ( !rootFS ) return -1;
	rootDisk = d;
	return 0;
}

//Funcao para a desmontagem do sistema de arquivos. Nao podem haver arquivos
//ou diretorios abertos para a desmontagem. Retorna 0 caso bem sucedido e -1
//caso contrario
int vfsUnmountRoot ( void ) {
	if ( !rootDisk || !rootFS ) return -1;
	if ( !rootFS->isidleFn (rootDisk) ) return -1;
	rootFS = NULL;
	rootDisk = NULL;
	return 0;
}

//Funcao para formatacao de um disco com o sistema de arquivos indicado pelo
//identificador do sistema de arquivos (fsId), com tamanho de blocos igual a
//blockSize. Retorna o numero total de blocos disponiveis no disco, se
//formatado com sucesso. Caso contrario, retorna -1.
int vfsFormat (Disk *d, unsigned int blockSize, char fsId) {
	FSInfo *fsInfo = NULL;
	if ( !d ) return -1;
	fsInfo = __vfsGetFSInfo (fsId);
	if ( !fsInfo ) return -1;
	return fsInfo->formatFn (d, blockSize);
}

//Funcao para abertura de um arquivo, a partir do caminho especificado em path,
//no modo Read/Write, criando o arquivo se nao existir. Retorna um descritor de 
//arquivo, em caso de sucesso. Retorna -1, caso contrario.
//Descritores de arquivo se iniciam em 1
int vfsOpen (const char *path) {
	if ( !rootDisk || !rootFS ) return -1;
	return rootFS->openFn (rootDisk, path);
}

//Funcao para a leitura de um arquivo, a partir de um descritor de arquivo
//existente. Os dados lidos sao copiados para buf e terao tamanho maximo de
//nbytes. Retorna o numero de bytes efetivamente lidos em caso de sucesso ou
//-1, caso contrario.
int vfsRead (int fd, char *buf, unsigned int nbytes) {
	if ( !rootDisk || !rootFS ) return -1;
	return rootFS->readFn (fd, buf, nbytes);
}

//Funcao para a escrita de um arquivo, a partir de um descritor de arquivo
//existente. Os dados de buf serao copiados para o disco e terao tamanho
//maximo de nbytes. Retorna o numero de bytes efetivamente escritos em caso
//de sucesso ou -1, caso contrario
int vfsWrite (int fd, const char *buf, unsigned int nbytes) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->writeFn (fd, buf, nbytes);
}

//Funcao para fechar um arquivo, a partir de um descritor de arquivo existente.
//Retorna 0 caso bem sucedido, ou -1 caso contrario
int vfsClose (int fd) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->closeFn (fd);
}

//Funcao para abertura de um diretorio, a partir do caminho especificado em
//path, no modo Read/Write, criando o diretorio se nao existir. Retorna um
//descritor de arquivo, em caso de sucesso. Retorna -1, caso contrario.
int vfsOpendir (const char *path) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->opendirFn (rootDisk, path);
}

//Funcao para a leitura de um diretorio, identificado por um descritor de
//arquivo existente. Os dados lidos correspondem a uma entrada de diretorio
//na posicao atual do cursor no diretorio. O nome da entrada e' copiado para
//filename, como uma string terminada em \0 (max 255+1). O numero do inode
//correspondente 'a entrada e' copiado para inumber. Retorna 1 se uma entrada
//foi lida, 0 se fim do diretorio ou -1 caso mal sucedido.
int vfsReaddir (int fd, char *filename, unsigned int *inumber) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->readdirFn (fd, filename, inumber);
}

//Funcao para adicionar uma entrada a um diretorio, identificado por um 
//descritor de arquivo existente. A nova entrada tera' o nome indicado por
//filename e apontara' para o numero de i-node indicado por inumber. Retorna 0\
//caso bem sucedido, ou -1 caso contrario.
int vfsLink (int fd, const char *filename, unsigned int inumber) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->linkFn (fd, filename, inumber);
}

//Funcao para remover uma entrada existente em um diretorio, este identificado
//por um descritor de arquivo existente. A entrada e' identificada pelo nome 
//indicado em filename. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int vfsUnlink (int fd, const char *filename) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->unlinkFn (fd, filename);
}

//Funcao para fechar um diretorio, identificado por um descritor de arquivo
//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int vfsClosedir (int fd) {
        if ( !rootDisk || !rootFS ) return -1;
        return rootFS->closedirFn (fd);
}

//Registra novo sistema de arquivos. Retorna um identificador unico (slot),
//caso o sistema de arquivos tenha sido registrado com sucesso. Caso contrario,
//retorna -1
int vfsRegisterFS (FSInfo* fsInfo) {
	int i;
	if ( !fsInfo ) return -1;
	for (i=MAX_INSTALLED_FS; i>0; i--)
		if ( !installedFSInfo[i-1] ) {
			installedFSInfo[i-1] = fsInfo;
			return 0;
		}
	return -1;
}

//Desfaz o registro de um sistema de arquivos. Um sistema de arquivos montado
//nao pode ter seu registro desfeito. Retorna 0 se bem sucedido e -1 caso
//contrario
int vfsUnregisterFS(char fsId) {
	if (fsId == rootFS->fsid) return -1;
	for (int i=0; i<MAX_INSTALLED_FS; i++) {
		if ( !installedFSInfo[i] ) continue;
		if ( fsId == installedFSInfo[i]->fsid ) {
			installedFSInfo[i] = NULL;
			return 0;
		}
	}
	return -1;
}

//Escreve na saida padrao as informacoes sobre sistemas de arquivos registrados
void vfsDumpFSInfo (void) {
	int noFS = 1;
	for (int i=0; i<MAX_INSTALLED_FS; i++)
		if ( installedFSInfo[i] ) {
			noFS = 0;
			printf ("\n-- FSInfo: FS ID: %u; FS Name: %s\n",
					installedFSInfo[i]->fsid,
					installedFSInfo[i]->fsname);
		}
	if (noFS) printf ("\n!! FSInfo: No file systems supported\n");
}

