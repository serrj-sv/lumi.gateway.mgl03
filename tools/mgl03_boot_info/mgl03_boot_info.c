#include <byteswap.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct boot_info {
   uint16_t  magic;
   uint16_t  vernum;
   uint16_t  chksum;
   uint8_t   cur_kern;
   uint8_t   cur_rootfs;
   uint8_t   newest_kern;
   uint8_t   newest_rootfs;
   uint32_t  kern0_size;
   uint16_t  kern0_chksm;
   uint8_t   kern0_err_cnt;
   uint32_t  kern1_size;
   uint16_t  kern1_chksm;
   uint8_t   kern1_err_cnt;
   uint32_t  rootfs0_size;
   uint16_t  rootfs0_chksm;
   uint8_t   rootfs0_err_cnt;
   uint32_t  rootfs1_size;
   uint16_t  rootfs1_chksm;
   uint8_t   rootfs1_err_cnt;
   uint8_t   root_sum_check;
   uint8_t   wdog_time;
   uint8_t   priv_mode;
   char   version[14];
} __attribute__((__packed__));

int chksum_gen(char * buffer) {
    uint8_t  i;
    uint32_t crc=0;

    for (i=6 ; i <= 52 ; i++) {
    	crc=crc+((uint8_t)buffer[i] << 8) + (uint8_t)buffer[i+1];
    	i++;
    };

    crc = (crc & 0xffff) + (crc >> 0x10);
    crc = ~crc & 0xffff;
    return crc;
};

int info_dump(struct boot_info b, int crc) {
    printf("boot_info: ver: %s\n", b.version);
    printf("kernel: newest:%d, curr:%d\n", b.newest_kern, b.cur_kern);
    printf("rootfs: newest:%d, curr:%d\n", b.newest_rootfs, b.cur_rootfs);
    printf("kernel[0]: sum:0x%x, size:%d, fail:%d\n",__bswap_16(b.kern0_chksm), __bswap_32(b.kern0_size), b.kern0_err_cnt);
    printf("      [1]: sum:0x%x, size:%d, fail:%d\n",__bswap_16(b.kern1_chksm), __bswap_32(b.kern1_size), b.kern1_err_cnt);
    printf("rootfs[0]: sum:0x%x, size:%d, fail:%d\n",__bswap_16(b.rootfs0_chksm), __bswap_32(b.rootfs0_size), b.rootfs0_err_cnt);
    printf("      [1]: sum:0x%x, size:%d, fail:%d\n",__bswap_16(b.rootfs1_chksm), __bswap_32(b.rootfs1_size), b.rootfs1_err_cnt);
    printf("root_sum_check: %d\n", b.root_sum_check);
    printf("priv_mode: %d\n", b.priv_mode);
    printf("vernum: %d\n", __bswap_16(b.vernum));
    printf("watchdog_time: %d\n", b.wdog_time);
    if (__bswap_16(b.chksum) == crc) {
      printf("boot_info_checksum: match\n");
    } else {
      printf("boot_info_checksum: don't match!!\n");
    }
}


int gen_image(char * fname) {
    const char hexstring[] = "7c91000044070000000000000000000000000000000000000000000000000000000000000000000001312e302e322e3030350000000000", *pos = hexstring;
    unsigned char val[54];
    FILE * fp;
    size_t count;

    for (count = 0; count < sizeof val/sizeof *val; count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2;
    }

    fp = fopen(fname,"wb");
    if (fp == NULL) {
        fprintf(stderr, "Error opening %s: %s\n", fname, strerror( errno ));
          exit(EXIT_FAILURE);
      } else {
          fwrite(val,54,1,fp);
          fclose (fp);
        }
    return 0;
}


int usage() {
  printf("MGL03 boot config tool.\n");
  printf("Usage: ./mgl03_boot_info [-f file] show\n");
  printf("       ./mgl03_boot_info [-f file] gen_default\n");
  printf("       ./mgl03_boot_info [-f file] priv_mode <on|off>\n");
  printf("       ./mgl03_boot_info [-f file] root_sum <on|off>\n");
  printf("       ./mgl03_boot_info [-f file] swap_slot\n");
  printf("       ./mgl03_boot_info [-f file] reset_fail\n");
  printf("       ./mgl03_boot_info [-f file] set_slot_newest <0|1> <0|1>\n");
  printf("       ./mgl03_boot_info [-f file] set_slot_current <0|1> <0|1>\n");
  printf("       ./mgl03_boot_info [-f file] set_kernel_0 <size> <chksum>\n");
  printf("       ./mgl03_boot_info [-f file] set_rootfs_0 <size> <chksum>\n");
  printf("       ./mgl03_boot_info [-f file] set_kernel_1 <size> <chksum>\n");
  printf("       ./mgl03_boot_info [-f file] set_rootfs_1 <size> <chksum>\n");
  printf("       ./mgl03_boot_info [-f file] set_vernum <value>\n");
  printf("       ./mgl03_boot_info [-f file] set_wdog_time <value>\n");
}

