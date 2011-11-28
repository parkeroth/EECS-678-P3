/* elf.h
 *   Data structures that describe the MIPS ELF32 format.
 *   Blatantly ripped off from the GNU BFD files by Jerry James.
 */

#ifndef _BIN_ELF_H
#define _BIN_ELF_H

#include <inttypes.h>
// FIXME: configurate this for platforms that use stdint.h instead */

// Fields in e_ident[].

#define EI_MAG0		0	/* File identification byte 0 index */
#define ELFMAG0		   0x7F	/* Magic number byte 0 */

#define EI_MAG1		1	/* File identification byte 1 index */
#define ELFMAG1		    'E'	/* Magic number byte 1 */

#define EI_MAG2		2	/* File identification byte 2 index */
#define ELFMAG2		    'L'	/* Magic number byte 2 */

#define EI_MAG3		3	/* File identification byte 3 index */
#define ELFMAG3		    'F'	/* Magic number byte 3 */

#define EI_CLASS	4	/* File class */
#define ELFCLASSNONE	      0	/* Invalid class */
#define ELFCLASS32	      1	/* 32-bit objects */
#define ELFCLASS64	      2	/* 64-bit objects */

#define EI_DATA		5	/* Data encoding */
#define ELFDATANONE	      0	/* Invalid data encoding */
#define ELFDATA2LSB	      1	/* 2's complement, little endian */
#define ELFDATA2MSB	      2	/* 2's complement, big endian */

#define EI_VERSION	6	/* File version */

#define EI_OSABI	7	/* Operating System/ABI indication */
#define ELFOSABI_NONE	      0	/* UNIX System V ABI */
#define ELFOSABI_STANDALONE 255	/* Standalone (embedded) application */

#define EI_ABIVERSION	8	/* ABI version */

#define EI_PAD		9	/* Start of padding bytes */

// Values for e_type, which identifies the object file type.

#define ET_NONE		0	/* No file type */
#define ET_REL		1	/* Relocatable file */
#define ET_EXEC		2	/* Executable file */
#define ET_DYN		3	/* Shared object file */
#define ET_CORE		4	/* Core file */

// Values for e_machine, which identifies the architecture.

#define EM_NONE		  0	/* No machine */
#define EM_MIPS		  8	/* MIPS R3000 (officially, big-endian only) */
#define EM_MIPS_RS3_LE	 10	/* MIPS R3000 little-endian (Oct 4 1999 Draft) Deprecated */

/* Values for e_version.  */

#define EV_NONE		0		/* Invalid ELF version */
#define EV_CURRENT	1		/* Current version */

typedef struct {
  unsigned char	e_ident[16];		/* ELF "magic number" */
  uint16_t	e_type;			/* Identifies object file type */
  uint16_t	e_machine;		/* Specifies required architecture */
  uint32_t	e_version;		/* Identifies object file version */
  uint32_t	e_entry;		/* Entry point virtual address */
  uint32_t	e_phoff;		/* Program header table file offset */
  uint32_t	e_shoff;		/* Section header table file offset */
  uint32_t	e_flags;		/* Processor-specific flags */
  uint16_t	e_ehsize;		/* ELF header size in bytes */
  uint16_t	e_phentsize;		/* Program header table entry size */
  uint16_t	e_phnum;		/* Program header table entry count */
  uint16_t	e_shentsize;		/* Section header table entry size */
  uint16_t	e_shnum;		/* Section header table entry count */
  uint16_t	e_shstrndx;		/* Section header string table index */
} Elf32_Elf_header;

typedef struct {
  uint32_t	p_type;			/* Identifies program segment type */
  uint32_t	p_offset;		/* Segment file offset */
  uint32_t	p_vaddr;		/* Segment virtual address */
  uint32_t	p_paddr;		/* Segment physical address */
  uint32_t	p_filesz;		/* Segment size in file */
  uint32_t	p_memsz;		/* Segment size in memory */
  uint32_t	p_flags;		/* Segment flags */
  uint32_t	p_align;		/* Segment alignment, file & memory */
} Elf32_Program_header;

/* Values for section header, sh_type field.  */

#define SHT_NULL	0		/* Section header table entry unused */
#define SHT_PROGBITS	1		/* Program specific (private) data */
#define SHT_SYMTAB	2		/* Link editing symbol table */
#define SHT_STRTAB	3		/* A string table */
#define SHT_RELA	4		/* Relocation entries with addends */
#define SHT_HASH	5		/* A symbol hash table */
#define SHT_DYNAMIC	6		/* Information for dynamic linking */
#define SHT_NOTE	7		/* Information that marks file */
#define SHT_NOBITS	8		/* Section occupies no space in file */
#define SHT_REL		9		/* Relocation entries, no addends */
#define SHT_SHLIB	10		/* Reserved, unspecified semantics */
#define SHT_DYNSYM	11		/* Dynamic linking symbol table */

#define SHT_INIT_ARRAY	  14		/* Array of ptrs to init functions */
#define SHT_FINI_ARRAY	  15		/* Array of ptrs to finish functions */
#define SHT_PREINIT_ARRAY 16		/* Array of ptrs to pre-init funcs */
#define SHT_GROUP	  17		/* Section contains a section group */
#define SHT_SYMTAB_SHNDX  18		/* Indicies for SHN_XINDEX entries */

/* Values for section header, sh_flags field.  */

#define SHF_WRITE	(1 << 0)	/* Writable data during execution */
#define SHF_ALLOC	(1 << 1)	/* Occupies memory during execution */
#define SHF_EXECINSTR	(1 << 2)	/* Executable machine instructions */
#define SHF_MERGE	(1 << 4)	/* Data in this section can be merged */
#define SHF_STRINGS	(1 << 5)	/* Contains null terminated character strings */
#define SHF_INFO_LINK	(1 << 6)	/* sh_info holds section header table index */
#define SHF_LINK_ORDER	(1 << 7)	/* Preserve section ordering when linking */
#define SHF_OS_NONCONFORMING (1 << 8)	/* OS specific processing required */
#define SHF_GROUP	(1 << 9)	/* Member of a section group */
#define SHF_TLS		(1 << 10)	/* Thread local storage section */

typedef struct {
  uint32_t	sh_name;		/* Section name, index in string tbl */
  uint32_t	sh_type;		/* Type of section */
  uint32_t	sh_flags;		/* Miscellaneous section attributes */
  uint32_t	sh_addr;		/* Section virtual addr at execution */
  uint32_t	sh_offset;		/* Section file offset */
  uint32_t	sh_size;		/* Size of section in bytes */
  uint32_t	sh_link;		/* Index of another section */
  uint32_t	sh_info;		/* Additional section information */
  uint32_t	sh_addralign;		/* Section alignment */
  uint32_t	sh_entsize;		/* Entry size if section holds table */
} Elf32_Section_header;

#endif /* _BIN_ELF_H */
