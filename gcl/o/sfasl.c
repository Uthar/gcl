/* 
   Copyright William Schelter. All rights reserved.
   There is a companion file rsym.c which is used to build
   a list of the external symbols in a COFF or A.OUT object file, for
   example saved_kcl.  These are loaded into kcl, and the
   linking is done directly inside kcl.  This saves a good 
   deal of time.   For example a tiny file foo.o with one definition
   can be loaded in .04 seconds.  This is much faster than
   previously possible in kcl.
   The function fasload from unixfasl.c is replaced by the fasload
   in this file.
   this file is included in unixfasl.c
   via #include "../c/sfasl.c\n" 
   */


/* for testing in standalone manner define STAND
   You may then compile this file cc -g -DSTAND -DDEBUG -I../hn
   a.out /tmp/foo.o /public/gcl/unixport/saved_kcl /public/gcl/unixport/
   will write a /tmp/sfasltest file
   which you can use comp to compare with one produced by ld.
   */
#if 0
#define DEBUG
#endif

#define IN_SFASL

/*  #ifdef STAND */
/*  #include "config.h" */
/*  #include "gclincl.h" */
/*  #define OUR_ALLOCA alloca */
/*  #include <stdio.h> */
/*  #include "mdefs.h" */

/*  #else */
#include "gclincl.h"
#include "include.h"
#undef S_DATA
/*  #endif */

#ifdef SEPARATE_SFASL_FILE
#include SEPARATE_SFASL_FILE
#else

#include "ext_sym.h"
struct node * find_sym();
int node_compare();
#ifndef _WIN32
void *malloc();
void *bsearch();
#endif

struct reloc relocation_info;
/* next 5 static after debug */

int debug;
#ifdef DEBUG
#define debug sfasldebug
int sfasldebug=1;
#define dprintf(s,ar) if ( debug ) { fprintf ( stderr, ( s ), ar ) ; fflush(stderr); }
#define STAT

#else  /* end debug */
#define dprintf(s,ar) 
#define STAT static
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#define PTABLE_EXTRA 20

#ifdef COFF
#  define COFF_SECTIONS 10   /* Numbner of section headers in section header buffer */
#endif

#define INVALID_NSCN 64000   /* A number greater than the number of sections ever likely to be read in. */

#ifdef _WIN32
unsigned int TEXT_NSCN = INVALID_NSCN, DATA_NSCN = INVALID_NSCN,
  BSS_NSCN = INVALID_NSCN, STAB_NSCN = INVALID_NSCN,
  STABSTR_NSCN = INVALID_NSCN, RDATA_NSCN = INVALID_NSCN,
  BIGGEST_NSCN_FOUND = 0;
#endif


struct sfasl_info {
    struct syment *s_symbol_table;
    char *s_start_address;
    char *s_start_data;
    char *s_start_rdata;
    char *s_start_bss;
    char *s_my_string_table;
    int   s_extra_bss;
    char *s_the_start;
};

struct sfasl_info *sfaslp;

#define symbol_table sfaslp->s_symbol_table
#define start_address sfaslp->s_start_address
#define my_string_table sfaslp->s_my_string_table
#define extra_bss sfaslp->s_extra_bss
#define the_start sfaslp->s_the_start

#ifdef DEBUG

#define describe_sym describe_sym1
void describe_sym1 ( int n, int aux_to_go )
{
    char *str;
    char tem[9];
    struct syment *sym;
    sym = &symbol_table[n];
    if ( sym->n_zeroes == 0 ) {
        str = &my_string_table[sym->n_offset];
    } else {
        if ( sym->n_name[SYMNMLEN -1] != 0 ) {
            /* MAKE IT NULL TERMINATED */
            strncpy ( tem, sym->n_name, SYMNMLEN );
            tem[SYMNMLEN] = '\0';
            str = &tem[0];
        } else {
            str = sym->n_name;
        }
    }
    if ( aux_to_go > 0 ) {
        fprintf ( stderr," symbol_table[%3d] (%8x): auxiliary entry (%d to go)\n", n, &symbol_table[n], aux_to_go - 1 ); 
    } else {
        if ( sym->n_zeroes == 0 ) {
            fprintf ( stderr,
                      "symbol_table[%3d] (%8x) (%22s): "
                      "n_offset %x, n_value %x, n_scnum %d, n_type %d, "
                      "n_sclass %d, n_numaux %d\n",
                      n,
                      &symbol_table[n],
                      str,
                      symbol_table[n].n_offset,
                      symbol_table[n].n_value,
                      symbol_table[n].n_scnum,
                      symbol_table[n].n_type,
                      symbol_table[n].n_sclass,
                      symbol_table[n].n_numaux );
        } else {
            fprintf ( stderr,
                      "symbol_table[%3d] (%8x) (%22s): "
                      "n_value %x, n_scnum %d, n_type %d, "
                      "n_sclass %d, n_numaux %d\n",
                      n,
                      &symbol_table[n],
                      str,
                      symbol_table[n].n_value ,
                      symbol_table[n].n_scnum ,
                      symbol_table[n].n_type ,
                      symbol_table[n].n_sclass ,
                      symbol_table[n].n_numaux );
        }
    }
}
#else
#  define describe_sym(a,b)
#endif /* DEBUG */


