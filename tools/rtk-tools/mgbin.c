/*
 *      utility to merge all binary into one image
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: mgbin.c,v 1.7 2008/11/25 15:09:22 bradhuang Exp $
 *
 */
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif
#define __mips__

#include "apmib.h"

#define CONFIG_RTL_819XD
#define COMPACT_FILENAME_BUFFER

//#define DEBUG_PRINT	printf
#define DEBUG_PRINT(...)  do {} while(0)

/////////////////////////////////////////////////////////////////////////////
/* Input file type */
typedef enum { BOOT_CODE=0, CONFIG, WEB_PAGES, SYS, ROOT, ALL, INVALID_FILE=-1 } TYPE_T;

typedef struct _sector {
	TYPE_T type;
#ifdef COMPACT_FILENAME_BUFFER
	const char *filename;
#else
	char filename[80];
#endif
	unsigned long offset;
	unsigned long size;
	int with_header;
} SECTOR_T;

/////////////////////////////////////////////////////////////////////////////
static char *copyright="Copyright (c) Realtek Semiconductor Corp., 2007. All Rights Reserved.";
static char *version="1.0";

static SECTOR_T sector[ROOT+1];
static int no_hw_config=0;

#define BYTE_SWAP(word) (((word >> 8) &0xff) | ((((unsigned char)word)<<8)&0xff00) )

