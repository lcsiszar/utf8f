//*******************************************************************
// utf8fmixedlatin2full2utf8f.c: utf8f/latin2->utf8f conversion,
//                               conversion table from recode.
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
#include "latin2full_to_unicode.h"

//*******************************************************************
static void f_utf8fmixedlatin2full2utf8f(int ifid, int ofid,unsigned int bufsize,int crlfmode)
{
   int n;
   char buf[bufsize];
   utf8fp u;
   ucsx_t ucsx;
   int mixedmode=0;
   int overeob=0;
   
   utf8fp_setup(&u,(utf8fchar_t*)buf);
   utf8fp_setmode(&u,UTF8FM_UTF8FMIXCODE8,_table_latin2full_to_unicode+128);
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
            int l;
            utf8fchar_t buf[UTF8FMAXBYTELEN];

            // if (-1==write(ofid,&c,sizeof(c))) perror("Write error!");
            // if (-1==write(ofid,&ucsx,sizeof(ucsx))) perror("Write error!");

            if (0==(l=ucsx2utf8f(ucsx,buf,sizeof(buf)))) error(1,0,"Inner error: ucsx2utf8f return 0");
            if (-1==write(ofid,buf,l)) perror("Write error!");

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
   f_utf8fmixedlatin2full2utf8f(0,1,bufsize,crlfmode);
   return 0;
}

//*******************************************************************


