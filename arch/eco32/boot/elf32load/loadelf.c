/*
 * loadelf.c -- load ELF file
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "elf.h"


/**************************************************************/


int verbose;


/**************************************************************/


void error(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("error: ");
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
  exit(1);
}


/**************************************************************/


int targetIsBigEndian = 0;


unsigned int read4(unsigned char *p) {
  if (targetIsBigEndian) {
    return (unsigned int) p[0] << 24 |
           (unsigned int) p[1] << 16 |
           (unsigned int) p[2] <<  8 |
           (unsigned int) p[3] <<  0;
  } else {
    return (unsigned int) p[3] << 24 |
           (unsigned int) p[2] << 16 |
           (unsigned int) p[1] <<  8 |
           (unsigned int) p[0] <<  0;
  }
}


void write4(unsigned char *p, unsigned int data) {
  if (targetIsBigEndian) {
    p[0] = (data >> 24) & 0xFF;
    p[1] = (data >> 16) & 0xFF;
    p[2] = (data >>  8) & 0xFF;
    p[3] = (data >>  0) & 0xFF;
  } else {
    p[3] = (data >> 24) & 0xFF;
    p[2] = (data >> 16) & 0xFF;
    p[1] = (data >>  8) & 0xFF;
    p[0] = (data >>  0) & 0xFF;
  }
}


unsigned short read2(unsigned char *p) {
  if (targetIsBigEndian) {
    return (unsigned short) p[0] << 8 |
           (unsigned short) p[1] << 0;
  } else {
    return (unsigned short) p[1] << 8 |
           (unsigned short) p[0] << 0;
  }
}


/**************************************************************/


#define LDERR_NONE	0	/* no error */
#define LDERR_SFH	1	/* cannot seek to file header */
#define LDERR_RFH	2	/* cannot read file header */
#define LDERR_WMN	3	/* wrong magic number in file header */
#define LDERR_WCL	4	/* wrong class */
#define LDERR_WDE	5	/* wrong data encoding */
#define LDERR_WOF	6	/* wrong object file type */
#define LDERR_WMT	7	/* wrong machine type */
#define LDERR_NPH	8	/* no program header */
#define LDERR_SPH	9	/* cannot seek to program header */
#define LDERR_RPH	10	/* cannot read program header */
#define LDERR_PHT	11	/* unknown program header type */
#define LDERR_SSS	12	/* cannot seek to segment start */
#define LDERR_MEM	13	/* no memory */
#define LDERR_RSG	14	/* cannot read segment */
#define LDERR_WBF	15	/* cannot write binary file */
#define LDERR_MAX	16	/* one above topmost error number */

#define MAX_PHDR_SIZE	100


typedef struct {
  Elf32_Addr vaddr;
  Elf32_Word size;
  unsigned char *data;
} SegmentInfo;


int segmentCompare(const void *entry1, const void *entry2) {
  Elf32_Addr vaddr1;
  Elf32_Addr vaddr2;

  vaddr1 = ((SegmentInfo *) entry1)->vaddr;
  vaddr2 = ((SegmentInfo *) entry2)->vaddr;
  if (vaddr1 < vaddr2) {
    return -1;
  }
  if (vaddr1 > vaddr2) {
    return 1;
  }
  return 0;
}


