/*
*  vfs.h - Definicao da API comum de sistemas de arquivos (virtual FS)
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#ifndef VFS_H
#define VFS_H

#include "disk.h"

#define MAX_FDS 128             //Numero maximo de descritores de arquivos
#define MAX_FILENAME_LENGTH 255 //Comprimento maximo do nome de arquivos

#define FILETYPE_DIR 128    //Identificador de tipo de arquivo: diretorio
#define FILETYPE_REGULAR 64 //Identificador de tipo de arquivo: arq regular

//Estrutura para definicao da API de sistemas de arquivos.
//Deve ser preenchida com os ponteiros das respectivas funcoes e passada
//para registro por meio da funcao vfsRegister()
typedef struct fs_info {
	char fsid;  // Identificador do tipo de sistema de arquivos
	char *fsname; // Nome do tipo de sistema de arquivos

	//Funcao para verificacao se o sistema de arquivos está ocioso, ou seja,
	//se nao ha quisquer descritores de arquivos em uso atualmente. Retorna
	//um positivo se ocioso ou, caso contrario, 0.
	int (*isidleFn) (Disk *d);

	//Funcao para formatacao de um disco com o novo sistema de arquivos
	//com tamanho de blocos igual a blockSize. Retorna o numero total de
	//blocos disponiveis no disco, se formatado com sucesso. Caso contrario,
	//retorna -1.
	int (*formatFn) (Disk *d, unsigned int blockSize);

	//Funcao para abertura de um arquivo, a partir do caminho especificado
	//em path, no disco montado especificado em d, no modo Read/Write,
	//criando o arquivo se nao existir. Retorna um descritor de arquivo,
	//em caso de sucesso. Retorna -1, caso contrario.
	int (*openFn) (Disk *d, const char *path);

	//Funcao para a leitura de um arquivo, a partir de um descritor de
	//arquivo existente. Os dados lidos sao copiados para buf e terao
	//tamanho maximo de nbytes. Retorna o numero de bytes efetivamente
	//lidos em caso de sucesso ou -1, caso contrario.
	int (*readFn) (int fd, char *buf, unsigned int nbytes);

	//Funcao para a escrita de um arquivo, a partir de um descritor de
	//arquivo existente. Os dados de buf serao copiados para o disco e
	//terao tamanho maximo de nbytes. Retorna o numero de bytes
	//efetivamente escritos em caso de sucesso ou -1, caso contrario
	int (*writeFn) (int fd, const char *buf, unsigned int nbytes);

	//Funcao para fechar um arquivo, a partir de um descritor de arquivo
	//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario
	int (*closeFn) (int fd);

	//Funcao para abertura de um diretorio, a partir do caminho
	//especificado em path, no disco indicado por d, no modo Read/Write,
	//criando o diretorio se nao existir. Retorna um descritor de arquivo,
	//em caso de sucesso. Retorna -1, caso contrario.
	int (*opendirFn) (Disk *d, const char *path);

	//Funcao para a leitura de um diretorio, identificado por um descritor
	//de arquivo existente. Os dados lidos correspondem a uma entrada de
	//diretorio na posicao atual do cursor no diretorio. O nome da entrada
	//e' copiado para filename, como uma string terminada em \0 (max 255+1).
	//O numero do inode correspondente 'a entrada e' copiado para inumber.
	//Retorna 1 se uma entrada foi lida, 0 se fim do diretorio ou -1 caso
	//mal sucedido.
	int (*readdirFn) (int fd, char *filename, unsigned int *inumber);

	//Funcao para adicionar uma entrada a um diretorio, identificado por um
	//descritor de arquivo existente. A nova entrada tera' o nome indicado
	//por filename e apontara' para o numero de i-node indicado por inumber.
	//Retorna 0 caso bem sucedido, ou -1 caso contrario.
	int (*linkFn) (int fd, const char *filename, unsigned int inumber);

	//Funcao para remover uma entrada existente em um diretorio, 
	//identificado por um descritor de arquivo existente. A entrada e'
	//identificada pelo nome indicado em filename. Retorna 0 caso bem
	//sucedido, ou -1 caso contrario.
	int (*unlinkFn) (int fd, const char *filename);

	//Funcao para fechar um diretorio, identificado por um descritor de
	//arquivo existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.	
	int (*closedirFn) (int fd);

} FSInfo;

//Funcao para inicializacao do sistema de arquivos virtual
void vfsInit ( void );

//Funcao para a montagem do sistema de arquivos que sera' a raiz da arvore
//unica do sistema (Unix-like). Retorna 0 caso bem sucedido e -1 em contrario
int vfsMountRoot (Disk *d, char fsId);

//Funcao para a desmontagem do sistema de arquivos. Nao podem haver arquivos
//ou diretorios abertos para a desmontagem. Retorna 0 caso bem sucedido e -1 
//caso contrario
int vfsUnmountRoot ( void );

//Funcao para formatacao de um disco com o sistema de arquivos indicado pelo
//identificador do sistema de arquivos (fsId), com tamanho de blocos igual a
//blockSize. Retorna o numero total de blocos disponiveis no disco, se
//formatado com sucesso. Caso contrario, retorna -1.
int vfsFormat (Disk *d, unsigned int blockSize, char fsId);

//Funcao para abertura de um arquivo, a partir do caminho especificado em path,
//no modo Read/Write, criando o arquivo se nao existir. Retorna um descritor de 
//arquivo, em caso de sucesso. Retorna -1, caso contrario.
int vfsOpen (const char *path);

//Funcao para a leitura de um arquivo, a partir de um descritor de arquivo
//existente. Os dados lidos sao copiados para buf e terao tamanho maximo de
//nbytes. Retorna o numero de bytes efetivamente lidos em caso de sucesso ou
//-1, caso contrario.
int vfsRead (int fd, char *buf, unsigned int nbytes);

//Funcao para a escrita de um arquivo, a partir de um descritor de arquivo
//existente. Os dados de buf serao copiados para o disco e terao tamanho
//maximo de nbytes. Retorna o numero de bytes efetivamente escritos em caso
//de sucesso ou -1, caso contrario
int vfsWrite (int fd, const char *buf, unsigned int nbytes);

//Funcao para fechar um arquivo, a partir de um descritor de arquivo existente.
//Retorna 0 caso bem sucedido, ou -1 caso contrario
int vfsClose (int fd);

//Funcao para abertura de um diretorio, a partir do caminho especificado em
//path, no modo Read/Write, criando o diretorio se nao existir. Retorna um
//descritor de arquivo, em caso de sucesso. Retorna -1, caso contrario.
int vfsOpendir (const char *path);

//Funcao para a leitura de um diretorio, identificado por um descritor de
//arquivo existente. Os dados lidos correspondem a uma entrada de diretorio
//na posicao atual do cursor no diretorio. O nome da entrada e' copiado para
//filename, como uma string terminada em \0 (max 255+1). O numero do inode 
//correspondente 'a entrada e' copiado para inumber. Retorna 1 se uma entrada
//foi lida, 0 se fim de diretorio ou -1 caso mal sucedido
int vfsReaddir (int fd, char *filename, unsigned int *inumber);

//Funcao para adicionar uma entrada a um diretorio, identificado por um 
//descritor de arquivo existente. A nova entrada tera' o nome indicado por
//filename e apontara' para o numero de i-node indicado por inumber. Retorna 0\
//caso bem sucedido, ou -1 caso contrario.
int vfsLink (int fd, const char *filename, unsigned int inumber);

//Funcao para remover uma entrada existente em um diretorio, identificado por
//um descritor de arquivo existente. A entrada e' identificada pelo nome 
//indicado em filename. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int vfsUnlink (int fd, const char *filename);

//Funcao para fechar um diretorio, identificado por um descritor de arquivo
//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario.
int vfsClosedir (int fd);

//Registra novo sistema de arquivos. Retorna um identificador unico (slot),
//caso o sistema de arquivos tenha sido registrado com sucesso. Caso contrario,
//retorna -1
int vfsRegisterFS (FSInfo *fsInfo);

//Remove um sistema de arquivos.
int vfsUnregisterFS (char fsId);

//Escreve na saida padrao as informacoes sobre sistemas de arquivos registrados
void vfsDumpFSInfo (void);

#endif
