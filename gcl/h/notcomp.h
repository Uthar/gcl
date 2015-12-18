#include <string.h>
#include <stdlib.h>

#define	CHAR_CODE_LIMIT	256	
#define	READ_TABLE_SIZE CHAR_CODE_LIMIT
#define ARRAY_RANK_LIMIT 63

void enter_mark_origin() ;

#ifdef __ia64__
EXTER int *cs_base2;
EXTER int *cs_org2;
#endif

EXTER int *cs_base;     
EXTER int *cs_org;     
EXTER int GBC_enable;

#define CHAR_SIZE 8
EXTER object sSAnotify_gbcA;

/* symbols which are not needed in compiled lisp code */
EXTER int interrupt_flag,interrupt_enable;
void install_default_signals();
/* void sigint(),sigalrm(); */
void segmentation_catcher();


EXTER int gc_enabled, saving_system;

EXTER object lisp_package,user_package;
/* #ifdef ANSI_COMMON_LISP */
/* /\* EXTER object common_lisp_package; *\/ */
/* #endif */
EXTER char *core_end;
EXTER int catch_fatal;
EXTER long real_maxpage;
char *getenv();
EXTER char *this_lisp;

#ifndef IN_MAIN
EXTER
char stdin_buf[], stdout_buf[];
#endif

EXTER object user_package;

#define TRUE 1
#define FALSE 0



#define GET_OPT_ARG(min,max) \
  va_list ap; \
  object  opt_arg[max - min]; object *__p= opt_arg ;\
  int _i=min, _nargs = VFUN_NARGS ; \
  va_start(ap); \
  if (_nargs < min || (_nargs > max)) FEerror("wrong number of args"); \
  while(_i++ <= max) { if (_i > _nargs) *__p++ = Cnil; \
			 else *__p++ = va_arg(ap,object);} \
  va_end(ap)

#ifndef NO_DEFUN
/* eg.
   A function taking from 2 to 8 args
   returning object the first args is object, the next 6 int, and last defaults to object.
   note the return type must also be put in the signature.
  DEFUN("AREF",object,fSaref,SI,2,8,NONE,oo,ii,ii,ii)
*/

#define MAKEFUN(pack,string,fname,argd) \
  (pack == SI ? SI_makefun(string,fname,argd) : \
   pack == LISP ? LISP_makefun(string,fname,argd) : \
   error("Bad pack variable in MAKEFUN\n"))

#define MAKEFUNB(pack,string,fname,argd,p)	\
  (GMP_makefunb(string,fname,argd,p))

#define MAKEFUNM(pack,string,fname,argd) \
  (pack == SI ? SI_makefunm(string,fname,argd) : \
   pack == LISP ? LISP_makefunm(string,fname,argd) : \
   error("Bad pack variable in MAKEFUN\n"))

#define mjoin(a_,b_) a_ ## b_
#define Mjoin(a_,b_) mjoin(a_,b_)

#define SI 0
#define LISP 1

#undef FFN
#undef LFD
#undef FFD
#undef STATD
#undef make_function
#undef make_si_function
#undef make_si_sfun
#undef make_special_form
#ifdef STATIC_FUNCTION_POINTERS
#define FFN(a_) Mjoin(a_,_static)
#define LFD(a_) static void FFN(a_) (); void a_  () { FFN(a_)();} static void FFN(a_)
#define FFD(a_) static void FFN(a_) (object); void a_  (object x) { FFN(a_)(x);} static void FFN(a_)
#define make_function(a_,b_) make_function_internal(a_,FFN(b_))
#define make_si_function(a_,b_) make_si_function_internal(a_,FFN(b_))
#define make_special_form(a_,b_) make_special_form_internal(a_,FFN(b_))
#define make_macro_function(a_,b_) make_macro_internal(a_,FFN(b_))
#define make_si_sfun(a_,b_,c_) make_si_sfun_internal(a_,FFN(b_),c_)
#define STATD static
#else
#define FFN(a_) (a_)
#define LFD(a_) void a_
#define FFD(a_) void a_
#define make_function(a_,b_) make_function_internal(a_,b_)
#define make_si_function(a_,b_) make_si_function_internal(a_,b_)
#define make_special_form(a_,b_) make_special_form_internal(a_,b_)
#define make_macro_function(a_,b_) make_macro_internal(a_,b_)
#define make_si_sfun(a_,b_,c_) make_si_sfun_internal(a_,b_,c_)
#define STATD
#endif