/////////////////////////////////////////////////////////////////////////////
static TYPE_T checkInputFile(char *filename, int *pWith_header)
{
	int fh;
	char signature[6];

#ifdef WIN32
	fh = open(filename, O_RDONLY|O_BINARY);
#else
	fh = open(filename, O_RDONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#endif
	if ( fh == -1 )
		return INVALID_FILE;

	lseek(fh, 0, SEEK_SET);

	if ( read(fh, signature, sizeof(signature)) != sizeof(signature)) {
		close(fh);
		return INVALID_FILE;
	}
	close(fh);

	*pWith_header = 1;
	if ( !memcmp(signature, BOOT_HEADER, 4))
		return BOOT_CODE;
	else if ( !memcmp(signature, WEB_HEADER, 3) )
		return WEB_PAGES;

	else if ( !memcmp(signature, FW_HEADER, 4) ||
			!memcmp(signature, FW_HEADER_WITH_ROOT, 4))
		return SYS;

	else if ( !memcmp(signature, ROOT_HEADER, 4) )
		return ROOT;

	else if ( !memcmp(signature, "h", 1) || !memcmp(signature, "COMPHS", 6))
		return CONFIG;
	else if ( !memcmp(signature, "6G", 2) || !memcmp(signature, "6A", 2) || !memcmp(signature, "6g", 2) || !memcmp(signature, "6a", 2) || !memcmp(signature, "COMPDS", 6)) {
		no_hw_config = 1;
		return CONFIG;
	}
	else if ( !memcmp(signature, "\x0b\xf0\x00\x02", 4)) {
		*pWith_header = 0;
		return BOOT_CODE;
	}
	return INVALID_FILE;
}

////////////////////////////////////////////////////////////////////////////////
static void showHelp(void)
{
	printf("\nRTL865x utility to merge binary.\n");
	printf("%s Ver %s.\n\n", copyright, version);
	printf("usage: mgbin [-s] [-c] [-a] -o outputfile bootcode config webpages linux root\n");
	printf("	-s : do byte swap\n");
	printf("	-c : cascade. May use this option to merge image for web upload\n");
	printf("	-a : add all tag in header\n");

}


////////////////////////////////////////////////////////////////////////////////
static void do_byteswap(unsigned char *buf, int len)
{
	unsigned short wVal, *ptr;
	int i=0;

	while (len > 0) {
		len -= 2;
		ptr = (unsigned short *)&buf[2*i];
		wVal = *ptr;
		*ptr = BYTE_SWAP( wVal );
		i++;
	}
}

static unsigned short calculateChecksum(char *buf, int len, unsigned short oldv)
{
	int i, j;
	unsigned short sum=oldv, tmp;

	j = (len/2)*2;

	for (i=0; i<j; i+=2) {
		tmp = *((unsigned short *)(buf + i));
		sum += WORD_SWAP(tmp);
	}

	if ( len % 2 ) {
		tmp = buf[len-1];
		sum += WORD_SWAP(tmp);
	}
	return sum;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	int argNum=1;
#ifdef COMPACT_FILENAME_BUFFER
	const char *outFile = NULL;
#else
	char outFile[80]={0};
#endif
	TYPE_T type;
	int mask=0, fh_out, fh_in, len, i, total=0;
	unsigned char *buf, *buf1;
	struct stat sbuf;
	int byteswap=0, cascade=0, last_idx=-1, is_all=0;
	int offset=0, with_header;
	IMG_HEADER_Tp pHeader;
	unsigned long burnAddr=0;
	IMG_HEADER_T allHeader;
	unsigned short checksum=0;

	memset(&sector, 0, sizeof(sector));

	while (argNum < argc) {
		if ( !strcmp(argv[argNum], "-o") ) {
			if (++argNum >= argc)
				break;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
		}
		else if ( !strcmp(argv[argNum], "-s") ) {
			byteswap = 1;
		}
		else if ( !strcmp(argv[argNum], "-c") ) {
			cascade = 1;
		}
		else if ( !strcmp(argv[argNum], "-a") ) {
			is_all = 1;
			memset(&allHeader, '\0', sizeof(IMG_HEADER_T));
			memcpy(allHeader.signature, ALL_HEADER, SIGNATURE_LEN);
		}
#if 0
		else if ( !strcmp(argv[argNum], "-a2") ) {
			is_all = 1;
			memset(&allHeader, '\0', sizeof(IMG_HEADER_T));
			memcpy(allHeader.signature, ALL_HEADER2, SIGNATURE_LEN);
		}
#endif
		else {
			if (is_all) {
				if ((argNum+1) < argc) {
					printf("argument error, only input file could be specified!\n");
					return -1;
				}
#ifdef COMPACT_FILENAME_BUFFER
				sector[0].filename = argv[argNum];
#else
				strcpy(sector[0].filename,argv[argNum]);
#endif
				sector[0].type=ALL;
				sector[0].with_header=0;
				mask |= (1 << ALL);
		}
		else {
			type=checkInputFile(argv[argNum], &with_header);
			DEBUG_PRINT("filename=%s, type=%d\n", argv[argNum], type);
			if (type == INVALID_FILE) {
				printf("\nInvalid input file %s!!\n", argv[argNum]);
				showHelp();
				exit(1);
			}
#ifdef COMPACT_FILENAME_BUFFER
			sector[type].filename = argv[argNum];
#else
			strcpy(sector[type].filename, argv[argNum]);
#endif
			sector[type].with_header = with_header;
			sector[type].type = type;
			mask |= (1 << type);
			}
		}
		argNum++;
	}

#ifdef COMPACT_FILENAME_BUFFER
	if( outFile == NULL )
#else
	if (!outFile[0])
#endif
	{
		printf("No output file specified!\n");
		showHelp();
		exit(1);
	}

	if (mask == 0) {
		printf("No valid input image found!\n");
		exit(1);
	}

	// Create output file
#ifdef WIN32
	_chmod(outFile, S_IREAD|S_IWRITE);
	fh_out = open(outFile, O_RDWR|O_CREAT|O_TRUNC|O_BINARY);
#else
	chmod(outFile, S_IREAD|S_IWRITE);
	fh_out = open(outFile, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);//mark_88
#endif
	if (fh_out == -1) {
		printf("Create output file %s error!\n", outFile);
		exit(1);
	}

	if (is_all) {
		write(fh_out, &allHeader, sizeof(allHeader));
		total += sizeof(allHeader);
		printf("\nGenerate ");
	}
	else
	printf("\nMerge ");

	for (i=BOOT_CODE; i<=ROOT ; i++) {
#ifdef COMPACT_FILENAME_BUFFER
		if (sector[i].filename)
#else
		if (sector[i].filename[0])
#endif
		{
			if ( stat(sector[i].filename, &sbuf) != 0 ) {
				printf("Stat file %s error!\n", sector[i].filename);
				exit(1);
			}
			buf = malloc(sbuf.st_size+1);
			if (buf == NULL) {
				printf("allocate buffer failed %d!\n", (int)sbuf.st_size);
				exit(1);
			}
#ifdef WIN32
			fh_in = open(sector[i].filename, O_RDONLY|O_BINARY);
#else
			fh_in = open(sector[i].filename, O_RDONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#endif
			if (fh_in < 0) {
				printf("Open file %s error!\n", sector[i].filename);
				close(fh_out);
				exit(1);
			}

			switch(sector[i].type) {
			case BOOT_CODE:
				printf("BOOT-CODE ");
				break;
			case CONFIG:
				printf("CONFIG-DATA(%s) ", (no_hw_config ? "no-hw" : "all"));
				break;
			case WEB_PAGES:
				printf("WEB-PAGES ");
				break;
			case SYS:
				printf("LINUX ");
				break;
			case ROOT:
				printf("ROOT ");
				break;
			case ALL:
				printf("ALL ");
				break;
			case INVALID_FILE:
				break;
			}

			if (read(fh_in, buf, sbuf.st_size) != sbuf.st_size) {
				printf("Read file %s error!\n", sector[i].filename);
				close(fh_in);
				close(fh_out);
				free(buf);
				exit(1);
			}

			if (is_all)
				checksum = calculateChecksum((char *)buf, sbuf.st_size, checksum);
			else {
			if (sector[i].with_header) {
				if (sector[i].type == CONFIG) {
					if (no_hw_config == 0)
						burnAddr = HW_SETTING_OFFSET;
					else
						burnAddr = DEFAULT_SETTING_OFFSET;
				}
				else {
					pHeader = (IMG_HEADER_Tp)buf;
					burnAddr = DWORD_SWAP(pHeader->burnAddr);
				}
			}
			else
				burnAddr = 0;

			if (byteswap) {
				if (sbuf.st_size % 2) {
					buf[sbuf.st_size] = '\0';
					sbuf.st_size++;
				}
				do_byteswap(buf, sbuf.st_size);
			}

			// try to append 0 if necessary
			if (!cascade && last_idx!=-1 && sector[i].with_header) {
				if ((sector[last_idx].offset+sector[last_idx].size) < burnAddr) {
					len = burnAddr - (sector[last_idx].offset+sector[last_idx].size);
					buf1 = calloc(len, 1);
					if (buf1 == NULL) {
						printf("allocate buffer failed %d!\n", len);
						exit(1);
					}
					write(fh_out, buf1, len);	// pad 0

					free(buf1);
					total += len;
//				printf("pad size=%d, last_idx=%d, burnAddr=%d\n", len, last_idx, burnAddr);
				}
			}

			// skip header if necessary
			if (!cascade && sector[i].with_header &&
					((sector[i].type == ROOT) || (sector[i].type == BOOT_CODE))) {
				offset = sizeof(IMG_HEADER_T);
//				sbuf.st_size -= sizeof(IMG_HEADER_T);
			}
			else
				offset = 0;
			}

//		printf("write offset=%d, size=%d\n", offset, 	sbuf.st_size);
			if ( write(fh_out, buf + offset , sbuf.st_size-offset) != sbuf.st_size-offset) {
				printf("Write output file %s error!\n", outFile);
				close(fh_in);
				close(fh_out);
				free(buf);
				exit(1);
			}
			close(fh_in);
			if (burnAddr > 0)
				sector[i].offset = burnAddr;
			else
				sector[i].offset = total;
			sector[i].size = sbuf.st_size-offset;

//	printf("section[%d], offset=%d, size=%d\n", i, sector[i].offset, sector[i].size);

			total += sbuf.st_size;
			free(buf);
			last_idx = i;
			if (is_all)
				break;
		}
	}

	if (is_all) {
		allHeader.len = DWORD_SWAP((total+2));
		checksum = calculateChecksum((char *)&allHeader, sizeof(allHeader), checksum);
		checksum = WORD_SWAP((~checksum+1));
		write(fh_out, &checksum, 2);
		lseek(fh_out, 0, SEEK_SET);
		allHeader.len = DWORD_SWAP((total+2));
//		allHeader.startAddr = DWORD_SWAP(image_num);
		write(fh_out, &allHeader, sizeof(allHeader));
		total += (2 + sizeof(allHeader));
	}
	close(fh_out);

#ifdef WIN32
	_chmod(outFile, S_IREAD);
#else
	chmod(outFile, DEFFILEMODE);
#endif

	printf("=> %s ok, size=%d.\n", outFile, total);
	exit(0);
}
