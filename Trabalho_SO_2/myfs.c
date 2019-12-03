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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "myfs.h"
#include "vfs.h"
#include "inode.h"
#include "util.h"

#define NUM_METADATA_ITEM 9
#define INODE_BEGINSECTOR 2
#define INODE_ENDSECTOR 18
#define FREE_SPACE_MANAGEMENT_BEGINSECTOR 19
#define FREE_SPACE_MANAGEMENT_ENDSECTOR 35
#define ROOT_DIRECTORY_BEGINSECTOR 36
#define ROOT_DIRECTORY_ENDSECTOR 52
#define DATA_BEGINSECTOR 53
#define POSITION_LAST_BYTE_IN_ROOT 10

/*
 * Estrutura do setor de metadados:
 * INODE_BEGIN | INODE_END | FREE_SPACE_MANAGMENT_BEGIN | FREE_SPACE_MANAGMENT_END
 */

//Declaracoes globais
char fsid = 0;  // Identificador do tipo de sistema de arquivos
char *fsname = "myFS"; // Nome do tipo de sistema de arquivos

int myFSslot;
unsigned int last_byte_in_root = 0;     // ultimo byte dentro dos setores do diretorio raiz
unsigned int sector_last_byte_in_root = 0;     // setor do ultimo byte dentro dos setores do diretorio raiz
unsigned int block_size = 0;

struct filedescriptor
{
  int status;
  int type;
  unsigned int fd;
  unsigned int pointer;
  Disk *disk;
  Inode *inode;
  char *path;
};
typedef struct filedescriptor FileDescriptor;
FileDescriptor *fileDescriptors[MAX_FDS];



// Funcao para salvar os caracteres de "from" em "saveTo" a partir de offsetTo e offsetFrom
int saveChar(unsigned char *saveTo, unsigned char *from, int offsetTo, int offsetFrom, int numBytes){
    int lengthOfArray = (int)(sizeof(from) / sizeof(from[0]));
    int j = offsetTo;
    for(int i = 0; i < numBytes; i++){
        saveTo[j] = from[offsetFrom + i];
        j++;
    }
    return 0;
}


int getNextFile(Disk *d, int *sector_pointer){
    int aux = *sector_pointer;
    *sector_pointer = (*sector_pointer) + 17;
    aux = aux + 17;
    return 0;
    /*
    int aux = *pointer;
    aux = aux + file_total_size;
    if(aux > DISK_SECTORDATASIZE){
        *current_sector  = (*current_sector) + 1;
        diskReadSector(d, *current_sector, second_sector);
        if(*current_sector < number_of_sectors)
            return 1;
        return 0;
    }
    */
}

int getEmptyFileDescriptor(Disk *d, char *path, char *file){
    for(int i = 0; i < MAX_FDS; i++){
        if (fileDescriptors[i] == NULL){
            Inode *inode = NULL;
            unsigned int inode_num = 0;
            unsigned char aux[4];
            saveChar(aux, file, 0, 0, 4);
            char2ul(aux, &inode_num);
            
            inode = inodeLoad(inode_num, d);
            
            fileDescriptors[i] = malloc(sizeof(FileDescriptor));
            fileDescriptors[i]->disk = d;
            fileDescriptors[i]->fd = i;
            fileDescriptors[i]->inode = inode;
            fileDescriptors[i]->path = path;
            fileDescriptors[i]->status = 1;
            fileDescriptors[i]->type = FILETYPE_REGULAR;
            
            return i;
        }
    }
    return -1;
}

void clearDisk(Disk *d){
    unsigned char emptySector[DISK_SECTORDATASIZE];
    for(int i = 0; i < DISK_SECTORDATASIZE; i++){
        emptySector[i] = ' ';
    }
    unsigned long numSectors = diskGetNumSectors(d);
    for(unsigned long i = 0; i < numSectors; i++){
        diskWriteSector(d, i, emptySector);
    }
}

unsigned int getLastByteInRoot(Disk *d){
    unsigned char sector[DISK_SECTORDATASIZE];
    unsigned char aux[4];
    unsigned int lastByte = 0;
    if(diskReadSector(d, 1, sector) == 0){
        int offset = POSITION_LAST_BYTE_IN_ROOT * 4;
        saveChar(aux, sector, 0, offset, 4);
        char2ul(aux, &lastByte);
        printf("\nPosicao: %u\n", lastByte);
    }
    return lastByte;
}

