//*******************************************************************
// tutf8f.c: Test utf8f functions.
//*******************************************************************

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <strings.h>

#include "utf8f.h"

//*******************************************************************
static void printfb_uchar(unsigned char c)
{
   int i;
   
   for(i=7;i>=0;i--)
   {
      printf(c&(1<<i)?"1":"0");
   }
} 

//*******************************************************************
static void printfb6_uint32(uint32_t c)
{
   int i;
   
   for(i=31;i>=0;i--)
   {
      printf(c&(1<<i)?"1":"0");
      if (i%6==0) printf(" ");
   }
} 

//*******************************************************************
void pUcsx(ucsx_t ucsx)
{
   utf8char_t utf8f[9];
   int i,l;
   
   bzero(utf8f,sizeof(utf8f));
   printf("ucsx: %d, ",ucsx);
   printfb6_uint32(ucsx);
   l=ucsx2utf8f(ucsx,utf8f,sizeof(utf8f));
   printf(", Len: %d\n",l);
   utf8f[l]='\n';
   
   printf("   ");
   for(i=0;i<l;i++)
   {
      printf(" %d: 0x%x,",i,utf8f[i]);
      printfb_uchar(utf8f[i]);
      printf("; ");
   }
   
   // printf("\n   utf8f: %s",utf8f);
   printf("\n");
   
   {
      utf8fp up;
      ucsx_t rUcsx;
      
      utf8fp_start(&up,utf8f,9);
      if (UTF8F_CHECK==(rUcsx=utf8fnextchar(&up)))
      {
         if (up.status==UTF8FS_VALID)
         {
            rUcsx=up.ucsx;
            printf("Check! ");
            goto valid;
         }
         else
         {
            printf("Invalid uft8 kód!\n");
         }
      }
      else
      {
         valid:
         printf("rUcsx: Valid: %d,",rUcsx);
         printfb6_uint32(rUcsx);
         printf("%s\n",ucsx==rUcsx?"Egyezik":"Nem egyezik!");
      }
   }
   // write(1,utf8f,l);
}

//*******************************************************************
int main()
{
   ucsx_t ucsx;
   
   for(ucsx=1;ucsx!=0; ucsx<<=1)
   {
      pUcsx(ucsx);
      // break;
   }
   pUcsx(0xffffffff);
   return 0;
}

//*******************************************************************