#define DEFUN(string,ret,fname,pack,min,max, flags, ret0a0,a12,a34,a56,args,doc) STATD ret FFN(fname) args;\
void Mjoin(fname,_init) () {\
  MAKEFUN(pack,string,(void *)FFN(fname),F_ARGD(min,max,(flags|ONE_VAL),ARGTYPES(ret0a0,a12,a34,a56))); \
}\
STATD ret FFN(fname) args

#define DEFUNB(string,ret,fname,pack,min,max, flags, ret0a0,a12,a34,a56,args,p,doc) STATD ret FFN(fname) args; \
void Mjoin(fname,_init) () {\
  MAKEFUNB(pack,string,(void *)FFN(fname),F_ARGD(min,max,(flags|ONE_VAL),ARGTYPES(ret0a0,a12,a34,a56)),p); \
}\
STATD ret FFN(fname) args

#define DEFUNM(string,ret,fname,pack,min,max, flags, ret0a0,a12,a34,a56,args,doc) STATD ret FFN(fname) args;\
void Mjoin(fname,_init) () {\
  MAKEFUNM(pack,string,(void *)FFN(fname),F_ARGD(min,max,flags,ARGTYPES(ret0a0,a12,a34,a56))); \
}\
STATD ret FFN(fname) args

/* eg.
   A function taking from 2 to 8 args
   returning object the first args is object, the next 6 int, and last defaults to object.
   note the return type must also be put in the signature.
  DEFUN("AREF",object,fSaref,SI,2,8,NONE,oo,ii,ii,ii)
*/

  /* these are needed to be linked in to be called by incrementally
   loaded code */
#define DEFCOMP(type,fun) type fun

#define  DEFVAR(name,cname,pack,val,doc) object cname
#define  DEFCONST(name,cname,pack,val,doc) object cname
#define  DEF_ORDINARY(name,cname,pack,doc) object cname  
#define DO_INIT(x)   
#endif /* NO_DEFUN */


object  type_name();
object fSincorret_type();


#define TYPE_OF(x) type_of(x)


/* For a faster way of checking if t0 is in several types,
   is t0 a member of types t1 t2 t3 
TS_MEMBER(t0,TS(t1)|TS(t2)|TS(t3)...)
*/
#define TS(s) (1<<s)
#define TS_MEMBER(t1,ts) ((TS(t1)) & (ts))

#define ASSURE_TYPE(val,t) if (type_of(val)!=t) TYPE_ERROR(val,type_name(t))

object IisArray();

/* array to which X is has its body displaced */
#define DISPLACED_TO(x) Mcar(x->a.a_displaced)

/* List of arrays whose bodies are displaced to X */

#define DISPLACED_FROM(x) Mcdr(x->a.a_displaced)

#define FIX_CHECK(x) (Mfix(Iis_fixnum(x)))

#define INITIAL_TOKEN_LENGTH 512

/* externals not needed by cmp */
/* print.d */
EXTER bool PRINTpackage;
EXTER bool PRINTstructure;

/* from format.c */
EXTER VOL object fmt_stream;
EXTER VOL int ctl_origin;
EXTER VOL int ctl_index;
EXTER VOL int ctl_end;
EXTER  object * VOL fmt_base;
EXTER VOL int fmt_index;
EXTER VOL int fmt_end;
typedef jmp_buf *jmp_bufp;
EXTER jmp_bufp VOL fmt_jmp_bufp;
EXTER VOL int fmt_indents;
EXTER VOL object fmt_string;
EXTER object endp_temp;

/* eval */
EXTER int eval1 ;
/* list.d */
EXTER bool in_list_flag;
EXTER object test_function;
EXTER object item_compared,reverse_comparison;
bool (*tf)();
EXTER object key_function;
object (*kf)();
object (*car_or_cdr)();


/* string.d */
EXTER  bool left_trim;
EXTER bool right_trim;
int  (*casefun)();

#define	Q_SIZE		256
#define IS_SIZE		256

struct printStruct {
 short p_queue[Q_SIZE];
 short p_indent_stack[IS_SIZE];
 int p_qh;
 int p_qt;
 int p_qc;
 int p_isp;
 int p_iisp;};

EXTER struct printStruct *printStructBufp;

#define SETUP_PRINT_DEFAULT(x) \
  struct printStruct printStructBuf; \
  struct printStruct * old_printStructBufp = printStructBufp; \
  printStructBufp = &printStructBuf; \
   setupPRINTdefault(x)

