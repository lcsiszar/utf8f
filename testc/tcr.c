//*******************************************************************
// tcr.c: Test file creating.
//*******************************************************************

//*******************************************************************
#define DEBUG(x) x

//*******************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>  
#include <strings.h>
#include <unistd.h>
 
//*******************************************************************
#include "utf8f.h"
  
//*******************************************************************
#define UTF8L_1     0x0000007f // 00000000 00000000 00000000 0xxxxxxx
#define UTF8L_2     0x000007ff // 00000000 00000000 00000yyy yyxxxxxx
#define UTF8L_3     0x0000ffff // 00000000 00000000 zzzzyyyy yyxxxxxx
#define UTF8L_4     0x001fffff // 00000000 000wwwzz zzzzyyyy yyxxxxxx
#define UTF8L_5     0x03ffffff // 000000vv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_6     0x7fffffff // 0uvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_7     0xffffffff // uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx

//*******************************************************************
void p(uint32_t ucsx)
{
   if (-1==write(1,&ucsx,sizeof(ucsx)))
   {
      perror("Write error!");
   }
}

void p3(uint32_t ucsx)
{
   p(ucsx);p(ucsx-1);p(ucsx+1);
}

//*******************************************************************
int main()
{

   p3(0);
   p(32);
   p3(UTF8L_1);
   p3(UTF8L_2);
   p3(UTF8L_3);
   p3(UTF8L_4);
   p3(UTF8L_5);
   p3(UTF8L_6);
   p3(UTF8L_7);
   
   return 0;
}

//*******************************************************************
