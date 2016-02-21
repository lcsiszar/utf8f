//*******************************************************************
//ucsx2utf8f.c: ucsx->utf8f conversion.
//*******************************************************************


#define DEBUG(x) x

//*******************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
 
#include "utf8f.h"
 
//*******************************************************************
static void f_ucsx2utf8f(int ifid, int ofid,unsigned int bufsize,int bufsize8)
{
   int n;
   uint32_t buf[bufsize];
   utf8char_t utf8buf[bufsize8*8];
   unsigned int iUtf8buf;
   unsigned int i;

   iutf8buf=0;
   if (bufsize<=0 || bufsize8<=0)
   {
      fprintf(stderr,"Invalid buffer size: bufsize: %d, bufsize8: %d\n",bufsize,bufsize8);
      return;
   }   

   while(0<(n=read(ifid,buf,sizeof(buf))))
   {
      for(i=0;i<(unsigned int)n/sizeof(*buf))
      {
         int l;
         
         if (0==(l=ucsxutf8f(buf[i],utf8buf+iUtf8buf,sizeof(utf8buf)-iUtf8buf)))
         {
            if (-1==write(ofid,utf8buf,iUtf8buf)) perror("Írási hiba!");
            iUtf8buf=0;
            if (0==(l=ucsxutf8f(buf[i],utf8buf+iUtf8buf,sizeof(utf8buf)-iUtf8buf)))
            {
               fprintf(stderr,"Inner error: ucsxutf8f return 0.\n");
               return;
            }
         }
         iUtf8buf+=l;
      }
   }
   
   if (iUtf8buf>0)
   {
      if (-1==write(ofid,utf8buf,iUtf8buf)) perror("Írási hiba!");
   }

}

//*******************************************************************
int main()
{
   f_ucsx2utf8f(0,1,8);
   return 0;
}

//*******************************************************************