#define CLEANUP_PRINT_DEFAULT \
  cleanupPRINT(); \
  printStructBufp = old_printStructBufp


/* on most machines this will test in one instruction
   if the pointer is on the C stack or the 0 pointer
   but if the CSTACK_ADDRESS is not negative then we can't use this cheap
   test..
*/
#ifndef NULL_OR_ON_C_STACK

#define NULL_OR_ON_C_STACK(x) ({\
      /* if ((void *)(x)<data_start && ((void *)(x)!=NULL) && ((object)(x))!=Cnil && ((object)(x))!=Ct) */ \
      /* {pp(x);printf("%p %p\n",(void *)(x),data_start);}			*/ \
      ((((void *)(x))<(void *)data_start || ((void *)(x))>=(void *)core_end));})

#endif /* NULL_OR_ON_C_STACK */

#define	siScomma     sSXB
EXTER object sSXB;

#define	inheap(pp)	((char *)(pp) < heap_end)

char *lisp_copy_to_null_terminated();

int
gcl_init_cmp_anon(void);

#undef SAFE_READ
#undef SAFE_FREAD
#ifdef SGC
#define SAFE_READ(a_,b_,c_) \
   ({int _a=(a_),_c=(c_);char *_b=(b_);extern int sgc_enabled;\
     if (sgc_enabled) memset(_b,0,_c);read(_a,_b,_c);})
#define SAFE_FREAD(a_,b_,c_,d_) \
   ({int _b=(b_),_c=(c_);char *_a=(a_);FILE *_d=(d_);extern int sgc_enabled; \
     if (sgc_enabled) memset(_a,0,_b*_c);fread(_a,_b,_c,_d);})
#else
#define SAFE_READ(a_,b_,c_) read((a_),(b_),(c_))
#define SAFE_FREAD(a_,b_,c_,d_) fread((a_),(b_),(c_),(d_))
#endif

#ifdef EXPORT_GMP
#include "bfdef.h"
#endif
#include "gmp_wrappers.h"


extern enum type t_vtype;
extern int vtypep_fn(object);
extern void Check_type(object *,int (*)(object),object);


#ifdef IN_MAIN
#define PFN(a_) int Join(a_,_fn)(object x) {return a_(x);}
#else
#define PFN(a_) extern int Join(a_,_fn)(object x);
#endif
PFN(integerp)
PFN(non_negative_integerp)
PFN(rationalp)
PFN(floatp)
PFN(realp)
PFN(numberp)
PFN(characterp)
PFN(symbolp)
PFN(stringp)
PFN(string_symbolp)
PFN(packagep)
PFN(consp)
PFN(listp)
PFN(streamp)
PFN(pathname_string_symbolp)
PFN(pathname_string_symbol_streamp)
PFN(randomp)
PFN(hashtablep)
PFN(arrayp)
PFN(vectorp)
PFN(readtablep)
PFN(functionp)

/* #define TPE(a_,b_,c_) Check_type(a_,b_,c_) */
#define TPE(a_,b_,c_) if (!(b_)(*(a_))) FEwrong_type_argument((c_),*(a_))

