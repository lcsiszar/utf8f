//*******************************************************************
// utf8fmixedlatin2full2latin2full.c: utf8f/latin2->latin2 conversion,
//                                    conversion table from recode.
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

//*******************************************************************
static
#include "unicode_to_latin2full.h"

//*******************************************************************
static ucsx_t ucsxconverter_2latin2full(ucsx_t ucsx, utf8fp *up)
{
   // fprintf(stderr,"converter: %d(%x)\n",ucsx,ucsx);
   return unicode_to_latin2full(ucsx,'?');
}

//*******************************************************************
static void f_utf8fmixedlatin2full2latin2full(int ifid, int ofid,unsigned int bufsize,int crlfmode)
{
   int n;
   char buf[bufsize];
   utf8fp u;
   ucsx_t ucsx;
   int mixedmode=0;
   int overeob=0;
   
   utf8fp_setup(&u,(utf8fchar_t*)buf);
   // utf8fp_setmode(&u,UTF8FM_UTF8FMIXCODE8,_table_latin2full_to_unicode);
   utf8fp_setmode(&u,UTF8FM_UTF8FMIXCODE8,NULL);
   // utf8fp_setucsxconverter(&u,(utf8f_ucsxconverter *)&ucsxconverter_2latin2full);
   utf8fp_setucsxconverter(&u,ucsxconverter_2latin2full);
   mixedmode=1;
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
            char c=ucsx&0xff;
            if (-1==write(ofid,&c,sizeof(c))) perror("Write error!");
            // if (-1==write(ofid,&ucsx,sizeof(ucsx))) perror("Write error!");
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
   int bufsize=1024;

   for(i=1;i<argc;i++)
   {
      // fprintf(stderr,"argv[%d]: '%s'\n",i,argv[i]);
      char *p=argv[i];
      if (!strcmp(p,"--crlf2lf"))         crlfmode=UTF8FCRLFMODE_CRLF2LF;
      else if (!strcmp(p,"--lf2crlf"))    crlfmode=UTF8FCRLFMODE_LF2CRLF;
      else if (!strcmp(p,"--nocrlfconv")) crlfmode=UTF8FCRLFMODE_NOCRLFCONV;
      else if (!strcmp(p,"--bigbuf"))     bufsize=4096;
      else if (!strcmp(p,"--smallbuf"))   bufsize=1;
      else error(1,0,"Unknown option: %s\n",p);
   }
   f_utf8fmixedlatin2full2latin2full(0,1,bufsize,crlfmode);
   return 0;
}

//*******************************************************************