int loadElf(FILE *inFile, FILE *outFile) {
  Elf32_Ehdr fileHeader;
  Elf32_Addr entry;
  Elf32_Off phoff;
  Elf32_Half phentsize;
  Elf32_Half phnum;
  int i;
  unsigned char phdrBuf[MAX_PHDR_SIZE];
  Elf32_Phdr *phdrPtr;
  Elf32_Word ptype;
  Elf32_Off offset;
  Elf32_Addr address;
  Elf32_Word filesize;
  Elf32_Word memsize;
  Elf32_Word flags;
  Elf32_Word align;
  unsigned char *buffer;
  int j;
  SegmentInfo *allSegments;
  int numSegments;

  /* read and inspect file header */
  if (fseek(inFile, 0, SEEK_SET) < 0) {
    return LDERR_SFH;
  }
  if (fread(&fileHeader, sizeof(Elf32_Ehdr), 1, inFile) != 1) {
    return LDERR_RFH;
  }
  if (fileHeader.e_ident[EI_MAG0] != ELFMAG0 ||
      fileHeader.e_ident[EI_MAG1] != ELFMAG1 ||
      fileHeader.e_ident[EI_MAG2] != ELFMAG2 ||
      fileHeader.e_ident[EI_MAG3] != ELFMAG3) {
    return LDERR_WMN;
  }
  if (fileHeader.e_ident[EI_CLASS] != ELFCLASS32) {
    return LDERR_WCL;
  }
  if (fileHeader.e_ident[EI_DATA] != ELFDATA2MSB) {
    return LDERR_WDE;
  }
  targetIsBigEndian = 1;
  if (read2((unsigned char *) &fileHeader.e_type) != ET_EXEC) {
    return LDERR_WOF;
  }
  if (read2((unsigned char *) &fileHeader.e_machine) != EM_ECO32) {
    return LDERR_WMT;
  }
  entry = read4((unsigned char *) &fileHeader.e_entry);
  phoff = read4((unsigned char *) &fileHeader.e_phoff);
  phentsize = read2((unsigned char *) &fileHeader.e_phentsize);
  phnum = read2((unsigned char *) &fileHeader.e_phnum);
  if (verbose) {
    printf("info: entry point (virtual addr) : 0x%08X\n", entry);
    printf("      program header table at    : %d bytes file offset\n", phoff);
    printf("      prog hdr tbl entry size    : %d bytes\n", phentsize);
    printf("      num prog hdr tbl entries   : %d\n", phnum);
  }
  if (phnum == 0) {
    return LDERR_NPH;
  }
  allSegments = malloc(phnum * sizeof(SegmentInfo));
  if (allSegments == NULL) {
    return LDERR_MEM;
  }
  /* read segments */
  numSegments = 0;
  for (i = 0 ; i < phnum; i++) {
    if (verbose) {
      printf("processing program header %d\n", i);
    }
    if (fseek(inFile, phoff + i * phentsize, SEEK_SET) < 0) {
      return LDERR_SPH;
    }
    if (fread(phdrBuf, 1, phentsize, inFile) != phentsize) {
      return LDERR_RPH;
    }
    phdrPtr = (Elf32_Phdr *) phdrBuf;
    ptype = read4((unsigned char *) &phdrPtr->p_type);
    if (ptype != PT_LOAD && ptype != PT_NOTE) {
      return LDERR_PHT;
    }
    offset = read4((unsigned char *) &phdrPtr->p_offset);
    address = read4((unsigned char *) &phdrPtr->p_vaddr);
    filesize = read4((unsigned char *) &phdrPtr->p_filesz);
    memsize = read4((unsigned char *) &phdrPtr->p_memsz);
    flags = read4((unsigned char *) &phdrPtr->p_flags);
    align = read4((unsigned char *) &phdrPtr->p_align);
    if (verbose) {
      printf("    offset   : 0x%08X bytes\n", offset);
      printf("    address  : 0x%08X\n", address);
      printf("    filesize : 0x%08X bytes\n", filesize);
      printf("    memsize  : 0x%08X bytes\n", memsize);
      printf("    flags    : 0x%08X\n", flags);
      printf("    align    : 0x%08X\n", align);
    }
    if (fseek(inFile, offset, SEEK_SET) < 0) {
      return LDERR_SSS;
    }
    buffer = malloc(memsize);
    if (buffer == NULL) {
      return LDERR_MEM;
    }
    if (fread(buffer, 1, filesize, inFile) != filesize) {
      return LDERR_RSG;
    }
    for (j = filesize; j < memsize; j++) {
      buffer[j] = 0;
    }
    if (verbose) {
      printf("    segment of %u bytes read", filesize);
      printf(" (+ %u bytes zeroed)", memsize - filesize);
      printf(", vaddr = 0x%08X\n", address);
    }
    allSegments[numSegments].vaddr = address;
    allSegments[numSegments].size = memsize;
    allSegments[numSegments].data = buffer;
    numSegments++;
  }
  /* sort and write segments */
  qsort(allSegments, numSegments, sizeof(SegmentInfo), segmentCompare);
  address = allSegments[0].vaddr;
  for (i = 0 ; i < numSegments; i++) {
    if (allSegments[i].size == 0) {
      continue;
    }
    while (address < allSegments[i].vaddr) {
      fputc(0, outFile);
      address++;
    }
    if (fwrite(allSegments[i].data, allSegments[i].size, 1, outFile) != 1) {
      return LDERR_WBF;
    }
    address += allSegments[i].size;
  }
  return LDERR_NONE;
}


/**************************************************************/


char *loadResult[] = {
  /*  0 */  "no error",
  /*  1 */  "cannot seek to file header",
  /*  2 */  "cannot read file header",
  /*  3 */  "wrong magic number in file header",
  /*  4 */  "wrong class",
  /*  5 */  "wrong data encoding",
  /*  6 */  "wrong object file type",
  /*  7 */  "wrong machine type",
  /*  8 */  "no program header",
  /*  9 */  "cannot seek to program header",
  /* 10 */  "cannot read program header",
  /* 11 */  "unknown program header type",
  /* 12 */  "cannot seek to segment start",
  /* 13 */  "no memory",
  /* 14 */  "cannot read segment",
  /* 15 */  "cannot write binary file",
};

int maxResults = sizeof(loadResult) / sizeof(loadResult[0]);


void usage(char *myself) {
  printf("usage: %s [-v] <ELF file> <binary file>\n", myself);
  exit(1);
}


int main(int argc, char *argv[]) {
  char *inName;
  char *outName;
  int i;
  FILE *inFile;
  FILE *outFile;
  int result;

  verbose = 0;
  inName = NULL;
  outName = NULL;
  for (i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      /* option */
      if (strcmp(argv[i], "-v") == 0) {
        verbose = 1;
      } else {
        usage(argv[0]);
      }
    } else {
      /* file */
      if (inName == NULL) {
        inName = argv[i];
      } else
      if (outName == NULL) {
        outName = argv[i];
      } else {
        error("more than two files specified");
      }
    }
  }
  if (inName == NULL) {
    error("no ELF file specified");
  }
  if (outName == NULL) {
    error("no binary file specified");
  }
  inFile = fopen(inName, "rb");
  if (inFile == NULL) {
    error("cannot open ELF file '%s'", inName);
  }
  outFile = fopen(outName, "wb");
  if (outFile == NULL) {
    error("cannot open binary file '%s'", outName);
  }
  result = loadElf(inFile, outFile);
  if (verbose || result != 0) {
    printf("%s: %s\n",
           result == 0 ? "result" : "error",
           result >= maxResults ? "unknown error number" :
                                  loadResult[result]);
  }
  fclose(inFile);
  fclose(outFile);
  return result;
}