#define check_type(a_,b_)                               ({t_vtype=(b_);TPE(&a_,vtypep_fn,type_name(t_vtype));})
#define check_type_function(a_)                         TPE(a_,functionp_fn,sLfunction)
#define check_type_integer(a_)                          TPE(a_,integerp_fn,sLinteger)
#define check_type_non_negative_integer(a_)             TPE(a_,non_negative_integerp_fn,TSnon_negative_integer)
#define check_type_rational(a_)                         TPE(a_,rationalp_fn,sLrational)
#define check_type_float(a_)                            TPE(a_,floatp_fn,sLfloat)
#define check_type_real(a_)                             TPE(a_,realp_fn,sLreal)
#define check_type_or_rational_float(a_)                TPE(a_,realp_fn,sLreal)
#define check_type_number(a_)                           TPE(a_,numberp_fn,sLnumber)
#define check_type_stream(a_)                           TPE(a_,streamp_fn,sLstream)
#define check_type_hash_table(a_)                       TPE(a_,hashtablep_fn,sLhash_table)
#define check_type_character(a_)                        TPE(a_,characterp_fn,sLcharacter)
#define check_type_symbol(a_)                           TPE(a_,symbolp_fn,sLsymbol)
#define check_type_string(a_)                           TPE(a_,stringp_fn,sLstring)
#define check_type_or_string_symbol(a_)                 TPE(a_,string_symbolp_fn,TSor_symbol_string)
#define check_type_or_symbol_string(a_)                 TPE(a_,string_symbolp_fn,TSor_symbol_string)
#define check_type_or_pathname_string_symbol_stream(a_) TPE(a_,pathname_string_symbol_streamp_fn,TSor_pathname_string_symbol_stream)
#define check_type_or_Pathname_string_symbol(a_)        TPE(a_,pathname_string_symbolp_fn,TSor_pathname_string_symbol)
#define check_type_package(a_)                          TPE(a_,packagep_fn,sLpackage)
#define check_type_cons(a_)                             TPE(a_,consp_fn,sLcons)
#define check_type_list(a_)                             TPE(a_,listp_fn,sLlist)
#define check_type_stream(a_)                           TPE(a_,streamp_fn,sLstream)
#define check_type_array(a_)                            TPE(a_,arrayp_fn,sLarray)
#define check_type_vector(a_)                           TPE(a_,vectorp_fn,sLvector)
#define check_type_readtable_no_default(a_)             TPE(a_,readtablep_fn,sLreadtable)
#define check_type_readtable(a_)                        ({if (*(a_)==Cnil) *(a_)=standard_readtable;TPE(a_,readtablep_fn,sLreadtable);})
#define check_type_random_state(a_)                     TPE(a_,randomp_fn,sLrandom_state)

#define stack_string(a_,b_) struct string _s={0};\
                            object a_=(object)&_s;\
                            set_type_of((a_),t_string);\
                            (a_)->st.st_self=(void *)(b_);\
                            (a_)->st.st_dim=(a_)->st.st_fillp=strlen(b_)

#define stack_fixnum(a_,b_) struct fixnum_struct _s={0};\
                            object a_;\
                            if (is_imm_fix(b_)) (a_)=make_fixnum(b_); else {\
                            (a_)=(object)&_s;\
                            set_type_of((a_),t_fixnum);\
                            (a_)->FIX.FIXVAL=(b_);}

/*FIXME the stack stuff is dangerous It works for error handling, but
  simple errors may evan pass the format tring up the stack as a slot
  in ansi*/
/* #define TYPE_ERROR(a_,b_) {stack_string(tp_err,"~S is not of type ~S.");\ */
/*                            Icall_error_handler(sKwrong_type_argument,tp_err,2,(a_),(b_));} */

EXTER object null_string;
object ihs_top_function_name(ihs_ptr h);
#define FEerror(a_,b_...)   Icall_error_handler(sLerror,null_string,\
                            4,sKformat_control,make_simple_string(a_),sKformat_arguments,list(b_))
#define CEerror(a_,b_,c_...)   Icall_continue_error_handler(make_simple_string(a_),sLerror,null_string,\
                               4,sKformat_control,make_simple_string(b_),sKformat_arguments,list(c_))

#define TYPE_ERROR(a_,b_)   Icall_error_handler(sLtype_error,null_string,\
                            4,sKdatum,(a_),sKexpected_type,(b_))
#define FEwrong_type_argument(a_,b_) TYPE_ERROR(b_,a_)
#define FEcannot_coerce(a_,b_)       TYPE_ERROR(b_,a_)
#define FEinvalid_function(a_)       TYPE_ERROR(a_,sLfunction)

#define CONTROL_ERROR(a_) Icall_error_handler(sLcontrol_error,null_string,4,sKformat_control,make_simple_string(a_),sKformat_arguments,Cnil)

#define PROGRAM_ERROR(a_,b_) Icall_error_handler(sLprogram_error,null_string,4,\
                                                 sKformat_control,make_simple_string(a_),sKformat_arguments,list(1,(b_)))
#define FEtoo_few_arguments(a_,b_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("~S [or a callee] requires more than ~R argument~:p."),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),make_fixnum((b_)-(a_))))
#define FEwrong_no_args(a_,b_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string(a_),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),(b_)))
#define FEtoo_few_argumentsF(a_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("Too few arguments."),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),(a_)))

#define FEtoo_many_arguments(a_,b_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("~S [or a callee] requires less than ~R argument~:p."),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),make_fixnum((b_)-(a_))))
#define FEtoo_many_argumentsF(a_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("Too many arguments."),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),(a_)))
#define FEinvalid_macro_call() \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("Invalid macro call to ~S."),\
                      sKformat_arguments,list(1,ihs_top_function_name(ihs_top)))