int findFile(Disk *d, char *path, char *file){
    unsigned int file_total_size = MAX_FILENAME_LENGTH + 5;     // bytes do nome + um unsigned int pra dizer qual Inode + 1 byte de \n no final
    int sector_pointer = 4;                                     // ponteiro pra percorrer o setor
    unsigned int number_of_sectors = ROOT_DIRECTORY_ENDSECTOR - ROOT_DIRECTORY_BEGINSECTOR + 1;
    unsigned int current_sector = 0;                            // setor atual
    unsigned int begin_file = 0;                                // posicao que comeca o nome do arquivo (4 primeiros sao bytes do Inode)
    
    unsigned char sector[DISK_SECTORDATASIZE];
    unsigned char sector_aux[DISK_SECTORDATASIZE];
    diskReadSector(d, ROOT_DIRECTORY_BEGINSECTOR + current_sector, sector);
    bool found = true;
    bool other_sector = false;
    unsigned int last_byte_root = getLastByteInRoot(d);
    
    while(begin_file < last_byte_root && current_sector < number_of_sectors){
        int path_pointer = 0;
//        for(int i = 0; i < (file_total_size - 5); i++){
        found = true;
        if(path[path_pointer] != sector[sector_pointer]){
            int result = getNextFile(d, &begin_file);
            if(result == 0){
                sector_pointer = begin_file + 4;
                if(sector_pointer >= DISK_SECTORDATASIZE){
                    strncpy(sector_aux, sector, DISK_SECTORDATASIZE);
                    diskReadSector(d, ROOT_DIRECTORY_BEGINSECTOR + current_sector, sector);
                    //int offset = DISK_SECTORDATASIZE - sector_pointer;
                    int offset = (DISK_SECTORDATASIZE - begin_file) - 1;
                    sector_pointer = offset;
                    current_sector += 1;
                    other_sector = true;
                }
                else{
                    other_sector = false;
                }
            }
            found = false;
            path_pointer = -1;
        }
        path_pointer++;
//        i = -1;
//        }
    }
    return -1;
}

int createNewFile(Disk *d, char *path){
    unsigned int inode_num = inodeFindFreeInode(0, d);
    Inode *i = inodeCreate(inode_num, d);
    
    char file[17];
    char aux[4];
    ul2char(inode_num, aux);
    saveChar(file, aux, 0, 0, 4);
    saveChar(file, path, 4, 0, 8);
    file[16] = '\n';
    
    
    
    unsigned char setor[DISK_SECTORDATASIZE];
    diskReadSector(d, sector_last_byte_in_root + ROOT_DIRECTORY_BEGINSECTOR, setor);
    
    saveChar(setor, file, last_byte_in_root, 0, 17);
    
    /// Verificação se cabe em 1 setor, se nao, salvar em dois, se couber apenas salvar
}





//void teste(unsigned char metadataSector){
//    unsigned char uiAux[4];                // Vetor para armazenar os valores de unsigned int
//    unsigned int aux = 0;
//    int offsetCounter = 0;
//    ul2char(NUM_METADATA_ITEM, &uiAux);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(INODE_BEGINSECTOR, &uiAux);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(INODE_ENDSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(FREE_SPACE_MANAGEMENT_BEGINSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(FREE_SPACE_MANAGEMENT_ENDSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(ROOT_DIRECTORY_BEGINSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(ROOT_DIRECTORY_ENDSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//    ul2char(DATA_BEGINSECTOR, &uiAux[0]);
//    offsetCounter = saveChar(uiAux, metadataSector, offsetCounter, 4);
//    char2ul(uiAux, &aux);
//    printf("%u ", aux);
//}

