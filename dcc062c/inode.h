/*
*  inode.h - Definicao de estrutura de dados e operacoes para i-nodes
*
*  Autor: Marcelo Moreno
*  Projeto: Trabalho Pratico II - Sistemas Operacionais
*  Organizacao: Universidade Federal de Juiz de Fora
*  Departamento: Dep. Ciencia da Computacao
*
*  => NAO MODIFIQUE ESTE ARQUIVO <=
*
*/

#ifndef INODE_H
#define INODE_H

#include "disk.h"

//Tipo para representacao de i-nodes
typedef struct inode Inode;

//Funcao que retorna o numero de i-nodes por setor
unsigned int inodeNumInodesPerSector ( void );

//Funcao que retorna o numero do primeiro setor da area de i-nodes
unsigned int inodeAreaBeginSector ( void );

//Funcao que cria um i-node vazio, identificado pelo seu numero (number),
//que deve ser unico no sistema de arquivos. Retorna ponteiro para o i-node
//criado ou NULL se nao houver memoria suficiente ou number invalido. A funcao
//salva o i-node em disco, com conteudo vazio e, portanto, o sobrescreve se ja 
//existente
Inode* inodeCreate (unsigned int number, Disk *d);

//Funcao que limpa todo o conteudo de um i-node. O i-node e' salvo em disco,
//sobrescrevendo-o se ja existente. Retorna 0 se bem sucedido ou -1, caso
//contrario
int inodeClear (Inode *i);

//Funcao que persiste um i-node em seu disco. Retorna 0 se gravacao bem sucedida
//ou -1 caso contrario. I-nodes sao salvos a partir do setor 2. Numero de
//i-nodes por setor pode variar de acordo com o tamanho do tipo unsigned int
int inodeSave (Inode *i);

//Funcao que recupera um i-node a partir do disco. Retorna ponteiro para o
//i-node lido ou NULL em caso de falha.
Inode* inodeLoad (unsigned int number, Disk *d);

//Funcao que modifica o tipo de arquivo referente a um i-node
void inodeSetFileType (Inode *i, unsigned int fileType);

//Funcao que modifica o tamanho do arquivo referente a um i-node, em bytes
void inodeSetFileSize (Inode *i, unsigned int fileSize);

//Funcao que modifica o proprietario do arquivo referente a um i-node
void inodeSetOwner (Inode *i, unsigned int owner);

//Funcao que modifica o grupo proprietario do arquivo referente a um i-node
void inodeSetGroupOwner (Inode *i, unsigned int groupOwner);

//Funcao que modifica as permissoes de acesso ao arquivo referente a um i-node
void inodeSetPermission (Inode *i, unsigned int permission);

//Funcao que modifica o contador de referencia do arquivo referente a um i-node
void inodeSetRefCount (Inode *i, unsigned int refCount);

//Funcao que adiciona um endereco ao fim do array de blocos de um i-node
//Retorna -1 caso a inclusao do endereco nao seja bem sucedida
//E' a unica funcao que salva automaticamente o i-node em disco
int inodeAddBlock (Inode *i, unsigned int blockAddr);

//Funcao que retorna o numero de um i-node.
unsigned int inodeGetNumber (Inode *i);

//Funcao que retorna o numero de um i-node.
unsigned int inodeGetNextNumber (Inode *i);

//Funcao que retorna o tipo de arquivo referente a um i-node.
unsigned int inodeGetFileType (Inode *i);

//Funcao que retorna o tamanho do arquivo referente ao i-node, em bytes
unsigned int inodeGetFileSize (Inode *i);

//Funcao que retorna o proprietario do arquivo referente a um i-node
unsigned int inodeGetOwner (Inode *i);

//Funcao que retorna o grupo proprietario do arquivo referente a um i-node
unsigned int inodeGetGroupOwner (Inode *i);

//Funcao que retorna as permissoes de acesso do arquivo referente a um i-node
unsigned int inodeGetPermission (Inode *i);

//Funcao que retorna o contador de referencias do arquivo referente a um i-node
unsigned int inodeGetRefCount (Inode *i);

//Funcao que retorna o endereco correspondente a um bloco (blockNum) no array
//de blocos de um i-node. O i-node precisa ser o primeiro de sua cadeia.
//Retorna 0 se o bloco nao possuir endereco em blockNum
unsigned int inodeGetBlockAddr (Inode *i, unsigned int blockNum);

//Funcao que encontra um i-node livre em um disco, a partir do i-node de numero
//startFrom. Retorna o numero do inode livre encontrado ou 0 se nao encontrado.
unsigned int inodeFindFreeInode (unsigned int startFrom, Disk *d);

#endif