#define FEunexpected_keyword(a_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string("~S does not allow the keyword ~S."),\
                      sKformat_arguments,list(2,ihs_top_function_name(ihs_top),(a_)))
#define FEinvalid_form(a_,b_) \
  Icall_error_handler(sLprogram_error,null_string,4,\
                      sKformat_control,make_simple_string(a_),\
                      sKformat_arguments,list(1,(b_)))
#define FEinvalid_variable(a_,b_) FEinvalid_form(a_,b_)

#define PARSE_ERROR(a_)   Icall_error_handler(sLparse_error,null_string,4,\
                                              sKformat_control,make_simple_string(a_),sKformat_arguments,Cnil)
#define STREAM_ERROR(a_,b_) Icall_error_handler(sLstream_error,null_string,6,\
                                                sKstream,a_,\
                                                sKformat_control,make_simple_string(b_),sKformat_arguments,Cnil)
#define READER_ERROR(a_,b_) Icall_error_handler(sLreader_error,null_string,6,\
                                                sKstream,a_,\
                                                sKformat_control,make_simple_string(b_),sKformat_arguments,Cnil)
#define FILE_ERROR(a_,b_) Icall_error_handler(sLfile_error,null_string,6,\
                                                sKpathname,a_,\
                                                sKformat_control,make_simple_string(b_),sKformat_arguments,Cnil)
#define END_OF_FILE(a_) Icall_error_handler(sLend_of_file,null_string,2,sKstream,a_)
#define PACKAGE_ERROR(a_,b_) Icall_error_handler(sLpackage_error,null_string,6,\
                                                 sKpackage,a_,\
                                                 sKformat_control,make_simple_string(b_),sKformat_arguments,Cnil)
#define FEpackage_error(a_,b_) PACKAGE_ERROR(a_,b_)
#define PACKAGE_CERROR(a_,b_,c_,d_...) \
                     Icall_continue_error_handler(make_simple_string(b_),\
                                                  sLpackage_error,null_string,6,\
                                                  sKpackage,a_,\
                                                  sKformat_control,make_simple_string(c_),sKformat_arguments,list(d_))
#define NEW_INPUT(a_) (a_)=Ieval1(read_object(sLAstandard_inputA->s.s_dbind))


#define CELL_ERROR(a_,b_) Icall_error_handler(sLcell_error,null_string,6,\
                                              sKname,a_,\
                                              sKformat_control,make_simple_string(b_),sKformat_arguments,Cnil)
#define UNBOUND_VARIABLE(a_) Icall_error_handler(sLunbound_variable,null_string,2,sKname,a_)
#define FEunbound_variable(a_) UNBOUND_VARIABLE(a_)

#define UNBOUND_SLOT(a_,b_) Icall_error_handler(sLunbound_slot,null_string,4,sKname,a_,sKinstance,b_)
#define UNDEFINED_FUNCTION(a_) Icall_error_handler(sLundefined_function,null_string,2,sKname,a_)
#define FEundefined_function(a_) UNDEFINED_FUNCTION(a_)

#define ARITHMETIC_ERROR(a_,b_) Icall_error_handler(sLarithmetic_error,null_string,4,sKoperation,a_,sKoperands,b_)
#define DIVISION_BY_ZERO(a_,b_) Icall_error_handler(sLdivision_by_zero,null_string,4,sKoperation,a_,sKoperands,b_)
#define FLOATING_POINT_OVERFLOW(a_,b_) Icall_error_handler(sLfloating_point_overflow,null_string,4,sKoperation,a_,sKoperands,b_)
#define FLOATING_POINT_UNDERFLOW(a_,b_) Icall_error_handler(sLfloating_point_underflow,null_string,4,sKoperation,a_,sKoperands,b_)
#define FLOATING_POINT_INEXACT(a_,b_) Icall_error_handler(sLfloating_point_inexact,null_string,4,sKoperation,a_,sKoperands,b_)
#define FLOATING_POINT_INVALID_OPERATION(a_,b_) Icall_error_handler(sLfloating_point_invalid_operation,null_string,4,sKoperation,a_,sKoperands,b_)

#define PATHNAME_ERROR(a_,b_,c_...) Icall_error_handler(sLfile_error,null_string,6,\
                                                        sKpathname,(a_),\
				   		        sKformat_control,make_simple_string(b_),\
                                                        sKformat_arguments,list(c_))
