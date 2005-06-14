#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <math.h>

/*  We try here to compile in function addresses to which it is known
    that the compiler will make *direct* reference.  20040308 CM */

extern int _mcount();

int
main(int argc,char * argv[],char *envp[]) {

  FILE *f=NULL;
  char ch=0;
  jmp_buf env;
  double d=0.1;

  bzero(&env,sizeof(env));

  getc(f);
  putc(ch,f);

  _mcount();

  setjmp(env);

  cos(d);
  sin(d);
  tan(d);

  acos(d);
  asin(d);
  atan(d);

  cosh(d);
  sinh(d);
  tanh(d);

#ifndef _WIN32
  acosh(d);
  asinh(d);
  atanh(d);
#endif

  exp(d);
  log(d);

  sqrt(d);
  
  return 0;

}