int main(int argc, char **argv)
{
  if (argc == 1){
    usage();
    exit(0);
  }

  if (strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"help") == 0 ){
    usage();
    exit(0);
  }


char * fname;
int argv_pos=1;

if (strcmp(argv[1],"-f") == 0) {
    fname = argv[2];
    argv_pos = 3;
  } else {
    fname = "/dev/mtdblock1";
  };


    FILE * fp;
    char buffer[54];
    struct boot_info b;
    uint32_t crc;

//Read data from file/device
if (strcmp(argv[argv_pos],"gen_default") == 0) {
  printf("Creating empty boot_info at: %s\n",fname);
  gen_image(fname);
  exit(0);
} else {
  printf("Reading from: %s\n",fname);
  fp = fopen(fname,"rb");
  if (fp == NULL) {
    fprintf(stderr, "Error opening %s: %s\n", fname, strerror( errno ));
    exit(EXIT_FAILURE);
  } else {
    fread(buffer,54,1,fp);
    fclose (fp);
  }
}
    crc=chksum_gen(buffer);
    memcpy(&b,buffer,sizeof(b));

if ( strcmp(argv[argv_pos],"show") == 0 ) {
  info_dump(b,crc);
  exit(0);
} else if ( strcmp(argv[argv_pos],"priv_mode") == 0 ) {
  if ( strcmp(argv[argv_pos+1],"on") == 0 ) {
    b.priv_mode=1;
  } else if ( strcmp(argv[argv_pos+1],"off") == 0 ) {
    b.priv_mode=0;
  } else {
    printf("Unknown argument [%s] for command [%s]!\n",argv[argv_pos+1],argv[argv_pos]);
    exit(1);
  }
} else if ( strcmp(argv[argv_pos],"root_sum") == 0 ) {
  if ( strcmp(argv[argv_pos+1],"on") == 0 ) {
    b.root_sum_check=1;
  } else if ( strcmp(argv[argv_pos+1],"off") == 0 ) {
    b.root_sum_check=0;
  } else {
    printf("Unknown argument [%s] for command [%s]!\n",argv[argv_pos+1],argv[argv_pos]);
    exit(1);
  }
} else if ( strcmp(argv[argv_pos],"swap_slot") == 0 ) {
  if ( b.newest_kern == 0 && b.newest_rootfs == 0 ) {
    printf("Current boot slot is 0. Switching to slot 1!\n");
    b.newest_kern=1;
    b.newest_rootfs=1;
  } else if ( b.newest_kern == 1 && b.newest_rootfs == 1 ) {
    printf("Current boot slot is 1. Switching to slot 0!\n");
    b.newest_kern=0;
    b.newest_rootfs=0;
  } else {
    printf("Unknown combination of kernel(%d) and rootfs(%d) boot slots!\n",b.newest_kern,b.newest_rootfs);
    exit(1);
  }
} else if ( strcmp(argv[argv_pos],"reset_fail") == 0 ) {
  printf("Resetting fail counters\n");
  b.kern0_err_cnt=0;
  b.rootfs0_err_cnt=0;
  b.kern1_err_cnt=0;
  b.rootfs1_err_cnt=0;
} else if ((strcmp(argv[argv_pos],"set_slot_newest") == 0 && (argc-argv_pos) == 3) && \
           (strcmp(argv[argv_pos+1],"0") == 0 || strcmp(argv[argv_pos+1],"1") == 0 ) && \
           (strcmp(argv[argv_pos+2],"0") == 0 || strcmp(argv[argv_pos+2],"1") == 0 )) {
      printf("Setting newest kernel: %d -> %s\n",b.newest_kern,argv[argv_pos+1]);
      printf("Setting newest rootfs: %d -> %s\n",b.newest_rootfs,argv[argv_pos+2]);
      b.newest_kern=atoi(argv[argv_pos+1]);
      b.newest_rootfs=atoi(argv[argv_pos+2]);
} else if ((strcmp(argv[argv_pos],"set_slot_current") == 0 && (argc-argv_pos) == 3) && \
           (strcmp(argv[argv_pos+1],"0") == 0 || strcmp(argv[argv_pos+1],"1") == 0 ) && \
           (strcmp(argv[argv_pos+2],"0") == 0 || strcmp(argv[argv_pos+2],"1") == 0 )) {
      printf("Setting current kernel: %d -> %s\n",b.cur_kern,argv[argv_pos+1]);
      printf("Setting current rootfs: %d -> %s\n",b.cur_rootfs,argv[argv_pos+2]);
      b.cur_kern=atoi(argv[argv_pos+1]);
      b.cur_rootfs=atoi(argv[argv_pos+2]);
} else if (strcmp(argv[argv_pos],"set_kernel_0") == 0 && (argc-argv_pos) == 3) {
      printf("Setting kernel 0 size: %d -> %s\n",__bswap_32(b.kern0_size),argv[argv_pos+1]);
      printf("Setting kernel 0 chksum: %x -> %s\n",__bswap_16(b.kern0_chksm),argv[argv_pos+2]);
      b.kern0_size=__bswap_32((uint32_t)strtoul(argv[argv_pos+1],(char**)0,10));
      b.kern0_chksm=__bswap_16((uint16_t)strtoul(argv[argv_pos+2],(char**)0,16));
} else if (strcmp(argv[argv_pos],"set_kernel_1") == 0 && (argc-argv_pos) == 3) {
      printf("Setting kernel 0 size: %d -> %s\n",__bswap_32(b.kern1_size),argv[argv_pos+1]);
      printf("Setting kernel 0 chksum: %x -> %s\n",__bswap_16(b.kern1_chksm),argv[argv_pos+2]);
      b.kern1_size=__bswap_32((uint32_t)strtoul(argv[argv_pos+1],(char**)0,10));
      b.kern1_chksm=__bswap_16((uint16_t)strtoul(argv[argv_pos+2],(char**)0,16));
} else if (strcmp(argv[argv_pos],"set_rootfs_0") == 0 && (argc-argv_pos) == 3) {
      printf("Setting rootfs 0 size: %d -> %s\n",__bswap_32(b.rootfs0_size),argv[argv_pos+1]);
      printf("Setting rootfs 0 chksum: %x -> %s\n",__bswap_16(b.rootfs0_chksm),argv[argv_pos+2]);
      b.rootfs0_size=__bswap_32((uint32_t)strtoul(argv[argv_pos+1],(char**)0,10));
      b.rootfs0_chksm=__bswap_16((uint16_t)strtoul(argv[argv_pos+2],(char**)0,16));
} else if (strcmp(argv[argv_pos],"set_rootfs_1") == 0 && (argc-argv_pos) == 3) {
      printf("Setting rootfs 1 size: %d -> %s\n",__bswap_32(b.rootfs1_size),argv[argv_pos+1]);
      printf("Setting rootfs 1 chksum: %x -> %s\n",__bswap_16(b.rootfs1_chksm),argv[argv_pos+2]);
      b.rootfs1_size=__bswap_32((uint32_t)strtoul(argv[argv_pos+1],(char**)0,10));
      b.rootfs1_chksm=__bswap_16((uint16_t)strtoul(argv[argv_pos+2],(char**)0,16));
} else if (strcmp(argv[argv_pos],"set_vernum") == 0 && (argc-argv_pos) == 2) {
      printf("Setting vernum: %d -> %s\n",__bswap_16(b.vernum),argv[argv_pos+1]);
      b.vernum=__bswap_16((uint16_t)strtoul(argv[argv_pos+1],(char**)0,10));
      b.chksum=0;
} else if (strcmp(argv[argv_pos],"set_wdog_time") == 0 && (argc-argv_pos) == 2) {
      printf("Setting watchdog_time value: %d -> %s\n",b.wdog_time,argv[argv_pos+1]);
      b.wdog_time=atoi(argv[argv_pos+1]);
}else {
  printf("Unknown command or syntax error: [%s]\n", argv[argv_pos]);
  usage();
  exit(0);
};

  memcpy(buffer, &b, sizeof(b));
  crc=chksum_gen(buffer);
  if ( __bswap_16(b.chksum) != crc) {
	    b.chksum=__bswap_16(crc);
	    memcpy(buffer, &b, sizeof(b));
      printf("Writing changes to: %s\n",fname);
      fp = fopen(fname,"wb");
      if (fp == NULL) {
		      fprintf(stderr, "Error opening %s: %s\n", fname, strerror( errno ));
		        exit(EXIT_FAILURE);
        } else {
		        fwrite(buffer,54,1,fp);
		        fclose (fp);
          }
    } else {
      printf("No changes. Exiting\n");
    }

    return 0;
}