#define WILD_PATH(a_) ({object _a=(a_);PATHNAME_ERROR(_a,"File ~s is wild",1,_a);})


#define NERROR(a_)  ({object fmt=make_simple_string(a_ ": line ~a, file ~a, function ~a");\
                    {object line=make_fixnum(__LINE__);\
                    {object file=make_simple_string(__FILE__);\
                    {object function=make_simple_string(__FUNCTION__);\
                     Icall_error_handler(sKerror,fmt,3,line,file,function);}}}})

#define ASSERT(a_) do {if (!(a_)) NERROR("The assertion " #a_ " failed");} while (0)

#define gcl_abort()  ({\
   frame_ptr fr=frs_sch_catch(sSPtop_abort_tagP->s.s_dbind);\
   vs_base[0]=sSPtop_abort_tagP->s.s_dbind;\
   vs_top=vs_base+1;\
   if (fr) unwind(fr,sSPtop_abort_tagP->s.s_dbind);\
   abort();\
 })

#define stack_cons() ({object _d=vs_pop,_a=vs_pop;vs_push(make_cons(_a,_d));})

char FN1[PATH_MAX],FN2[PATH_MAX],FN3[PATH_MAX],FN4[PATH_MAX],FN5[PATH_MAX];

#define coerce_to_filename(a_,b_) coerce_to_filename1(a_,b_,sizeof(b_))

#include <errno.h>
#define massert(a_) ({errno=0;if (!(a_)||errno) assert_error(#a_,__LINE__,__FILE__,__FUNCTION__);})

extern bool writable_malloc;
#define writable_malloc_wrap(f_,rt_,a_...) ({rt_ v;bool w=writable_malloc;writable_malloc=1;v=f_(a_);writable_malloc=w;v;})
#define fopen(a_,b_) writable_malloc_wrap(fopen,FILE *,a_,b_)

#define Mcar(x)	(x)->c.c_car
#define Mcdr(x)	(x)->c.c_cdr
#define Mcaar(x)	(x)->c.c_car->c.c_car
#define Mcadr(x)	(x)->c.c_cdr->c.c_car
#define Mcdar(x)	(x)->c.c_car->c.c_cdr
#define Mcddr(x)	(x)->c.c_cdr->c.c_cdr
#define Mcaaar(x)	(x)->c.c_car->c.c_car->c.c_car
#define Mcaadr(x)	(x)->c.c_cdr->c.c_car->c.c_car
#define Mcadar(x)	(x)->c.c_car->c.c_cdr->c.c_car
#define Mcaddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_car
#define Mcdaar(x)	(x)->c.c_car->c.c_car->c.c_cdr
#define Mcdadr(x)	(x)->c.c_cdr->c.c_car->c.c_cdr
#define Mcddar(x)	(x)->c.c_car->c.c_cdr->c.c_cdr
#define Mcdddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_cdr
#define Mcaaaar(x)	(x)->c.c_car->c.c_car->c.c_car->c.c_car
#define Mcaaadr(x)	(x)->c.c_cdr->c.c_car->c.c_car->c.c_car
#define Mcaadar(x)	(x)->c.c_car->c.c_cdr->c.c_car->c.c_car
#define Mcaaddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_car->c.c_car
#define Mcadaar(x)	(x)->c.c_car->c.c_car->c.c_cdr->c.c_car
#define Mcadadr(x)	(x)->c.c_cdr->c.c_car->c.c_cdr->c.c_car
#define Mcaddar(x)	(x)->c.c_car->c.c_cdr->c.c_cdr->c.c_car
#define Mcadddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_cdr->c.c_car
#define Mcdaaar(x)	(x)->c.c_car->c.c_car->c.c_car->c.c_cdr
#define Mcdaadr(x)	(x)->c.c_cdr->c.c_car->c.c_car->c.c_cdr
#define Mcdadar(x)	(x)->c.c_car->c.c_cdr->c.c_car->c.c_cdr
#define Mcdaddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_car->c.c_cdr
#define Mcddaar(x)	(x)->c.c_car->c.c_car->c.c_cdr->c.c_cdr
#define Mcddadr(x)	(x)->c.c_cdr->c.c_car->c.c_cdr->c.c_cdr
#define Mcdddar(x)	(x)->c.c_car->c.c_cdr->c.c_cdr->c.c_cdr
#define Mcddddr(x)	(x)->c.c_cdr->c.c_cdr->c.c_cdr->c.c_cdr

#include "prelink.h"
