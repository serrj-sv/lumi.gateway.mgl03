/*
 *      Tool to convert ELF image to be the AP downloadable binary
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: cvimg.c,v 1.4 2009/06/12 07:10:44 michael Exp $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#undef HOME_GATEWAY
#define HOME_GATEWAY
#define RTK_4K_CHKSUM_FIX
#define CONFIG_RTL_819XD
#define COMPACT_FILENAME_BUFFER

#include "apmib.h"

/* 32-bit ELF base types. */
typedef unsigned int	Elf32_Addr;
typedef unsigned short	Elf32_Half;
typedef unsigned int	Elf32_Off;
typedef unsigned int	Elf32_Word;

#define EI_NIDENT	16

typedef struct elf32_hdr{
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;  /* Entry point */
  Elf32_Off	e_phoff;
  Elf32_Off	e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

#define DEFAULT_START_ADDR	0x80500000
#define DEFAULT_BASE_ADDR	0x80000000

static unsigned short calculateChecksum(char *buf, int len);

/////////////////////////////////////////////////////////
static unsigned int calculate_long_checksum(unsigned int *buf, int len)
{
	int i, j;
	unsigned int sum=0, tmp;

	j = len/4;

	for (i=0; i<j; buf++, i++) {
		tmp = *buf;
		sum += DWORD_SWAP(tmp);
	}
	return ~sum+1;
}

unsigned int extractStartAddr(char *filename)
{
    int fh;
    Elf32_Ehdr hdr;
    char *buf;

    buf = (char *)&hdr;
    fh = open(filename, O_RDONLY);
	if ( fh == -1 ) {
		printf("Open input file error2!\n");
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		printf("Read file error2!\n");
		close(fh);
		exit(1);
	}
	close(fh);

	return(hdr.e_entry);
}

void printf_usage(void)
{
    printf("Version: 1.1\n");
    printf("Usage: cvimg <option> input-filename output-filename start-addr burn-addr [signature]\n");
    printf("<option>: root|linux|linux-ro|boot|all|vmlinux|vmlinuxhdr|signature\n");
    printf("[signature]: user-specified signature (4 characters)\n");
}

#ifdef RTK_4K_CHKSUM_FIX
#define JFFS2_ENDMARK_LEN 4
static int rtk_fix_4k_chk(char *inputFile,char *outFile)
{
	struct stat status;
	char *buf;
	int fh, size;
	IMG_HEADER_Tp pHeader;
	unsigned short checksum=0;
	int old_linx_len=0;

	if ( stat(inputFile, &status) < 0 ) {
		printf("Can't stat file! [%s]\n", inputFile );
		exit(1);
	}

	//decide output size , current , stat+(checksum)
	size= status.st_size ;

	buf = malloc(size);
	if (buf == NULL) {
		printf("Malloc buffer failed!\n");
		exit(1);
	}
	memset(buf, '\0', size);

	pHeader = (IMG_HEADER_Tp)buf;

	fh = open(inputFile, O_RDONLY);
	if ( fh == -1 ) {
		printf("Open input file error!\n");
		free( pHeader );
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);

	if ( read(fh, buf, status.st_size) != status.st_size) {
		printf("Read file error!\n");
		close(fh);
		free(pHeader);
		exit(1);
	}
	close(fh);

	//update  len(not include jffs2 endmark)
	old_linx_len = DWORD_SWAP(pHeader->len);
	//printf("old_linx_len = %8x\n !!",old_linx_len);
	pHeader->len = DWORD_SWAP((size-sizeof(IMG_HEADER_T)-JFFS2_ENDMARK_LEN));

	//erase old chksum
	//printf("old_cksum = %8x\n !!",*((unsigned short *)&buf[sizeof(IMG_HEADER_T)+old_linx_len-sizeof(checksum)]));
	*((unsigned short *)&buf[sizeof(IMG_HEADER_T)+old_linx_len-sizeof(checksum)]) = 0;

	//add new chksum
	checksum = WORD_SWAP(calculateChecksum(buf+sizeof(IMG_HEADER_T), (size-sizeof(IMG_HEADER_T)-JFFS2_ENDMARK_LEN)));
	//*((unsigned short *)&buf[status.st_size]) = checksum;
	*((unsigned short *)&buf[sizeof(IMG_HEADER_T)+old_linx_len-sizeof(checksum)]) = checksum;

	//write to new file
	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU); //mark_88
	if ( fh == -1 ) {
		printf("Create output file error! [%s]\n", outFile);
		free(pHeader);
		exit(1);
	}
	write(fh, pHeader, size);
	close(fh);
	chmod(outFile, DEFFILEMODE);

	printf("Generate 4k alignment image(with jffs2 end mark)successfully, length=%d, checksum=0x%x\n", (int)DWORD_SWAP(pHeader->len), checksum);

	free(pHeader);

	return 0;
}
#endif

