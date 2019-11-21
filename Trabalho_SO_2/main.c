/*
*  main.c - Aplicacao para simular carga sobre o VFS 
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
#include <string.h>
#include "myfs.h"
#include "vfs.h"
#include "inode.h"

#define MAX_CONNECTEDDISKS 1

#define RESULT_MSGDELAY 1000

#define NO_ID -1

//Tipo para manter dados sobre descritores de arquivos
typedef struct fd {
	int status; //Status do descritor de arquivos: 0 fechado, 1 aberto
	int type;   //Tipo do arquivo, conforme FILETYPE_* (vfs.h)
	char path[MAX_FILENAME_LENGTH+1]; //Caminho do arquivo/diretorio
} FD;

Disk *disks[MAX_CONNECTEDDISKS]; //Discos conectados ao sistema
unsigned int connectedDisks = 0; //Numero de discos conectados

Disk *rd = NULL;	//Disco montado como sistema de arquivos raiz 
int rfsid = NO_ID;	//ID do sistema de arquivo montado como raiz

FD fds[MAX_FDS];	//Status, tipo e path dos descritores de arquivo
unsigned int fdc = 0;	//Numero de descritores de arquivos abertos	

//Interface para contruir novo disco ou reconstruir disco existente (formatacao
//de baixo nivel). Para construcao de novo disco, e' previsto que o disco nao
//esteja conectado ao sistema hipotetico
void doDiskBuild() {
	char rawDiskPath[MAX_FILENAME_LENGTH+1];
	unsigned long numCylinders;
	printf ("\n>> Build: Raw disk file (e.g. 1024cyl.dsk): ");
	scanf (" %s", rawDiskPath);
	printf (">> Build: Number of cylinders (0: cancel): ");
	scanf (" %lu", &numCylinders);
	if (!numCylinders) return;
	printf ("\n-- Building... "); fflush (stdout);

	if ( diskCreateRawDisk (rawDiskPath, numCylinders) != -1 )
		printf ("Disk %s successfully (re)built\n", rawDiskPath);
	else
		printf ("\n!! Build: FAILED. No permission or not enough "
		        "free space\n");

	SLEEP (RESULT_MSGDELAY);
}


//Interface para conectar um disco existente ao sistema operacional hipotetico
void doDiskConnect(char *rawDiskPath) {
	if ( connectedDisks == MAX_CONNECTEDDISKS )
		printf ("\n!! DiskConnect: FAILED. "
		        "Maximum number of connected disks reached!\n");
	else {
		int id = -1;
		int pending = 0;
		for (int a=0; a<MAX_CONNECTEDDISKS; a++)
			if (!disks[a]) { 
				id = a;
				break;
			}
		if (!rawDiskPath) {
			rawDiskPath = malloc (sizeof (
			                      char[MAX_FILENAME_LENGTH+1]));
			pending = 1;
			printf ("\n>> DiskConnect: Raw disk file (e.g. "
			        "1024cyl.dsk): ");
			scanf (" %s", rawDiskPath);
		}
		printf ("\n-- Connecting... "); fflush (stdout);
		disks[id] = diskConnect (id, rawDiskPath);
		if (disks[id]) {
			printf ("Disk %s successfully connected\n",
			        rawDiskPath);
			connectedDisks++;
		}
		else
			printf ("\n!! DiskConnect: FAILED. No such file or "
			        "file is inaccessible/corrupted\n");
		if (pending) { 
			free (rawDiskPath);
			rawDiskPath = NULL;
		}
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para listar dados dos discos atualmente conectados ao sistema
//operacional hipotetico
void doDiskList (void) {
	if ( !connectedDisks )
		printf ("\n-- DiskList: No connected disks!\n");
	else {
		printf ("\n-- DiskList: Listing...\n");
		for (int id = 0; id<MAX_CONNECTEDDISKS; id++) {
			printf ("-- DiskID: %d; NumCylinders: %lu; "
			        "DataSize: %lu\n",
				id, diskGetNumCylinders(disks[id]),
				diskGetSize(disks[id]));
		}
	}
	SLEEP(RESULT_MSGDELAY);
}

//Interface para mostrar na saida padrao o conteudo de uma faixa de setores de
//um disco conectado ao sistema operacional hipotetico
void doDiskReadPrintSectors (void) {
	if ( !connectedDisks )
		printf ("\n!! DiskReadSector: No connected disks!\n");
	else {
		int id;
		printf ("\n>> DiskReadSector: Disk ID: ");
		scanf (" %u", &id);
		if ( id > MAX_CONNECTEDDISKS - 1 || !disks[id])
			printf ("\n!! DiskReadSector: FAILED. "
			        "Invalid identifier!\n");
		else {
			unsigned long from, to;
			unsigned long numSectors;
		       	numSectors = diskGetNumSectors(disks[id]);
			printf (">> DiskReadSector: From sector #: ");
			scanf (" %lu", &from);
			printf (">> DiskReadSector: To sector #: ");
			scanf (" %lu", &to);
			if ( from > numSectors || to < from)
				printf ("\n!! DiskReadSector: FAILED. "
				        "Invalid range!\n");
			else {
				unsigned char sector[DISK_SECTORDATASIZE]; 
				if ( to > numSectors ) to = numSectors;
				for (unsigned long a=from; a<=to; a++) {
					if ( diskReadSector (disks[id],
					                     a, sector) < 0 )
						printf ("\n!! DiskReadSector: "
						        "FAILED. Cannot read!"
						        "\n");
					else { 
						printf ("-- Sector #%lu: ", a);
						for (int b=0; 
						     b < DISK_SECTORDATASIZE;
						     b++)
							printf ("%02X", 
							        sector[b]);
						printf ("\n");
					}
				}
			}
		}
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para desconectar um disco do sistema operacional hipotetico
void doDiskDisconnect ( int id ) {
	if ( !connectedDisks )
		printf ("\n!! DiskDisconnect: FAILED. No connected disks!\n");
	else {
		if ( id == NO_ID ) {
			printf ("\n>> DiskDisconnect: Disk ID: ");
			scanf (" %u", &id);
		}
		if ( id > MAX_CONNECTEDDISKS - 1 || !disks[id])
			printf ("\n!! DiskDisconnect: FAILED. "
			        "Invalid identifier!\n");
		else if (disks[id] == rd) 
			printf ("\n!! DiskDisconnect: FAILED. Cannot "
			        "disconnect the root filesystem disk\n");
		else {
			printf ("\n-- Disconnecting... "); fflush (stdout);
			if ( diskDisconnect (disks[id]) > -1 ) {
				printf ("Disk %d successfully disconnected."
					"\n", id);
				disks[id] = NULL;
				connectedDisks--;
			}
			else
				printf ("\n!! DiskDisconnect: FAILED. Cannot "
				        "close the raw disk file!\n");
		}
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para formatar um disco conectado ao sistema operacional hipotetico,
//fazendo com que receba as estruturas de um sistema de arquivos suportado pelo
//sistema operacional hipotetico
void doFSFormat (void) {
	if ( !connectedDisks )
		printf ("\n!! DiskFormat: FAILED. No connected disks!\n");
	else {
		int id;
		printf ("\n>> DiskFormat: Disk ID: ");
		scanf (" %u", &id);
		if ( id > MAX_CONNECTEDDISKS - 1 || !disks[id])
			printf ("\n!! DiskFormat: FAILED. "
			        "Invalid identifier!\n");
		else if (disks[id] == rd) 
			printf ("\n!! DiskFormat: FAILED. "
			        "Cannot format the root filesystem disk\n");
		else {
			int fsid, bs;
			printf (">> DiskFormat: Filesystem ID: ");
			scanf (" %u", &fsid);
			printf (">> DiskFormat: Block size in # of sectors "
			        "(0: cancel): ");
			scanf (" %u", &bs);
			if (!bs) return;
			bs = bs * DISK_SECTORDATASIZE;
			printf ("\n-- Formatting... "); fflush (stdout);
			if ( vfsFormat (disks[id], bs, fsid) > -1 )
				printf ("Disk %d successfully formatted.\n",
				       	id);
			else
				printf ("\n!! DiskFormat: FAILED. Filesystem "
				        "not supported or operation "
				        "failed!\n");
		}
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para montar um disco conectado ao sistema operacional hipotetico,
//para atuar como sistema de arquivos raiz
void doFSMountRoot (void) {
	if ( !connectedDisks )
		printf ("\n!! MountRoot: FAILED. No connected disks!\n");
	else if ( rd )
		printf ("\n!! MountRoot: FAILED. Root filesystem already "
		        "mounted!\n");
	else {
		int id;
		printf ("\n>> MountRoot: Disk ID: ");
		scanf (" %u", &id);
		if ( id > MAX_CONNECTEDDISKS - 1 || !disks[id])
			printf ("\n!! MountRoot: FAILED. "
			        "Invalid identifier!\n");
		else {
			int fsid;
			printf (">> MountRoot: Filesystem ID: ");
			scanf (" %u", &fsid);
			printf ("\n-- Mounting... "); fflush (stdout);
			if ( vfsMountRoot (disks[id], fsid) > -1 ) {
				printf ("Disk %d successfully mounted as "
				        "root filesystem.\n", id);
				rd = disks[id];
				rfsid = fsid;
			}
			else
				printf ("\n!! MountRoot: FAILED. Filesystem "
				        "not supported or operation "
				        "failed!\n");
		}
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para mostrar os dados sobre descritores de arquivo em uso no
//sistema operacional hipotetico
void doFSShowFDs (void) {
	if ( !rd )
		printf ("\n!! ShowFDs: FAILED. No root filesystem mounted!\n");
	else {
		if (!fdc)	
			printf ("\n!! ShowFDs: No file descriptors in use!\n");
		else {
			printf("\n-- ShowFDs: Showing...\n");
			for (int fd = 1; fd<=MAX_FDS; fd++)
				if ( fds[fd-1].status ) {
					printf ("-- FD: %3d;   "
					        "Type: %3d;   Path: %s\n",
					        fd, fds[fd-1].type,
					        fds[fd-1].path);
				}
		}
	}
	SLEEP(RESULT_MSGDELAY);
}


//Interface para desmontar o atual sistema de arquivos raiz
void doFSUnmountRoot (void) {
	if ( !rd )
		printf ("\n!! UnmountRoot: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		printf ("\n-- Unmounting... "); fflush (stdout);
		if ( vfsUnmountRoot () > -1 ) {
			printf ("Disk %d successfully unmounted as root file"
			        "system.\n", diskGetId(rd));
			rd = NULL;
			rfsid = NO_ID;
		}
		else
			printf ("\n!! UnmountRoot: FAILED. Root file"
				"system is busy or operation failed!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para abrir um arquivo, criando-o se nao existir, em modo
//leitura/escrita
void doFileOpen (void) {
	if ( !rd )
		printf ("\n!! FileOpen: FAILED. No root filesystem "
		        "mounted!\n");
	else if (fdc == MAX_FDS)
		printf ("\n!! FileOpen: FAILED. Maximum number of file "
		        "descriptors reached!\n");
	else {
		char filePath[MAX_FILENAME_LENGTH+1];
		int fd;
		printf ("\n>> FileOpen: File path (e.g. /home/moreno/doc1): ");
		scanf (" %s", filePath);
		printf ("\n-- Opening... "); fflush (stdout);
		fd = vfsOpen(filePath);
	       	if ( fd > 0 ) {
			printf ("File %s successfully opened as FD %d.\n",
			        filePath, fd);
			fds[fd-1].status = 1;
			fds[fd-1].type = FILETYPE_REGULAR;
			strcpy (fds[fd-1].path, filePath);
			fdc++;
		}
		else
			printf ("\n!! FileOpen: FAILED. Invalid path or"
				"no blocks/i-nodes available!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para ler e imprimir bytes de um arquivo aberto
void doFileReadPrint (void) {
	if ( !rd )
		printf ("\n!! FileRead: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		int fd, rbytes;
		unsigned int nbytes;
		char *buffer;
		printf ("\n>> FileRead: File descriptor (#): ");
		scanf (" %u", &fd);
		printf (">> FileRead: Number of bytes (e.g. 512): ");
		scanf (" %u", &nbytes);
		printf ("\n-- Reading... "); fflush (stdout);
		buffer = malloc (sizeof(unsigned char[nbytes]));
		rbytes = vfsRead (fd, buffer, nbytes);
		if ( rbytes > -1 ) {
			printf ("File %s successfully read. %d bytes read.\n",
			        fds[fd-1].path, rbytes);
			for (int a=0; a<rbytes; a++)
				printf ("%c",buffer[a]);
			if (rbytes > 0) printf ("\n");
		}
		else
			printf ("\n!! FileRead: FAILED. Invalid file "
			        "descriptor or i-node saving failure!\n");
		free (buffer);
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para escrever bytes de um arquivo aberto
void doFileWrite (void) {
	if ( !rd )
		printf ("\n!! FileWrite: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		int fd, wbytes;
		unsigned int nbytes;
		char *buffer;
		printf ("\n>> FileWrite: File descriptor (#): ");
		scanf (" %u", &fd);
		printf (">> FileWrite: Number of bytes: ");
		scanf (" %u", &nbytes);
		printf ("\n-- Writing... "); fflush (stdout);
		buffer = malloc (sizeof(unsigned char[nbytes]));
		if ( fd <= MAX_FDS && fds[fd-1].status ) {
			int pathlen = strlen(fds[fd-1].path);
			for (int a=0; a<nbytes; a++)
				buffer[a] = fds[fd-1].path[a%pathlen];
		}
		wbytes = vfsWrite (fd, buffer, nbytes);
		if ( wbytes > -1 )
			printf ("File %s successfully wrote. %d bytes "
			        "wrote.\n", fds[fd-1].path, wbytes);
		else
			printf ("\n!! FileWrite: FAILED. Invalid file "
			        "descriptor or i-node saving failure!\n");
		free (buffer);
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para fechar um arquivo aberto
void doFileClose (int fd) {
	if ( !rd )
		printf ("\n!! FileClose: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		if ( fd == NO_ID ) {
			printf ("\n>> FileClose: File descriptor (#): ");
			scanf (" %u", &fd);
		}
		printf ("\n-- Closing... "); fflush (stdout);
		if ( vfsClose(fd) > -1 ) {
			printf ("File %s successfully closed.\n", 
			        fds[fd-1].path);
			fds[fd-1].status = 0;
			fds[fd-1].type = 0;
			strcpy (fds[fd-1].path, "");
			fdc--;
		}
		else
			printf ("\n!! FileClose: FAILED. Invalid file "
			        "descriptor or i-node saving failure!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para abrir um diretorio, criando-o se nao existir
void doDirOpen (void) {
	if ( !rd )
		printf ("\n!! DirOpen: FAILED. No root filesystem "
		        "mounted!\n");
	else if (fdc == MAX_FDS)
		printf ("\n!! DirOpen: FAILED. Maximum number of file "
		        "descriptors reached!\n");
	else {
		char dirPath[MAX_FILENAME_LENGTH+1];
		int fd;
		printf ("\n>> DirOpen: Directory path (e.g. /home/moreno/): ");
		scanf (" %s", dirPath);
		printf ("\n-- Opening... "); fflush (stdout);
		fd = vfsOpendir(dirPath);
	       	if ( fd > 0 ) {
			printf ("Directory %s successfully opened as FD %d.\n",
			       	dirPath, fd);
			fds[fd-1].status = 1;
			fds[fd-1].type = FILETYPE_DIR;
			strcpy (fds[fd-1].path, dirPath);
			fdc++;
		}
		else
			printf ("\n!! DirOpen: FAILED. Invalid path or"
				"no blocks/i-nodes available!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para ler e listar entradas de um diretorio aberto
void doDirList (void) {
	if ( !rd )
		printf ("\n!! DirList: FAILED. No root filesystem mounted!\n");
	else {
		int fd, res;
		char entryname[MAX_FILENAME_LENGTH+1];
		unsigned int inumber;
		printf ("\n>> DirList: Directory descriptor (#): ");
		scanf (" %u", &fd);
		printf ("\n-- DirList: Listing...\n"); fflush (stdout);
		res = vfsReaddir (fd, entryname, &inumber);
		while ( res > 0 ) {
			printf ("-- Inode #: %5d     Name: %s\n",
			        inumber, entryname);
			res = vfsReaddir (fd, entryname, &inumber);
		}
		if ( res == -1 )
			printf ("\n!! DirList: FAILED. Invalid file "
			        "descriptor or i-node saving failure!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para adicionar uma entrada (Link) em um diretorio aberto
void doDirLink (void) {
	if ( !rd )
		printf ("\n!! DirLink: FAILED. No root filesystem mounted!\n");
	else {
		int fd;
		unsigned int inumber;
		char entryname[MAX_FILENAME_LENGTH+1];
		printf ("\n>> DirLink: Directory descriptor (#): ");
		scanf (" %u", &fd);
		printf (">> DirLink: New entry name (e.g. doc1): ");
		scanf (" %s", entryname);
		printf (">> DirLink: I-node number to be linked "
		        "(0: cancel): ");
		scanf (" %u", &inumber);
		printf ("\n-- Linking... "); fflush (stdout);
		if ( vfsLink (fd, entryname, inumber) == 0 )
			printf ("Entry %s successfully linked to i-node %d "
			        "in directory %s\n",
			        entryname, inumber, fds[fd-1].path);
		else
			printf ("\n!! DirLink: FAILED. Invalid file "
			        "descriptor or i-node!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}

//Interface para remover uma entrada (Unlink) de um diretorio aberto
void doDirUnlink (void) {
	if ( !rd )
		printf ("\n!! DirUnlink: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		int fd;
		char entryname[MAX_FILENAME_LENGTH+1];
		printf ("\n>> DirUnlink: Directory descriptor (#): ");
		scanf (" %u", &fd);
		printf (">> DirUnlink: Entry name (e.g. doc1): ");
		scanf (" %s", entryname);
		printf ("\n-- Unlinking... "); fflush (stdout);
		if ( vfsUnlink (fd, entryname) == 0 )
			printf ("Entry %s successfully unlinked from "
			        "directory %s\n", entryname, fds[fd-1].path);
		else
			printf ("\n!! DirUnlink: FAILED. Invalid file "
			        "descriptor or entry name!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}


//Interface para fechar um arquivo aberto
void doDirClose (int fd) {
	if ( !rd )
		printf ("\n!! DirClose: FAILED. No root filesystem "
		        "mounted!\n");
	else {
		if ( fd == NO_ID ) {
			printf ("\n>> DirClose: File descriptor (#): ");
			scanf (" %u", &fd);
		}
		printf ("\n-- Closing... "); fflush (stdout);
		if ( vfsClosedir(fd) > -1 ) {
			printf ("Directory %s successfully closed.\n",
			        fds[fd-1].path);
			fds[fd-1].status = 0;
			fds[fd-1].type = 0;
			strcpy (fds[fd-1].path, "");
			fdc--;
		}
		else
			printf ("\n!! DirClose: FAILED. Invalid file "
			        "descriptor or i-node saving failure!\n");
	}
	SLEEP (RESULT_MSGDELAY);
}


//Trabalho necessario para um encerramento suave do sistema operacional
//hipotetico, fechando descritores de arquivos, desmontando sistemas de
//arquivos e desconectando discos
char sanitizeBeforeQuit () {
	//Fechando arquivos/diretorios abertos
	if (fdc)
		for (int a=1; a<=MAX_FDS; a++)
			if ( fds[a-1].status ) {
				if ( fds[a-1].type == FILETYPE_REGULAR )
					doFileClose(a);
				else doDirClose(a);
			}
	//Desmontando a raiz do sistema de arquivos
	if (rd) doFSUnmountRoot();

	//Desconectando discos
	if (connectedDisks)
		for (int a=0; a<MAX_CONNECTEDDISKS; a++)
			if (disks[a]) doDiskDisconnect(a);

	if (rd || connectedDisks)
		return ' ';

	return 'q';
}

//Interface para o menu de selecao de operacoes sobre diretorios
void dirMenuSelection (void) {
	char choice = ' ';
	while ( choice != '<' ) {
		printf ("\nDIRECTORY operations:                   "
			  "               Disks: %u / Root Disk: %d\n"
			  "     [O]pen directory\n"
		          "     [L]ist directory entries\n"
		          "     [A]dd a new entry to directory (Link)\n"
			  "     [R]emove an entry from directory (Unlink)\n"
		          "     [C]lose directory\n"
		          "     [<]back to MAIN menu\n"
		          "\n>> Your selection: ", connectedDisks,
			  (rd ? diskGetId(rd) : -1));
		scanf (" %c", &choice);
		switch (choice) {
			case 'O': case 'o': doDirOpen(); break;
			case 'L': case 'l': doDirList(); break;
			case 'A': case 'a': doDirLink(); break;
			case 'R': case 'r': doDirUnlink(); break;
			case 'C': case 'c': doDirClose(NO_ID); break;
		}
	}
}


//Interface para o menu de selecao de operacoes sobre arquivos regulares
void fileMenuSelection (void) {
	char choice = ' ';
	while ( choice != '<' ) {
		printf ("\nFILE operations:                        "
			  "               Disks: %u / Root Disk: %d\n"
			  "     [O]pen file\n"
		          "     [R]ead bytes from file\n"
		          "     [W]rite bytes to file\n"
			  "     [C]lose file\n"
		          "     [<]back to MAIN menu\n"
		          "\n>> Your selection: ", connectedDisks,
			  (rd ? diskGetId(rd) : -1));
		scanf (" %c", &choice);
		switch (choice) {
			case 'O': case 'o': doFileOpen(); break;
			case 'R': case 'r': doFileReadPrint(); break;
			case 'W': case 'w': doFileWrite(); break;
			case 'C': case 'c': doFileClose(NO_ID); break;
		}
	}
}

//Interface para o menu de selecao de operacoes de gerenciamento de um
//sistema de arquivos
void fsMenuSelection (void) {
	char choice = ' ';
	while ( choice != '<' ) {
		printf ("\nFILESYSTEM management:                  "
			  "               Disks: %u / Root Disk: %d\n"
			  "     [L]ist supported filesystems\n"
		          "     [F]ormat a disk (high-level format)\n"
		          "     [M]ount root filesystem\n"
		          "     [S]how file descriptors in use\n"
			  "     [U]mount root filesystem\n"
		          "     [<]back to MAIN menu\n"
		          "\n>> Your selection: ", connectedDisks,
			  (rd ? diskGetId(rd) : -1));
		scanf (" %c", &choice);
		switch (choice) {
			case 'L': case 'l': vfsDumpFSInfo();
			                    SLEEP(RESULT_MSGDELAY);
					    break;
			case 'F': case 'f': doFSFormat(); break;
			case 'M': case 'm': doFSMountRoot(); break;
			case 'S': case 's': doFSShowFDs(); break;
			case 'U': case 'u': doFSUnmountRoot(); break;
		}
	}
}

//Interface para o o menu de selecao de operacoes de gerenciamento de um disco
void diskMenuSelection (void) {
	char choice = ' ';
	while ( choice != '<' ) {
		printf ("\nDISK operations:                        "
			  "               Disks: %u / Root Disk: %d\n"
		          "     [B]uild/rebuild a disk (Low-level format)\n"
		          "     [C]onnect a disk\n"
			  "     [L]ist connected disks\n"
			  "     [R]ead/print sector range from a disk\n"
		          "     [D]isconnect a disk\n"
		          "     [<]back to MAIN menu\n"
		          "\n>> Your selection: ", connectedDisks,
			  (rd ? diskGetId(rd) : -1));
;
		scanf (" %c", &choice);
		switch (choice) {
			case 'B': case 'b': doDiskBuild(); break;
			case 'C': case 'c': doDiskConnect(NULL); break;
			case 'L': case 'l': doDiskList(); break;
			case 'R': case 'r': doDiskReadPrintSectors(); break;
			case 'D': case 'd': doDiskDisconnect(NO_ID); break;
		}
	}
}

//Interface para o menu de selecao principal
void mainMenuSelection (void) {
	char choice = ' ';
	while ( choice!='Q' && choice!='q' ) {
		printf ("\nMAIN Menu:                              "
		         "               Disks: %u / Root Disk: %d\n"
		         "     [D]isk operations\n"
		         "     [F]ile system management\n"
			 "    f[I]le operations\n"
		         "   di[R]ectory operations\n"
		         "     [Q]uit\n"
		         "\n>> Your selection: ", connectedDisks,
			  (rd ? diskGetId(rd) : -1));

		scanf(" %c", &choice);
		switch (choice) {
			case 'D': case 'd': diskMenuSelection(); break;
			case 'F': case 'f': fsMenuSelection(); break;
			case 'I': case 'i': fileMenuSelection(); break;
			case 'R': case 'r': dirMenuSelection(); break;
			case 'Q': case 'q': choice = sanitizeBeforeQuit();
					    break; 
		}
	}
}

int main (int argc, char* argv[]) {

	installMyFS();

	for (int a=0; a<MAX_CONNECTEDDISKS; a++)
		disks[a] = NULL;
	for (int a=1; a<=MAX_FDS; a++) {
		fds[a-1].status = 0;
		fds[a-1].type = 0;
		strcpy (fds[a-1].path, "");
	}

	if (argc > 1) 
		doDiskConnect (argv[1]);

	mainMenuSelection();

	printf ("\n");

	return EXIT_SUCCESS;

}