#ifdef STAND
#include "rel_stand.c"
#endif

/* begin reloc_file */
#include RELOC_FILE

/* end reloc_file */
int get_extra_bss ( struct syment *sym_table, int length, int start, int *ptr, int bsssize);
void relocate_symbols ( unsigned int length );
void set_symbol_address ( struct syment *sym, char *string );

/* Loop through the section headers to determine the index fo each section header
 * This is needed because depending on the compiler flags, different sections
 * can be at different indices, eg if stabs output is not present then .rdata
 * is adjacent to .bss (index 3), rather than after .stabstr (index 6).
 * This will make it easier to handle object files from other compilers than
 * our old friend gcc too, for example Visual C++.
 */
static void work_out_section_indices ( struct scnhdr *section )
{
#ifdef _WIN32
  unsigned int i;

  /* Initialise the global *_NSCN variables to INVALID_NSCN */
  TEXT_NSCN = DATA_NSCN = BSS_NSCN = STAB_NSCN = STABSTR_NSCN = RDATA_NSCN = INVALID_NSCN;
  
  for ( i = 1; i < COFF_SECTIONS; i++ ) {
    if ( strcmp(section[i].s_name, _TEXT) == 0 ) {
      TEXT_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
    if ( strcmp(section[i].s_name, _DATA) == 0 ) {
      DATA_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
    if ( strcmp(section[i].s_name, _BSS ) == 0 ) {
      BSS_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
    if ( strcmp(section[i].s_name, _STAB ) == 0 ) {
      STAB_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
    if ( strcmp(section[i].s_name, _STABSTR) == 0 ) {
      STABSTR_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
    if ( strcmp(section[i].s_name, _RDATA) == 0 ) {
      RDATA_NSCN = i;
      if ( i > BIGGEST_NSCN_FOUND ) BIGGEST_NSCN_FOUND = i;
    }
  }
#endif
}

int fasload ( object faslfile )
{
    long fasl_vector_start;
    struct filehdr fileheader;
    struct sfasl_info sfasl_info_buf;
#ifdef COFF
    struct scnhdr section[COFF_SECTIONS];
    struct aouthdr header;
#endif
    int textsize = 0, datasize = 0, bsssize = 0, stabsize = 0,
      stabstrsize = 0, rdatasize = 0, nsyms = 0;
#if defined ( READ_IN_STRING_TABLE ) || defined ( HPUX )
    int string_size=0;
#endif        

    object memory, data;
    FILE *fp;
    char filename[MAXPATHLEN];
    int i;
    int init_address=0;
    int aux_to_go = 0;
    size_t sections_read = 0;
    
#ifndef STAND	
    object *old_vs_base = vs_base;
    object *old_vs_top = vs_top;
#endif

    /* Zero out the COFF section header storage space */    
    memset ( section, 0, COFF_SECTIONS * sizeof ( struct scnhdr ) );

    sfaslp = &sfasl_info_buf;

    extra_bss=0;
#ifdef STAND
    strcpy(filename,faslfile);
    fp=fopen(filename,"r");
#else
    coerce_to_filename(faslfile, filename);
    faslfile = open_stream(faslfile, smm_input, Cnil, sKerror);
    vs_push(faslfile);
    fp = faslfile->sm.sm_fp;
#endif	

    /* Read the object file header */
    HEADER_SEEK(fp);
    if ( !fread ( (char *) &fileheader, sizeof(struct filehdr), 1, fp ) ) {
        FEerror("Could not get the header",0);
    }
    nsyms = NSYMS(fileheader);

#ifdef COFF
#  ifdef AIX3
    setup_for_aix_load();
#  endif	

    /* Read the optional object file header */
    fread ( &header, 1, fileheader.f_opthdr, fp );

    /* Read the object file section headers */
    if ( fileheader.f_nscns > ( COFF_SECTIONS - 1 ) ) {
        FEerror("Too many section headers to be read into the static storage space", 0);
    }
    sections_read = fread ( &section[1],
                            sizeof ( struct  scnhdr ),
                            fileheader.f_nscns,
                            fp );
    if ( sections_read != fileheader.f_nscns ) {
        fprintf ( stderr, "Could not get the expected number of sections (%d), got %d\n",
		  fileheader.f_nscns, sections_read );
	fflush ( stderr );
    }

    /* For platforms which have variable section header offsets/indices,
     * work out the indices. */
    work_out_section_indices ( section );


    /* Check for text section that does not come first - not currently handled. */
    if ( ( INVALID_NSCN != TEXT_NSCN ) && ( TEXT_NSCN != 1 ) ) {
      FEerror ("Can't handle a Text section at other than index 1.",0);
    }

    /* Determine the section sizes used later to allocate storage and to calculate
     * the offsets of the sections once read in. Allow for the possibility
     * of an invalid index, that is, that a section may not be present. */
    if ( INVALID_NSCN != TEXT_NSCN ) {
      textsize = section[TEXT_NSCN].s_size;
    }
    if ( INVALID_NSCN != DATA_NSCN ) {
      datasize = section[DATA_NSCN].s_size;
    }
    if ( INVALID_NSCN != BSS_NSCN ) {
      bsssize = section[BSS_NSCN].s_size;
    }
    if ( INVALID_NSCN != STAB_NSCN ) {
      stabsize = section[STAB_NSCN].s_size;
    }
    if ( INVALID_NSCN != STABSTR_NSCN ) {
      stabstrsize = section[STABSTR_NSCN].s_size;
    }
    if ( INVALID_NSCN != RDATA_NSCN ) {
      rdatasize = section[RDATA_NSCN].s_size;
    }

#endif

#ifdef BSD
    textsize=fileheader.a_text;
    datasize=fileheader.a_data;
    bsssize=fileheader.a_bss;
#endif
    dprintf ( "fasload: %s, ", filename );
    dprintf ( "number of symbols: %d\n", nsyms );

    /* Allocate space for the symbol table */
    symbol_table =
        (struct syment *) OUR_ALLOCA( sizeof(struct syment)*
                                      (unsigned int)nsyms);
    memset ( symbol_table, 0, sizeof(struct syment) * (unsigned int)nsyms );
    /* move the file pointer to the start of the symbol table */
    fseek(fp,(int)( N_SYMOFF(fileheader)),  0);

    /* Read the symbol table */
    for (i = 0;  i < nsyms;  i++) {
        fread((char *)&symbol_table[i], SYMESZ, 1, fp);
#ifdef HPUX
        symbol_table[i].n_un.n_strx = string_size;
        dprintf("string_size %d\n", string_size);
        string_size += symbol_table[i].n_length + 1;
        fseek(fp,(int)symbol_table[i].n_length,1);
#endif
    }
    /*	
       on MP386
       The sizeof(struct syment) = 20, while only SYMESZ =18. So we had to read
       one at a time.
       fread((char *)symbol_table, SYMESZ*fileheader.f_nsyms,1,fp);
       */

#ifdef READ_IN_STRING_TABLE
    my_string_table=READ_IN_STRING_TABLE(fp,string_size);
#else 
#  ifdef MUST_SEEK_TO_STROFF
    fseek(fp,N_STROFF(fileheader),0);
#  endif	
    {
        int ii=0;
	if ( !fread ( (char *) &ii, sizeof(int), 1, fp ) ) {
            FEerror ( "The string table of this file did not have any length",
                      0 );
        }
        fseek(fp,-4,1);
        /* at present the string table is located just after the symbols */
        my_string_table = OUR_ALLOCA ( (unsigned int) ii );
        memset ( my_string_table, 0, ii );
        dprintf( " string table length = %d \n", ii);
        if ( ii != fread ( my_string_table, 1, ii, fp ) ) {
            FEerror ( "Could not read whole string table", 0 );
	}
#endif
        
#ifdef SEEK_TO_END_OFILE
        SEEK_TO_END_OFILE(fp);	
#else
        /* go past any zeroes */
 	while ( ( i = getc ( fp ) ) == 0 );
	ungetc(i, fp);
#endif
        /* point at the GCL fasl data */
	fasl_vector_start=ftell(fp);

        if ( ! ( (c_table.ptable) && *(c_table.ptable) ) ) {
#ifdef DEBUG            
            fprintf ( stderr, "About to build symbol table.\n" );
#endif            
            build_symbol_table();
        }

        /* figure out if there is more bss space needed */
	extra_bss = get_extra_bss ( symbol_table,
                                    nsyms,
                                    datasize+textsize+bsssize+stabsize+stabstrsize+rdatasize,
                                    &init_address,
                                    bsssize );
	
        /* allocate some memory */
#ifndef STAND  
	{
            BEGIN_NO_INTERRUPT;
            memory = alloc_object ( t_cfdata );
            memory->cfd.cfd_self = 0;
            memory->cfd.cfd_start = 0;
            memory->cfd.cfd_size = datasize + textsize + bsssize + stabsize + stabstrsize + rdatasize + extra_bss;
            vs_push(memory);
            the_start = start_address =        
                memory->cfd.cfd_start =	
                    alloc_contblock ( memory->cfd.cfd_size );
            sfaslp->s_start_data  = start_address + textsize;
            sfaslp->s_start_rdata = sfaslp->s_start_data + datasize + stabsize + stabstrsize;
            sfaslp->s_start_bss   = sfaslp->s_start_rdata + rdatasize;
            END_NO_INTERRUPT;
        }
#else
        /* What does this mean? */
#  ifdef SILLY        
	the_start = start_address
            = malloc ( datasize + textsize + bsssize + stabsize + stabstrsize + rdatasize + extra_bss + 0x80000 );
	the_start = start_address = (char *) ( 0x1000 * ( ( ( (int) the_start + 0x70000) + 0x1000) / 0x1000 ) );
#  else  /* SILLY */
        the_start = start_address
            = malloc ( datasize + textsize + bsssize + stabsize + stabstrsize + rdatasize + extra_bss );
#  endif /* SILLY */       
	sfaslp->s_start_data  = start_address + textsize;
        sfaslp->s_start_rdata = sfaslp->s_start_data + datasize + stabsize + stabstrsize;
	sfaslp->s_start_bss   = sfaslp->s_start_rdata + rdatasize;
#endif
	dprintf(" Code size %d, ", datasize+rdatasize+textsize+bsssize + extra_bss);
	if ( fseek ( fp, N_TXTOFF(fileheader), 0) < 0 ) {
            FEerror("file seek error",0);
        }
	SAFE_FREAD ( the_start, textsize + datasize + stabsize + stabstrsize + rdatasize, 1, fp );
	dprintf("read %d bytes of text + data into memory at ", textsize + datasize + stabsize + stabstrsize + rdatasize );
        /* relocate the actual loaded text  */

        dprintf(" the_start (%x)\n", the_start);
#ifdef DEBUG        
        fprintf ( stderr,
                  "fasload %s: memory->cfd.cfd_start %x,  memory->cfd.cfd_start +  memory->cfd.cfd_size %x\n",
                  filename,
                  memory->cfd.cfd_start,
                  memory->cfd.cfd_start +  memory->cfd.cfd_size );
#endif        
	/* record which symbols are used */
        
#ifdef SYM_USED
        {
            int j=0;
            for(j=1; j< BSS_NSCN ; j++) {
                dprintf(" relocating section %d \n",j);
                if (section[j].s_nreloc) fseek(fp,section[j].s_relptr,0);
                for(i=0; i < section[j].s_nreloc; i++) {
                    struct syment *sym;
                    fread(&relocation_info, RELSZ, 1, fp);
                    sym = & symbol_table[relocation_info.r_symndx];
                    if (TC_SYMBOL_P(sym)) {
                        SYM_USED(sym) = 1;
                    }
                }
            }
        }
#endif
	/* this looks up symbols in c.ptable and also adds new externals to
	   that c.table */
	relocate_symbols(NSYMS(fileheader));  
	
#ifdef COFF
        {
            int j = 0;

	    /* Here the assumption remains that RDATA_NSCN is the highest section number. */
            for ( j = 1; j <= BIGGEST_NSCN_FOUND ; j++ ) {
                if ( ( j == TEXT_NSCN || j == DATA_NSCN || j == RDATA_NSCN ) &&
		     ( 0 != section[j].s_nreloc ) ) {
                    dprintf ( "relocating section %d \n", j );
                    fseek ( fp, section[j].s_relptr, 0 );
                    if ( j == TEXT_NSCN ) {
		      the_start = memory->cfd.cfd_start;
		    } else {
		      if ( j == DATA_NSCN ) {
			the_start = sfaslp->s_start_data;
		      } else {
			if ( j == RDATA_NSCN ) {
			  the_start = sfaslp->s_start_rdata;
			} else {
			  the_start = 0;
			  fprintf ( stderr, "Warning: the_start set to 0\n" ); fflush(stderr);
			}
		      }
		    }
                    for ( i=0; i < section[j].s_nreloc; i++ ) {
                        fread ( &relocation_info, RELSZ, 1, fp );
                        dprintf ( "    item %3d: ", i );
                        relocate() ;
                    }
                }
            }
        }
#endif
        
#ifdef BSD
        fseek(fp,N_RELOFF(fileheader),0);
	{
            int nrel = fileheader.a_trsize/sizeof(struct reloc);
            for ( i=0; i < nrel; i++ ) {
                fread ( (char *)&relocation_info, sizeof(struct reloc),
			1, fp);
                dprintf("relocating %d\n",i);
                relocate();
            }
	}
#  ifdef N_DRELOFF
        fseek (fp, N_DRELOFF(fileheader), 0);
#  endif
        {
            int nrel = fileheader.a_drsize/sizeof(struct reloc);
            the_start += fileheader.a_text;
            for (i=0; i < nrel; i++) {
                fread ( (char *) &relocation_info, sizeof(struct reloc),
                        1, fp);
                dprintf("relocating %d\n",i);
                relocate();
            }
        }
#endif

        /* end of relocation */
        dprintf(" END OF RELOCATION \n",0);
        dprintf(" invoking init function at %x", start_address);
        dprintf(" textsize is %x",textsize);
        dprintf(" datasize is %x\n",datasize);
        dprintf(" stabsize is %x\n",stabsize);
        dprintf(" stabstrsize is %x\n",stabstrsize);
        dprintf(" rdatasize is %x\n",rdatasize);

#ifdef DEBUG
        /* Output the symbol table for debugging.
         * 
         * Must do this after the string table has been read,
         * rather than while reading in the symbol table as
         * done previously. */
        aux_to_go = 0;
	for (i = 0;  i < nsyms;  i++) {
            if ( aux_to_go <= 0 ) {
                aux_to_go = symbol_table[i].n_numaux;
            }
	    if ( debug ) {
                describe_sym ( i, aux_to_go );
            }
            if ( aux_to_go > 0 ) {
                aux_to_go--;
            }
        }
#endif

        /* read in the fasl vector */
	fseek(fp,fasl_vector_start,0);
        if ( feof ( fp ) ) {
            data=0;
        } else {
            data = read_fasl_vector(faslfile);
            vs_push(data);
#ifdef COFF
            dprintf(" read fasl now symbols %d \n", fileheader.f_nsyms);
#endif
	}
	close_stream(faslfile);

#ifndef STAND
	ALLOCA_FREE(my_string_table);
	ALLOCA_FREE(symbol_table);
#  ifdef CLEAR_CACHE
	CLEAR_CACHE;
#  endif
        dprintf ( "About to call_init %x \n", init_address );
	call_init ( init_address, memory, data, 0);
        dprintf ( "Finished call_init %x \n", memory );
	
        vs_base = old_vs_base;
	vs_top = old_vs_top;
        dprintf ( "About to symbol_value %x \n", data );

        if ( symbol_value(sLAload_verboseA) != Cnil ) {
            printf("start address -T %x ", memory->cfd.cfd_start);
        }
	return ( memory->cfd.cfd_size );
#endif
#if 0        
	{
            FILE *out;
            out=fopen("/tmp/sfasltest","w");
            fwrite((char *)&fileheader, sizeof(struct filehdr), 1, out);
            fwrite(start_address,sizeof(char),datasize+textsize+stabsize+stabstrsize+rdatasize,out);
            fclose(out);
        }
#endif        
        printf("\n(start %x)\n",start_address);
    }
}

int get_extra_bss(sym_table,length,start,ptr,bsssize)
    int length,bsssize;
    struct syment *sym_table;
    int *ptr;                   /* store init address offset here */
{
    int result = start;

#ifdef AIX3
    int next_bss =  start - bsssize;
#endif

    struct syment *end,*sym;

#ifdef BSD
    char tem[SYMNMLEN +1];
#endif

    end =sym_table + length;
    for(sym=sym_table; sym < end; sym++)
        {
            
#ifdef FIND_INIT
            FIND_INIT
#endif

#ifdef AIX3
                /* we later go through the relocation entries making this 1
                   for symbols used */
#ifdef SYM_USED 
                if(TC_SYMBOL_P(sym))
                    {SYM_USED(sym) = 0;}
#endif
            
            /* fix up the external refer to _ptrgl to be local ref */
            if (sym->n_scnum == 0 &&
                 strcmp(sym->n_name,"_ptrgl")==0)
                {struct syment* s =
                     get_symbol("._ptrgl",TEXT_NSCN,sym_table,length);
                 if (s ==0) FEerror("bad glue",0);
                 sym->n_value = next_bss ;
                 ptrgl_offset = next_bss;
                 ptrgl_text = s->n_value;
                 next_bss += 0xc;
                 sym->n_scnum = DATA_NSCN;
                 ((union auxent *)(sym+1))->x_csect.x_scnlen = 0xc;

             }

            if(sym->n_scnum != BSS_NSCN) goto NEXT;
            if(SYM_EXTERNAL_P(sym))
                {int val=sym->n_value;
                 struct node joe;
                 if (val && c_table.ptable)
                     {struct node *answ;
                      answ= find_sym(sym,0);
                      if(answ)
                          {sym->n_value = answ->address ;
                           sym->n_scnum = N_UNDEF;
                           val= ((union auxent *)(sym+1))->x_csect.x_scnlen;
                           result -= val;
                           goto NEXT;
                       }}
             }
            /* reallocate the bss space */
            if (sym->n_value == 0)
                {result += ((union auxent *)(sym+1))->x_csect.x_scnlen;}
            sym->n_value = next_bss;
            next_bss += ((union auxent *)(sym+1))->x_csect.x_scnlen;
        NEXT:
            ;
            /* end aix3 */
#endif
            

            
#ifdef BSD
            tem;                /* ignored */
            if(SYM_EXTERNAL_P(sym) && SYM_UNDEF_P(sym))
#endif
#ifdef COFF
                if(0)
                    /* what we really want is
                       if (sym->n_scnum==0 && sym->n_sclass == C_EXT
                       && !(bsearch(..in ptable for this symbol)))
                       Since this won't allow loading in of a new external array
                       char foo[10]  not ok
                       static foo[10] ok.
                       for the moment we give undefined symbol warning..
                       Should really go through the symbols, recording the external addr
                       for ones found in ptable, and for the ones not in ptable
                       set some flag, and add up the extra_bss required.  Then
                       when you have the new memory chunk in hand,
                       you could make the pass setting the relative addresses.
                       for the ones you flagged last time.
                       */
#endif
                    /* external bss so not included in size of bss for file */
                    {int val=sym->n_value;
                     if (val && c_table.ptable
                          && (0== find_sym(sym,0)))
                         { sym->n_value=result;
                           result += val;}}
            
            sym += NUM_AUX(sym); 

        }
    return (result-start);
}



/* go through the symbol table changing the addresses of the symbols
   to reflect the current cfd_start */


void relocate_symbols ( unsigned int length )
{
    struct syment *end,*sym;
    unsigned int typ;
    char *str;
    char tem[SYMNMLEN +1];
    int n_value=(int)start_address;
    tem[SYMNMLEN]=0;

    end =symbol_table + length;
    for ( sym = symbol_table; sym < end; sym++ ) {
      typ = NTYPE ( sym );

#ifdef BSD
#  ifdef N_STAB    
        if (N_STAB & sym->n_type) continue; /* skip: It  is for dbx only */
#  endif    
        typ=N_SECTION(sym);
        /* if(sym->n_type  &  N_EXT) should add the symbol name,
           so it would be accessible by future loads  */
        if ( typ == N_ABS || typ == N_TEXT || typ == N_DATA || typ == N_BSS ) {
#endif

#ifdef COFF
	if ( typ == TEXT_NSCN || typ == DATA_NSCN || typ == RDATA_NSCN || typ == BSS_NSCN ) {
#ifdef  _WIN32
	  if ( typ == TEXT_NSCN )  n_value = (int)start_address;
	  if ( typ == DATA_NSCN )  n_value = (int)sfaslp->s_start_data;
	  if ( typ == RDATA_NSCN ) n_value = (int)sfaslp->s_start_rdata;
	  if ( typ == BSS_NSCN )   n_value = (int)sfaslp->s_start_bss;
#endif /* _WIN32 */
#endif /* COFF */
	  str=SYM_NAME(sym);
#ifdef AIX3 
	  if ( N_SECTION(sym) == DATA_NSCN
	       && NUM_AUX(sym) 
	       && allocate_toc(sym) )
	    break;
#endif     
	  sym->n_value = n_value;
        } else {
          if ( typ == N_UNDEF ) {
            str=SYM_NAME(sym);
            dprintf("relocate_symbols: N_UNDEF symbol %s \n", str);	
            dprintf("relocate_symbols:    symbol diff %d \n", sym - symbol_table);
            describe_sym ( sym-symbol_table, 0 );
            set_symbol_address(sym,str);
            describe_sym ( sym-symbol_table, 0 );
          } else {
#ifdef COFF
            dprintf("relocate_symbols: am ignoring a scnum %d\n",(sym->n_scnum));
#endif
	  }
        }
        sym += NUM_AUX(sym);
    }
}

/* 
   STEPS:
   1) read in the symbol table from the file,
   2) go through the symbol table, relocating external entries.
   3) for i <=2 go thru the relocation information for this section
   relocating the text.
   4) done.
   */

struct node *find_sym ( struct syment *sym, char *name )
{
    char tem[SYMNMLEN +1];
    tem [SYMNMLEN] = 0;
    if (name==0) name = SYM_NAME(sym);
    return find_sym_ptable(name);
}

void set_symbol_address ( struct syment *sym, char *string )
{
    struct node *answ;
    if ( c_table.ptable ) {
        dprintf("set_symbol_address: string %s\n", string);
        answ = find_sym(sym,string);
        dprintf("answ %d \n", (answ ? answ->address : -1) );
        if ( answ ) {
#ifdef COFF
#ifdef _AIX370
            if ( NTYPE(sym) == N_UNDEF )   
                sym->n_value = answ->address;
            else 
#endif 
                sym->n_value = answ->address -sym->n_value;
            /* for symbols in the local  data,text and bss this gets added
               on when we add the current value */
#endif
#ifdef BSD
            /* the old value of sym->n_value is the length of the common area
               starting at this address */
            sym->n_value = answ->address;
#endif
#ifdef AIX3
            fix_undef_toc_address(answ,sym,string);
#endif
      
        } else {
            fprintf ( stdout,"undefined %s symbol", string );
            fflush(stdout);
        }
    } else {
        FEerror("symbol table not loaded",0);
    }
}

/* include the machine independent stuff */
#include "sfasli.c"

#ifdef DEBUG
void print_name ( struct syment *p )
{
    char tem[10], *name;
    name = SYM_NAME(p);
    name = (((p)->n_zeroes == 0) ? 
             &my_string_table[(p)->n_offset] :
             ((p)->n_name[SYMNMLEN -1] ? 
               (strncpy(tem,(p)->n_name,  
                         SYMNMLEN), 
                 (char *)tem) : 
               (p)->n_name ));
    
    printf("(name:|%s|)",name);
    printf("(sclass 0x%x)",p->n_sclass);
#ifndef __MINGW32__ 
    printf("(external_p 0x%x)",SYM_EXTERNAL_P(p));
#endif 
    printf("(n_type 0x%x)",p->n_type);
    printf("(n_value 0x%x)",p->n_value);
    printf("(numaux 0x%x)\n",NUM_AUX(p));
    fflush(stdout);
}
#endif

#endif /* SEPARATE_SFASL_FILE */