int main(int argc, char** argv)
{
#ifdef COMPACT_FILENAME_BUFFER
	const char *inFile = NULL, *outFile = NULL;
#else
	char inFile[80]={0}, outFile[80]={0};
#endif
	int fh, size;
	struct stat status;
	char *buf;
	IMG_HEADER_Tp pHeader;
	unsigned int startAddr;
	unsigned int burnAddr;
	unsigned short checksum=0;
	int is_vmlinux = 0;
	int is_vmlinuxhdr = 0;
    int is_signature = 0;
	unsigned int lchecksum, padding_len=0;
	unsigned int start_addr=0;

	if (argc == 4 && !strcmp(argv[1], "size_chk")) {
		unsigned int total_size;

#ifdef COMPACT_FILENAME_BUFFER
		inFile = argv[2];
#else
		sscanf(argv[2], "%s", inFile);
#endif
		sscanf(argv[3], "%x", &startAddr);
		if ( stat(inFile, &status) < 0 ) {
			printf("Can't stat file! [%s]\n", inFile );
			exit(1);
		}
		printf("==============================================\n");
		printf("Summary ==>\n");
		printf("Image loading  addr          :0x%x\n", (int)startAddr);
		printf("Image decompress end addr    :0x%x\n", ((unsigned int)DEFAULT_BASE_ADDR)+(unsigned int)status.st_size);

		total_size = startAddr - ((unsigned int)DEFAULT_BASE_ADDR);

		if (status.st_size > (int)total_size)
		{
			printf("Error!!!! : Kernel image decompress will overwirte load image\n");
			exit(1);
		}
		else
			printf("Available size               :0x%08x\n",
					 (unsigned int)(total_size - status.st_size));

		exit(0);
	}

#ifdef CONFIG_RTL_FLASH_MAPPING_ENABLE
	if (argc == 3 && !strcmp(argv[1], "flash_size_chk")) {
		unsigned int total_size;

#ifdef COMPACT_FILENAME_BUFFER
		inFile = argv[2];
#else
		sscanf(argv[2], "%s", inFile);
#endif
		if ( stat(inFile, &status) < 0 ) {
			printf("Can't stat file! [%s]\n", inFile );
			exit(1);
		}
		printf("==============================================\n");
		printf("Summary ==>\n");
		printf("Image flash start  addr          :0x%x\n", (unsigned int)CODE_IMAGE_OFFSET);
		printf("Image flash end addr             :0x%x\n", (unsigned int)CODE_IMAGE_OFFSET+(unsigned int)status.st_size);

#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		total_size = ROOT_IMAGE_OFFSET - CODE_IMAGE_OFFSET - CERT_SIZE;	// To reserve CERT_SIZE for 802.1x wlan client mode to store 802.1x certs
#else
		total_size = ROOT_IMAGE_OFFSET - CODE_IMAGE_OFFSET;
#endif

		if (status.st_size > (int)total_size)
		{
			printf("Error!!!! : Kernel image too big will overwirte rootfs image, cur size(%d), available size(%d).\n",(int)status.st_size, total_size);
			exit(1);
		}
		else
			printf("Available size               :0x%08x\n",
					 (unsigned int)(total_size - status.st_size));

		exit(0);
	}
#endif

#ifdef RTK_4K_CHKSUM_FIX
	if (argc == 4 && !strcmp(argv[1], "fix_chksum"))
	{
	    rtk_fix_4k_chk(argv[2],argv[3]);
	    return 0;
	}
#endif

	if (argc == 4 && !strcmp(argv[1], "vmlinux"))
		is_vmlinux = 1;

    if (argc == 5 && !strcmp(argv[1], "vmlinuxhdr")) {
		is_vmlinuxhdr = 1;
		start_addr = extractStartAddr(argv[4]);
	}

	if (!strcmp(argv[1], "signature")) {
		is_signature = 1;
		if (argc != 7) {
		    printf_usage();
		    exit(1);
		}
	}

	// parse input arguments
	if ( argc != 6 && !is_vmlinux && !is_vmlinuxhdr && !is_signature) {
	    printf_usage();
		exit(1);
	}



#ifdef COMPACT_FILENAME_BUFFER
	inFile = argv[2];
	outFile = argv[3];
#else
	sscanf(argv[2], "%s", inFile);
	sscanf(argv[3], "%s", outFile);
#endif

	if ((!is_vmlinux) && (!is_vmlinuxhdr)) {
		sscanf(argv[4], "%x", &startAddr);
		sscanf(argv[5], "%x", &burnAddr);
	}
	// check input file and allocate buffer
	if ( stat(inFile, &status) < 0 ) {
		printf("Can't stat file! [%s]\n", inFile );
		exit(1);
	}

	if (is_vmlinuxhdr) {
		size = status.st_size + sizeof(padding_len) + sizeof(lchecksum) + sizeof(start_addr);
		padding_len = 4 - (size%4);
		size += padding_len;
	}
	else if (!is_vmlinux) {
		size = status.st_size + sizeof(IMG_HEADER_T) + sizeof(checksum);
		if (size%2)
			size++; // pad
	}
	else {
		size = status.st_size + sizeof(padding_len) + sizeof(lchecksum);
		padding_len = 4 - (size%4);
		size += padding_len;
	}

	buf = malloc(size);
	if (buf == NULL) {
		printf("Malloc buffer failed!\n");
		exit(1);
	}
	memset(buf, '\0', size);
	pHeader = (IMG_HEADER_Tp)buf;

    if (is_vmlinuxhdr)
        buf += 8; // skip padding-length field and start-address field
	else if (!is_vmlinux)
		buf += sizeof(IMG_HEADER_T);
	else
		buf += 4; // skip padding-length field

	// Read data and generate header
	fh = open(inFile, O_RDONLY);
	if ( fh == -1 ) {
		printf("Open input file error!\n");
		free( pHeader );
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, status.st_size) != status.st_size) {
		printf("Read file error!\n");
		close(fh);
		free(pHeader);
		exit(1);
	}
	close(fh);

	if (is_vmlinuxhdr) {
		*((unsigned int *)pHeader) = DWORD_SWAP(padding_len);
		*((unsigned int *)((char *)pHeader+4)) = start_addr;
		lchecksum = DWORD_SWAP(calculate_long_checksum((unsigned int *)buf, size-12));
		memcpy(&buf[size-12], &lchecksum, 4);
	}
	else if (!is_vmlinux) {
		if( !strcmp("root", argv[1]))
			memcpy(pHeader->signature, ROOT_HEADER, SIGNATURE_LEN);
		else if ( !strcmp("boot", argv[1]))
			memcpy(pHeader->signature, BOOT_HEADER, SIGNATURE_LEN);
		else if ( !strcmp("linux", argv[1]))
			memcpy(pHeader->signature, FW_HEADER, SIGNATURE_LEN);
		else if ( !strcmp("linux-ro", argv[1]))
			memcpy(pHeader->signature, FW_HEADER_WITH_ROOT, SIGNATURE_LEN);
		else if ( !strcmp("signature", argv[1]))
			memcpy(pHeader->signature, argv[6], SIGNATURE_LEN);
		else{
			printf("not supported signature\n");
			exit(1);
		}
		pHeader->len = DWORD_SWAP((size-sizeof(IMG_HEADER_T)));
		pHeader->startAddr = DWORD_SWAP(startAddr);
		pHeader->burnAddr = DWORD_SWAP(burnAddr);

		if( !strcmp("root", argv[1])) {
			#define SIZE_OF_SQFS_SUPER_BLOCK 640
			unsigned int fs_len;
			fs_len = DWORD_SWAP((size-sizeof(IMG_HEADER_T) - sizeof(checksum)- SIZE_OF_SQFS_SUPER_BLOCK));
			memcpy(buf + 8, &fs_len, 4);
		}

		checksum = WORD_SWAP(calculateChecksum(buf, status.st_size));
		*((unsigned short *)&buf[size-sizeof(IMG_HEADER_T)-sizeof(checksum)]) = checksum;
	}
	else { // is_vmlinux=1
		*((unsigned int *)pHeader) = DWORD_SWAP(padding_len);
		lchecksum = DWORD_SWAP(calculate_long_checksum((unsigned int *)buf, size-8));
		memcpy(&buf[size-8], &lchecksum, 4);
	}

	// Write image to output file
	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU); //mark_88
	if ( fh == -1 ) {
		printf("Create output file error! [%s]\n", outFile);
		free(pHeader);
		exit(1);
	}
	write(fh, pHeader, size);
	close(fh);
	chmod(outFile, DEFFILEMODE);

	if (is_vmlinuxhdr)
	    printf("Generate image successfully, length=%d, checksum=0x%x, padding=%d, start address=0x%08x\n", size-12-padding_len, lchecksum, padding_len, DWORD_SWAP(start_addr));
	else if (!is_vmlinux)
		printf("Generate image successfully, length=%d, checksum=0x%x\n", (int)DWORD_SWAP(pHeader->len), checksum);
	else
		printf("Generate image successfully, length=%d, checksum=0x%x, padding=%d\n", size-8-padding_len, lchecksum, padding_len);

	free(pHeader);
	return 0;
}

static unsigned short calculateChecksum(char *buf, int len)
{
	int i, j;
	unsigned short sum=0, tmp;

	j = (len/2)*2;

	for (i=0; i<j; i+=2) {
		tmp = *((unsigned short *)(buf + i));
		sum += WORD_SWAP(tmp);
	}

	if ( len % 2 ) {
		tmp = buf[len-1];
		sum += WORD_SWAP(tmp);
	}
	return ~sum+1;
}
