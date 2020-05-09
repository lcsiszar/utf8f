//*******************************************************************
// c_latin2full_unicode_pairs.cpp: Create latin2 (full ISO) and
//                                 unicode pairs.  Codec by recode.
//*******************************************************************

//*******************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <error.h>

//*******************************************************************
#define FLATIN2FULL "latin2full_8bit.txt"
#define FLATIN2FULL_UCSX "latin2full_ucsx.txt"
#define FLATIN2FULL_UCSX_CODEC "latin2full_ucsx_codec.txt"

#define FLATIN2FULL_UCSX_PAIRS "latin2full_ucsx_pairs.txt"

//*******************************************************************
#define MAX_UCSX_TABLE   0xffff
#define MAX_LATIN2FULL_TABLE 256

//*******************************************************************
static uint32_t ucsx_buf[MAX_LATIN2FULL_TABLE];

//*******************************************************************
int w_latin2full()
{
   int i;

   FILE *f;

   if (NULL==(f=fopen(FLATIN2FULL,"w")))
   {
      error(1,errno,"%s: create error",FLATIN2FULL);
   }

   for(i=0;i<MAX_LATIN2FULL_TABLE;i++)
   {
      fprintf(f,"%c",(unsigned char)i);
   }

   fclose(f);
}

//*******************************************************************
void w_ucsx()
{
   char buf[512];
   int w;

   if (sizeof(buf)<snprintf(buf,sizeof(buf),
                            "recode latin2..ucs4-le<%s >%s",
                            FLATIN2FULL,FLATIN2FULL_UCSX))
   {
      error(1,0,"Out of buffer");
   }
   
   if (0!=(w=system(buf)))
   {
      error(1,0,"recode failed: ret: %d, %s",w,buf);
   }
}

//*******************************************************************
void r_ucsx()
{
   int i;

   FILE *f;

   if (NULL==(f=fopen(FLATIN2FULL_UCSX,"r")))
   {
      error(1,errno,"%s: open error",FLATIN2FULL_UCSX);
   }
   if (sizeof(ucsx_buf)!=fread(ucsx_buf,1,sizeof(ucsx_buf),f)) error(1,errno,"%s: read error",FLATIN2FULL_UCSX);
   fclose(f);
}

//*******************************************************************
void p_codec()
{
   int i;

   FILE *f;

   if (NULL==(f=fopen(FLATIN2FULL_UCSX_CODEC,"w")))
   {
      error(1,errno,"%s: create error",FLATIN2FULL_UCSX_CODEC);
   }
   for(i=0;i<256;i++)
   {
      fprintf(f,"%.3d(%.3x): %.4d(%.4x)\n",i,i,ucsx_buf[i],ucsx_buf[i]);
   }
   fclose(f);
}

//*******************************************************************
void w_pairs()
{
   int i;

   FILE *f;

   if (NULL==(f=fopen(FLATIN2FULL_UCSX_PAIRS,"w")))
   {
      error(1,errno,"%s: open error",FLATIN2FULL_UCSX_PAIRS);
   }

   int sorhossz=8;
   int isor=0;
   for(i=0;i<MAX_LATIN2FULL_TABLE;i++)
   {
      if (i!=ucsx_buf[i])
      {
         fprintf(f,"   {0x%.2x, 0x%.3x}, // 0x%.2x(%3.d), 0x%.3x(%.3d)\n",
                        i,ucsx_buf[i],i,i,ucsx_buf[i],ucsx_buf[i]);
      }
   }
   fclose(f);
}

//*******************************************************************
int main()
{
   w_latin2full();
   w_ucsx();
   r_ucsx();
   p_codec();
   w_pairs();
   // exit(0);
}

//*******************************************************************