// Função auxliar para escrever todos o metadados num vetor de caracteres
unsigned int createMetadataSector(Disk *d, unsigned char *metadataSector, unsigned int blockSize){
    unsigned char uiAux[4];                // Vetor para armazenar os valores de unsigned int
    unsigned int numBlocks = (diskGetNumSectors(d) - ROOT_DIRECTORY_ENDSECTOR)/blockSize;
    int offsetCounter = 0;
    ul2char(NUM_METADATA_ITEM, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(INODE_BEGINSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(INODE_ENDSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(FREE_SPACE_MANAGEMENT_BEGINSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(FREE_SPACE_MANAGEMENT_ENDSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(ROOT_DIRECTORY_BEGINSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(ROOT_DIRECTORY_ENDSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(DATA_BEGINSECTOR, uiAux);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(blockSize, &uiAux[0]);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(numBlocks, &uiAux[0]);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    offsetCounter += 4;
    ul2char(0, &uiAux[0]);
    saveChar(metadataSector, uiAux, offsetCounter, 0, 4);
    return numBlocks;
    
}

//Funcao para verificacao se o sistema de arquivos está ocioso, ou seja,
//se nao ha quisquer descritores de arquivos em uso atualmente. Retorna
//um positivo se ocioso ou, caso contrario, 0.
int myFSIsIdle (Disk *d) {
    for(int i = 0; i < MAX_FDS; i++){
        if (fileDescriptors[i] != NULL && diskGetId(d) == diskGetId(fileDescriptors[i]->disk)){
            return 0;
        }
    }
    return 1;
}

//Funcao para formatacao de um disco com o novo sistema de arquivos
//com tamanho de blocos igual a blockSize. Retorna o numero total de
//blocos disponiveis no disco, se formatado com sucesso. Caso contrario,
//retorna -1.
int myFSFormat (Disk *d, unsigned int blockSize) {
    clearDisk(d);
    block_size = blockSize;
    unsigned char metadataSector[DISK_SECTORDATASIZE];
    unsigned int numBlocks = createMetadataSector(d, metadataSector, blockSize);
    if(diskWriteSector(d, 1, metadataSector) == 0){
        return numBlocks;
    }
    return -1;
}

//Funcao para abertura de um arquivo, a partir do caminho especificado
//em path, no disco montado especificado em d, no modo Read/Write,
//criando o arquivo se nao existir. Retorna um descritor de arquivo,
//em caso de sucesso. Retorna -1, caso contrario.
int myFSOpen (Disk *d, const char *path) {
    int fd = -1;
    char file[17];
    if(findFile(d, *path, file) == 0){
        fd = getEmptyFileDescriptor(d, path, file);
        if(fd != -1){
            printf("\nErro ao criar File descriptor\n");
            return -1;
        }
    }
    else{
        fd = createNewFile(d, *path);
    }
    return -1;
}
	
//Funcao para a leitura de um arquivo, a partir de um descritor de
//arquivo existente. Os dados lidos sao copiados para buf e terao
//tamanho maximo de nbytes. Retorna o numero de bytes efetivamente
//lidos em caso de sucesso ou -1, caso contrario.
int myFSRead (int fd,unsigned char *buf, unsigned int nbytes) {

    struct inode *inodeaux;

    struct disk *discoaux;
    unsigned char* bufferAux;
    int totalBytes=0;
    int x=0;
    unsigned int bufferAuxINT;
    unsigned int enderecoBloco;
    int leituraSetorDisco;

    if(fd>MAX_FDS){
        printf("ID FORA DO RANGE");
        return -1;
    }
    else{

        if(files[fd].status==0){
            printf("arquivo fechado");
            return -1;
        }

        else{
                //variavel auxiliar para pegar o inode e o disco
                //Precisa de um loop para percorrer todos inodes(se tiver mais de 1 inode)
//                discoaux=files[fd].disk;
//                inodeaux=files[fd].inode;
                //percorre todos blocos do inode
                //para cada bloco, o inode vai achar seu setor de dados e atribui ao buffer
                for(int i=0; i<8; i++)
                {
                    enderecoBloco=inodeGetBlockAddr(inodeaux,i);
//                    setor=
//                    leituraSetorDisco=diskReadSector(discoaux,setor,bufferAux);
                    char2ul(bufferAux,bufferAuxINT);
                    //se o total de bytes estiver no maximo, entao ja chegou no limite
                    //caso contrario o total soma com mais dados.
                    if(totalBytes>=nbytes){
                        x=bufferAuxINT;
                        break;
                    }
//                    total+=bufferAuxINT;

                }
        }

    }
    if(x>0){
        if(totalBytes>nbytes)
        totalBytes=nbytes-totalBytes;

        printf("Mais bytes que o limite");
    }
    if(totalBytes<nbytes){
        printf("menos bytes que o limite");
    }
    //verifica se esta aberto
    //verificar tamanho do id (dentro do range)
    //verifica se possui essa quantidade de byte. para mais ou menos
    //ao entrar no descritor e estiver aberto, para caida inode vou percorrer seus blocos e para cada bloco percorrido vou ler os setores(cada setor tem 512 bytes)
        //o setor de dados sera o ultimo
	return totalBytes;
}

//Funcao para a escrita de um arquivo, a partir de um descritor de
//arquivo existente. Os dados de buf serao copiados para o disco e
//terao tamanho maximo de nbytes. Retorna o numero de bytes
//efetivamente escritos em caso de sucesso ou -1, caso contrario
int myFSWrite (int fd, const char *buf, unsigned int nbytes) {
    if(fileDescriptors[fd] == NULL || fileDescriptors[fd]->status == 0){
        printf("O arquivo não está aberto\n");
        return -1;
    }
    unsigned int writtenBytes = 0;
    Inode *i = fileDescriptors[fd]->inode;
    unsigned int current_buf_pointer = 0;
    if(i != NULL){
        while(writtenBytes < nbytes){
            
        }
    }
    
}

//Funcao para fechar um arquivo, a partir de um descritor de arquivo
//existente. Retorna 0 caso bem sucedido, ou -1 caso contrario
int myFSClose (int fd) {
    if(fileDescriptors[fd] == NULL || fileDescriptors[fd]->status == 0){
        printf("O descritor de arquivo já está fechado\n");
        return -1;
    }
    fileDescriptors[fd]->status = 0;
    fileDescriptors[fd]->disk == NULL;
    fileDescriptors[fd]->inode == NULL;
    free(fileDescriptors[fd]);
    return 0;
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



