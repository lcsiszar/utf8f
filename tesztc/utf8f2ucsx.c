//*******************************************************************
// utf8f2ucsx.c: utf8f->ucsx conversion.
//*******************************************************************


// #define DEBUG(x) x
#define DEBUG(x)

//*******************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
 
#include "utf8f.h"

#include "ucsx_latin2_to_unicode.h"

#define MODE_UTF8FFIXED           0
#define MODE_UTF8FMIXED_LATIN2    1
 
//*******************************************************************
static void f_utf8f2ucsx(int ifid, int ofid,unsigned int bufsize,int mode,int crlfmode)
{
   int n;
   char buf[bufsize];
   utf8fp u;
   ucsx_t ucsx;
   int mixedmode=0;
   int overeob=0;
   
   utf8fp_setup(&u,(utf8fchar_t*)buf);
   if (mode==MODE_UTF8FMIXED_LATIN2)
   {
      utf8fp_setmode(&u,UTF8FM_UTF8FMIXCODE8,_ucsx_termcharset_latin2_to_unicode+128);
      mixedmode=1;
   }
   utf8fp_setcrlfmode(&u,crlfmode);
   
   while(1)
   {
      if (!mixedmode)
      {
         if (0<(n=read(ifid,buf,bufsize)))
         {
            // fprintf(stderr,"read: n: %d\n",n);
            utf8fp_cont_l(&u,n);
         }
         else
            break;
      }
      else
      {
         if (overeob) break;

         if (0<(n=read(ifid,buf,bufsize)))
         {
            // fprintf(stderr,"read: n: %d\n",n);
            utf8fp_cont_l(&u,n);
         }
         else
            overeob=1;
      }
      
      while(1)
      {
         if (UCSX_CHECK==(ucsx=utf8fp_nextchar_line(&u)) && u.state!=UTF8FS_VALID)
         {
            if (u.state==UTF8FS_INVALID)
            {
               fprintf(stderr,"Invalid byte: %x\n",u.ucsx);
               utf8fp_nextbyte(&u);
            }
            else
               break;
         }
         else
         {
            DEBUG(fprintf(stderr,"norm: %x\n",ucsx);)
            if (-1==write(ofid,&ucsx,sizeof(ucsx))) perror("Írási hiba!");
         }
      } 
   }
   // fprintf(stderr,"State: %d,%x\n",u.state,u.ucsx);
   if (u.state!=UTF8FS_EOB)
   {
      fprintf(stderr,"Unexpected end of file: %d, %x\n",u.state,u.ucsx);
      exit(1);
   }
   exit(0);
}

//*******************************************************************
int main(int argc,char *argv[])
{
   int i;
   int crlfmode=UTF8FCRLFMODE_NOCRLFCONV;
   int mode=MODE_UTF8FFIXED;
   int bufsize=1;

   for(i=1;i<argc;i++)
   {
      // fprintf(stderr,"argv[%d]: '%s'\n",i,argv[i]);
      char *p=argv[i];
      if (!strcmp(p,"--crlf2lf"))    crlfmode=UTF8FCRLFMODE_CRLF2LF;
      else if (!strcmp(p,"--lf2crlf"))    crlfmode=UTF8FCRLFMODE_LF2CRLF;
      else if (!strcmp(p,"--nocrlfconv")) crlfmode=UTF8FCRLFMODE_NOCRLFCONV;
      else if (!strcmp(p,"--utf8fmixedlatin2")) mode=MODE_UTF8FMIXED_LATIN2;
      else if (!strcmp(p,"--utf8ffixed"))       mode=MODE_UTF8FFIXED;
      else if (!strcmp(p,"--bigbuf"))           bufsize=4096;
      else error(1,0,"Unknown option: %s\n",p);
   }
   f_utf8f2ucsx(0,1,bufsize,mode,crlfmode);
   return 0;
}

//*******************************************************************
