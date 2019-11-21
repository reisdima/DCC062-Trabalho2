/*
*  inode.c - Definicao das informacoes e operacoes sobre i-nodes
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
#include "inode.h"
#include "util.h"

#define INODE_SIZE 16		//Tamanho do i-node em numero de unsigned ints
#define NUMBLOCKS_PERINODE 8	//No. de enderecos de bloco por i-node
#define NUMITEMS_PERINODE (INODE_SIZE - 2)	//Numero de "itens" por i-node
#define INODE_ITEM_BLOCKADDR 0		//Itens 0 a 7: Enderecos de bloco
#define INODE_ITEM_FILETYPE (INODE_SIZE - 8)	//Item 8: Tipo de arquivo
#define INODE_ITEM_FILESIZE (INODE_SIZE - 7)	//Item 9: Tamanho do arquivo
#define INODE_ITEM_OWNER (INODE_SIZE - 6)	//Item 10: Proprietario
#define INODE_ITEM_GROUPOWNER (INODE_SIZE - 5)	//Item 11: Grupo Proprietario
#define INODE_ITEM_PERMISSION (INODE_SIZE - 4)	//Item 12: Permissao
#define INODE_ITEM_REFCOUNT (INODE_SIZE - 3)	//Item 13: Contador referencia

#define INODE_BEGINSECTOR 2

//Tipo para representacao de i-nodes
struct inode {
	unsigned int inodeItem[NUMITEMS_PERINODE]; //Blocos e dados do i-node
	unsigned int number; 	//Numero do i-node
	unsigned int next;	//Numero do proximo i-node em caso de extensao
	Disk *d; 		//Disco ao qual pertence o i-node
};

//Funcao interna que retorna a ultima extensao de um i-node. Retorna NULL
//se nao houver extensoes do i-node fornecido.
Inode* __inodeGetLastExtension (Inode *i) {
	unsigned int niNumber = 0;
	Disk *d = i->d;
	if (i->next) {
		niNumber = i->next;
		i = inodeLoad (niNumber, d);
		if (!i) return NULL;
	} 
	else return NULL;
	while (i->next != 0) {
		niNumber = i->next;
		free (i);
		i = inodeLoad (niNumber, d);
		if (!i) return NULL;
	}
	return i;
}

//Funcao que retorna o numero de i-nodes por setor
unsigned int inodeNumInodesPerSector ( void ) {
	return DISK_SECTORDATASIZE / (INODE_SIZE * sizeof (unsigned int));
}

//Funcao que retorna o numero do primeiro setor da area de i-nodes
unsigned int inodeAreaBeginSector ( void ) {
	return INODE_BEGINSECTOR;
}

//Funcao que cria um i-node vazio, identificado pelo seu numero (number),
//que deve ser unico no sistema de arquivos. Retorna ponteiro para o i-node
//criado ou NULL se nao houver memoria suficiente ou number invalido. A funcao
//salva o i-node em disco, com conteudo vazio e, portanto, o sobrescreve se ja 
//existente
Inode* inodeCreate (unsigned int number, Disk *d) {
	if (number < 1) return NULL;
	Inode *i = malloc (sizeof(Inode));
	i->d = d;
	i->number = number;
	i->next = 0;
	if ( inodeClear (i) == 0 ) return i;
	else free (i);
	return NULL;
}

//Funcao que limpa todo o conteudo de um i-node. O i-node e' salvo em disco,
//sobrescrevendo-o se ja existente. Retorna 0 se bem sucedido ou -1, caso contrario
int inodeClear (Inode *i) {
	if (i) {
		if (i->next != 0) {
			Inode* ni = inodeLoad (i->next, i->d);
			if ( !ni ) return -1;
			if ( inodeClear (ni) != 0 ) {
				free (ni);
				return -1;
			}
			free (ni);
		}	
		i->next = 0;
		for (int a = 0; a < NUMITEMS_PERINODE; a++)
			i->inodeItem[a] = 0;
		return inodeSave(i);
	}
	return -1;
}

//Funcao que persiste um i-node em seu disco. Retorna 0 se gravacao bem sucedida
//ou -1 caso contrario. I-nodes sao salvos a partir do setor INODE_1STSECTOR. Numero de
//i-nodes por setor pode variar de acordo com o tamanho do tipo unsigned int
//Em arquiteturas de 64 bits testadas, unsigned int ocupa 32 bits. Nesse caso,
//cada setor pode receber 8 i-nodes 
int inodeSave (Inode *i) {
	if (i) {
		unsigned long int sizeUInt = sizeof(unsigned int);
		//Endereco do setor no qual o i-node sera' salvo
		unsigned long int inodeSectorAddr = 
			INODE_BEGINSECTOR + (i->number - 1) * INODE_SIZE 
			* sizeUInt / DISK_SECTORDATASIZE;
		unsigned char sector[DISK_SECTORDATASIZE];

		int ret = diskReadSector (i->d, inodeSectorAddr, sector);
		if (ret < 0) return ret;

		//Posicao de inicio do i-node dentro do setor
		unsigned long int offset = ((i->number - 1) % 
			   (DISK_SECTORDATASIZE / (INODE_SIZE * sizeUInt)))
                           * INODE_SIZE * sizeUInt;

		//Alterando enderecos de blocos e atributos do i-node no setor
		for (int a=0; a < NUMITEMS_PERINODE; a++)
			ul2char (i->inodeItem[a], 
			         &sector[offset+a*sizeUInt]);
		ul2char (i->number, 
		         &sector[offset+(INODE_SIZE-2)*sizeUInt]);
		ul2char (i->next, 
			 &sector[offset+(INODE_SIZE-1)*sizeUInt]);

		//Salvando todo o setor onde se encontra o i-node...
		ret = diskWriteSector (i->d, inodeSectorAddr, sector);
		return ret;
	}
	return -1;
}

//Funcao que recupera um i-node a partir do disco. Retorna ponteiro para o
//i-node lido ou NULL em caso de falha.
Inode* inodeLoad (unsigned int number, Disk *d) {
	unsigned long int sizeUInt = sizeof(unsigned int);
	//Endereco do setor do qual o i-node sera' lido
	unsigned long int inodeSectorAddr = 
		INODE_BEGINSECTOR + (number - 1) * INODE_SIZE * sizeUInt
		    / DISK_SECTORDATASIZE;
	unsigned char sector[DISK_SECTORDATASIZE];
	Inode *i = NULL;

	int ret = diskReadSector (d, inodeSectorAddr, sector);
	if (ret < 0) return NULL;

	//Posicao de inicio do i-node dentro do setor
	unsigned long int offset = ((number - 1) % 
		(DISK_SECTORDATASIZE / (INODE_SIZE * sizeUInt)))
		* INODE_SIZE * sizeUInt;

	i = malloc (sizeof(Inode));
	if (i) {
		i->d = d;
		//Recuperando enderecos de blocos e atributos do i-node no setor
		for (int a=0; a < NUMITEMS_PERINODE; a++)
			char2ul (&sector[offset+a*sizeUInt],
			         &(i->inodeItem[a]));
		char2ul (&sector[offset+(INODE_SIZE-2)*sizeUInt],
		         &(i->number));
		char2ul (&sector[offset+(INODE_SIZE-1)*sizeUInt],
		         &(i->next));
	}
	return i;
}

//Funcao que modifica o tipo de arquivo referente a um i-node
void inodeSetFileType (Inode *i, unsigned int fileType) {
	if (i) i->inodeItem[INODE_ITEM_FILETYPE] = fileType;
}

//Funcao que modifica o tamanho do arquivo referente a um i-node, em bytes
void inodeSetFileSize (Inode *i, unsigned int fileSize) {
	if (i) i->inodeItem[INODE_ITEM_FILESIZE] = fileSize;
}

//Funcao que modifica o proprietario do arquivo referente a um i-node
void inodeSetOwner (Inode *i, unsigned int owner) {
	if (i) i->inodeItem[INODE_ITEM_OWNER] = owner;
}

//Funcao que modifica o grupo proprietario do arquivo referente a um i-node
void inodeSetGroupOwner (Inode *i, unsigned int groupOwner) {
	if (i) i->inodeItem[INODE_ITEM_GROUPOWNER] = groupOwner;
}

//Funcao que modifica as permissoes de acesso ao arquivo referente a um i-node
void inodeSetPermission (Inode *i, unsigned int permission) {
	if (i) i->inodeItem[INODE_ITEM_PERMISSION] = permission;
}

//Funcao que modifica o contador de referencia do arquivo referente a um i-node
void inodeSetRefCount (Inode *i, unsigned int refCount) {
	if (i) i->inodeItem[INODE_ITEM_REFCOUNT] = refCount;
}

//Funcao que adiciona um endereco ao fim do array de blocos de um i-node
//Retorna -1 caso a inclusao do endereco nao seja bem sucedida
//E' a unica funcao que salva automaticamente o i-node em disco
int inodeAddBlock (Inode *i, unsigned int blockAddr) {
	if (i) {
		Disk *d = i->d;
		Inode* lastInodeExt = NULL;
		unsigned int niNumber;
		int ret, numblocks = NUMBLOCKS_PERINODE;
		lastInodeExt = __inodeGetLastExtension (i);
		if (lastInodeExt) {
			numblocks = NUMITEMS_PERINODE;
			if ( inodeSave (i) < 0 ) return -1;
		}
		else if (i->next != 0) return -1;
		else lastInodeExt = i;

		for (int a = 0; a < numblocks; a++)
			//Encontrar bloco sem endereco
			if (lastInodeExt->inodeItem[a] == 0) {
				lastInodeExt->inodeItem[a] = blockAddr;
				ret = inodeSave(lastInodeExt);
				if (numblocks != NUMBLOCKS_PERINODE) 
					free (lastInodeExt);
				return ret;
			}
		//i-node esta' sem bloco a preencher. Obter nova extensao
		niNumber = inodeFindFreeInode (lastInodeExt->number, d);
		if (niNumber) {
			lastInodeExt->next = niNumber;
			ret = inodeSave (lastInodeExt);
			if (numblocks != NUMBLOCKS_PERINODE) 
				free (lastInodeExt);
			if (ret < 0) return ret;
		}
		else {
			if (numblocks != NUMBLOCKS_PERINODE)
				free (lastInodeExt);
			return -1;
		}
		lastInodeExt = inodeLoad (niNumber, d);
		if (!lastInodeExt) return -1;
		lastInodeExt->inodeItem[0] = blockAddr;
		ret = inodeSave (lastInodeExt);
		free (lastInodeExt);
		return ret;
	}
	return -1;
}

//Funcao que retorna o numero de um i-node.
unsigned int inodeGetNumber (Inode *i) {
	return (i ? i->number : 0);
}

//Funcao que retorna o numero de um i-node.
unsigned int inodeGetNextNumber (Inode *i) {
	return (i ? i->next : 0);
}


//Funcao que retorna o tipo de arquivo referente a um i-node.
unsigned int inodeGetFileType (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_FILETYPE] : 0);
}

//Funcao que retorna o tamanho do arquivo referente ao i-node, em bytes
unsigned int inodeGetFileSize (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_FILESIZE] : 0);
}


//Funcao que retorna o prorprietario do arquivo referente a um i-node
unsigned int inodeGetOwner (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_OWNER] : 0);
}


//Funcao que retorna o grupo proprietario do arquivo referente a um i-node
unsigned int inodeGetGroupOwner (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_GROUPOWNER] : 0);
}


//Funcao que retorna as permissoes de acesso do arquivo referente a um i-node
unsigned int inodeGetPermission (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_PERMISSION] : 0);
}

//Funcao que retorna o contador de referencias do arquivo referente a um i-node
unsigned int inodeGetRefCount (Inode *i) {
	return (i ? i->inodeItem[INODE_ITEM_REFCOUNT] : 0);
}


//Funcao que retorna o endereco correspondente a um bloco (blockNum) no array
//de blocos de um i-node. O i-node precisa ser o primeiro de sua cadeia.
//Retorna 0 se o bloco nao possuir endereco em blockNum
unsigned int inodeGetBlockAddr (Inode *i, unsigned int blockNum) {
	unsigned int numblocks = NUMBLOCKS_PERINODE;
	if (i) {
		if (blockNum < NUMBLOCKS_PERINODE)
			return i->inodeItem[blockNum];
		else {
			unsigned int extNum = 1 + 
			                      (blockNum - NUMBLOCKS_PERINODE) 
			                      / NUMITEMS_PERINODE;
			unsigned int offset = (blockNum - NUMBLOCKS_PERINODE)
			                      % NUMITEMS_PERINODE;
			Inode *ni = inodeLoad (i->next, i->d);
			for (int a = 1; a < extNum; a++) {
				Disk *d = ni->d;
				unsigned int niNumber = ni->next;
				free (ni);
				ni = inodeLoad (niNumber, d);
			}
			return ni->inodeItem[offset];
		}
	}
	return 0;
}

//Funcao que encontra um i-node livre em um disco, a partir do i-node de numero
//startFrom. Retorna o numero do inode livre encontrado ou 0 se nao encontrado.
unsigned int inodeFindFreeInode (unsigned int startFrom, Disk *d) {
	Inode *i = NULL;
	unsigned int number = 0;
	if (startFrom < 1) return 0;
	for (unsigned int a = startFrom; number == 0; a++) {
		i = inodeLoad (a, d);
		if (!i) break;
		if (inodeGetBlockAddr(i, 0) == 0)
			number = inodeGetNumber(i);
		free (i);
	}
	return number;
}
