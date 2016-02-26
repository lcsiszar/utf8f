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
 
#include "utf8f.h"
 
//*******************************************************************
static void f_utf8f2ucsx(int ifid, int ofid,unsigned int bufsize)
{
   int n;
   char buf[bufsize];
   utf8fp u;
   ucsx_t ucsx;
   
   utf8fp_setup(&u,(utf8char_t*)buf,bufsize);
   
   while(0<(n=read(ifid,buf,bufsize)))
   {
      utf8fp_cont_l(&u,n);
      while(1)
      {
         if (UTF8F_CHECK==(ucsx=utf8fp_nextchar(&u)))
         {
            if (u.state==UTF8FS_VALID)
            {
               DEBUG(fprintf(stderr,"check: %x\n",ucsx);)
               if (-1==write(ofid,&ucsx,sizeof(ucsx))) perror("Írási hiba!");
            }
            else if (u.state==UTF8FS_INVALID)
            {
               DEBUG(fprintf(stderr,"Invalid byte: %x\n",u.ucsx);)
               utf8fp_nextbyte(&u);
               continue;
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
}

//*******************************************************************
int main()
{
   f_utf8f2ucsx(0,1,8);
   return 0;
}

//*******************************************************************

