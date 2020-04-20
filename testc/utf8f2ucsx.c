//*******************************************************************
// utf8f2ucsx.c: utf8f->ucsx conversion.
//*******************************************************************


// #define DEBUG(x) x
#define DEBUG(x)

//*******************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
 
#include "utf8f.h"
 
//*******************************************************************
static void f_utf8f2ucsx(int ifid, int ofid,unsigned int bufsize)
{
   int n;
   char buf[bufsize];
   utf8fp u;
   ucsx_t ucsx;
   
   utf8fp_setup(&u,(utf8fchar_t*)buf);
   
   while(0<(n=read(ifid,buf,bufsize)))
   {
      utf8fp_cont_l(&u,n);
      while(1)
      {
         if (UTF8F_CHECK==(ucsx=utf8fp_nextchar(&u)) && u.state!=UTF8FS_VALID)
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
int main()
{
   f_utf8f2ucsx(0,1,7000);
   return 0;
}

//*******************************************************************
