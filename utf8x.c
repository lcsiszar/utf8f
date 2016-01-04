//*******************************************************************
// utf8f.c: utf8f-ucsf konverzió.
//*******************************************************************


/*
 32 bites számokkal kódolt karaktersorozatokat kódol át utf8-ra és vissza
 egyértelműen. A 32 bites karaktereket ucsf karakternek hívja.
 MInden 32 bites számot lekódol.
 Egy utf8f karakter akkor érvényes kód, ha ucsf-re konvertálva, majd
 vissza konvertálva ugyanazt az kódot kapjuk.
 
 Mj.: Érvénytelenség okai:
      - Nem minimális a kódolás
      - Nem szabály szerinti kódolás,
      - véget ér a string, de a fejléc szerint még folytatódna.
      
*/
/*
Tervezett rutinok:
  Egy karaktert átkódolni.
  Pufferben mozogni és kódolni.
  Pufferben oda-vissza mozogni.
  Tetszőleges pozícióból karakter végét/elejét megkeresni.

*/

#include <stdint.h>
#include <stddef.h>

typedef int8_t   utf8f_int8_t;
typedef uint8_t  utf8f_uint8_t;
typedef int16_t  utf8f_int16_t;
typedef uint16_t utf8f_uint16_t;
typedef int32_t  utf8f_int32_t;
typedef uint32_t utf8f_uint32_t;
typedef size_t   utf8f_size_t;
//typedef ssize_t  utf8f_ssize_t; // Az unistd.h-ban van.
// typedef bool     utf8f_bool;
         
typedef unsigned char utf8char_t;
typedef uint32_t ucsf_t;

//*******************************************************************
// Egy puffer cim, egy pointer benne, meg a puffer hossza.
// Ezt használja, amikor részletekben jön az input és azt
// kell folyamként kódolni.
typedef struct _utf8fp
{
   int status;
   utf8char_t *buf;
   size_t i;
   size_t l;
   ucsf_t ucsf;
} utf8fp;


//*******************************************************************
#define UTF8F_CHECK  ((ucsf_t)-1) // Lehetne 0 is.

//*******************************************************************
#define UTF8FS_VALID   0
#define UTF8FS_INVALID 1

#define UTF8FS_S1      2


/*
00000000 00000000 00000000 0xxxxxxx 	0xxxxxxx
00000000 00000000 00000yyy yyxxxxxx 	110yyyyy 10xxxxxx
00000000 00000000 zzzzyyyy yyxxxxxx 	1110zzzz 10yyyyyy 10xxxxxx
00000000 000wwwzz zzzzyyyy yyxxxxxx 	11110www 10zzzzzz 10yyyyyy 10xxxxxx
000000vv wwwwwwzz zzzzyyyy yyxxxxxx 	111110vv 10wwwwww 10zzzzzz 10yyyyyy 10xxxxxx
0uvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx 	1111110u 10vvvvvv 10wwwwww 10zzzzzz 10yyyyyy 10xxxxxx
uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx 	11111110 100000uu 10vvvvvv 10wwwwww 10zzzzzz 10yyyyyy 10xxxxxx

00000000 00000000 00000000 0xxxxxxx 	0x0000007f // mask
00000000 00000000 00000yyy yyxxxxxx 	0x000007ff
00000000 00000000 zzzzyyyy yyxxxxxx 	0x0000ffff
00000000 000wwwzz zzzzyyyy yyxxxxxx 	0x001fffff
000000vv wwwwwwzz zzzzyyyy yyxxxxxx 	0x03ffffff
uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx     0xffffffff

10xxxxxx  0x80 // bit

11xxxxxx  0xc0

110yyyyy  0xc0
1110zzzz  0xe0
11110www  0xf0
111110vv  0xf8
1111110u  0xfc
11111110  0xfe

*/


//*******************************************************************

#define UTF8_CHARSIGN 0x80 // 10xxxxxx
#define UTF8_CHARMASK 0x3f // 00111111

#define UTF8_HEADSIGN 0xc0 // 11xxxxxx

#define UTF8_HEADSIGN_2   0xc0 // 110yyyyy
#define UTF8_HEADSIGN_3   0xe0 // 1110zzzz
#define UTF8_HEADSIGN_4   0xf0 // 11110www
#define UTF8_HEADSIGN_5   0xf8 // 111110vv
#define UTF8_HEADSIGN_6   0xfc // 1111110u
#define UTF8_HEADSIGN_7   0xfe // 11111110

#define UTF8_HEADMASK_1   0x7f // 01111111 (mask 0xxxxxxx)
#define UTF8_HEADMASK_2   0x1f // 00011111 (mask 110yyyyy)
#define UTF8_HEADMASK_3   0x0f // 00001111 (mask 1110zzzz)
#define UTF8_HEADMASK_4   0x07 // 00000111 (mask 11110www)
#define UTF8_HEADMASK_5   0x03 // 00000011 (mask 111110vv)
#define UTF8_HEADMASK_6   0x01 // 00000001 (mask 1111110u)
#define UTF8_HEADMASK_7   0x00 // 00000000 (mask 11111110)


#define UTF8L_1     0x0000007f // 00000000 00000000 00000000 0xxxxxxx
#define UTF8L_2     0x000007ff // 00000000 00000000 00000yyy yyxxxxxx
#define UTF8L_3     0x0000ffff // 00000000 00000000 zzzzyyyy yyxxxxxx
#define UTF8L_4     0x001fffff // 00000000 000wwwzz zzzzyyyy yyxxxxxx
#define UTF8L_5     0x03ffffff // 000000vv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_6     0xffffffff // uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx

//*******************************************************************
static int ucsx2utf8f(uint32_t ucsx,utf8char_t *buf, utf8f_size_t buflen)
{
   if (ucsx<=UTF8L_1)
   {
      if (buflen<1) return 0;
      buf[0]=(utf8char_t)ucsx;
      return 1;
   }
   else if (ucsx<=UTF8L_2)
   {
      if (buflen<2) return 0;
      buf[0]=(utf8char_t)((ucsx>>6)|UTF8_HEADSIGN_2);
      buf[1]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 2;
   }
   else if (ucsx<=UTF8L_3)
   {
      if (buflen<3) return 0;
      buf[0]=(utf8char_t)((ucsx>>12)|UTF8_HEADSIGN_3);
      buf[1]=(utf8char_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 3;
   }
   else if (ucsx<=UTF8L_4)
   {
      if (buflen<4) return 0;
      buf[0]=(utf8char_t)((ucsx>>(18))|UTF8_HEADSIGN_4);
      buf[1]=(utf8char_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8char_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 4;
   }
   else if (ucsx<=UTF8L_5)
   {
      if (buflen<5) return 0;
      buf[0]=(utf8char_t)((ucsx>>(24))|UTF8_HEADSIGN_5);
      buf[1]=(utf8char_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8char_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8char_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 5;
   }
   else if (ucsx<=UTF8L_6)
   {
      if (buflen<6) return 0;
      buf[0]=(utf8char_t)((ucsx>>(30))|UTF8_HEADSIGN_5);
      buf[1]=(utf8char_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8char_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8char_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8char_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[5]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 6;
   }
   
   if (buflen<7) return 0;
   buf[0]=UTF8_HEADSIGN_7;
   buf[1]=(utf8char_t)(((ucsx>>30)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[2]=(utf8char_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[3]=(utf8char_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[4]=(utf8char_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[5]=(utf8char_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[6]=(utf8char_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
   return 7;
}


static const char lenFromUtfHead[256] = { // 0= invalid header (0b10xxxxxx and 0b11111111).
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x00-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x10-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x20-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x30-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x40-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x50-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x60-
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x70-0x7f

   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x80(0b10000000)-
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x90-
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xa0-
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xb0-0xbf(0b101111111)

   2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xc0(0b11000000)-
   2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xd0-0xdf(0b11011111)
   3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, // 0xe0(0b11100000)-0xef(0b11101111)
   4,4,4,4,4,4,4,4,                 // 0xf0(0b11110000)-0xf7(0b11110111)
   5,5,5,5,                         // 0xf8(0b11111000)-0xfb(0b11111011)
   6,6,                             // 0xfc(0b11111100)-0xfd(0b11111101)
   7,                               // 0xfe(0b11111110)-0xfe(0b11111110)
   0,                               // 0xff(0b11111111)-0xff(0b11111111)
};


//*******************************************************************
#define utf8fpNext(buf,i); ((buf)->buf+=i)

//*******************************************************************
// A getUCSxFromUtf8f_n(): felteszi, hogy van elég hely és a közbenső
// (10xxxxxx bájtokban) a fejlécek rendben vannak.
#define getUcsxFromUtf8f_2(p) ((((p)&UTF8_HEADMASK_2)<< 6)|((p)+1)&UTF8_CHARMASK)
#define getUcsxFromUtf8f_3(p) ((((p)&UTF8_HEADMASK_3)<<12)|   \
                               ((((p)+1)&UTF8_CHARMASK)<<6)|\
                               ((p)+2)&UTF8_CHARMASK)
#define getUcsxFromUtf8f_4(p) ((((p)&UTF8_HEADMASK_4)<<18)|   \
                               ((((p)+1)&UTF8_CHARMASK)<<12)|\
                               ((((p)+2)&UTF8_CHARMASK)<<6)|\
                               ((p)+3)&UTF8_CHARMASK)

#define getUcsxFromUtf8f_5(p) ((((p)&UTF8_HEADMASK_5)<<24)|   \
                               ((((p)+1)&UTF8_CHARMASK)<<18)|\
                               ((((p)+2)&UTF8_CHARMASK)<<12)|\
                               ((((p)+3)&UTF8_CHARMASK)<<6)|\
                               ((p)+4)&UTF8_CHARMASK)

#define getUcsxFromUtf8f_6(p) ((((p)&UTF8_HEADMASK_6)<<30)|   \
                               ((((p)+1)&UTF8_CHARMASK)<<24)|\
                               ((((p)+2)&UTF8_CHARMASK)<<18)|\
                               ((((p)+3)&UTF8_CHARMASK)<<12)|\
                               ((((p)+4)&UTF8_CHARMASK)<<6)|\
                               ((p)+5)&UTF8_CHARMASK)
#define getUcsxFromUtf8f_7(p) (0/*(((p)&UTF8_HEADMASK_7)<<36)*/|   \
                               ((((p)+1)&UTF8_CHARMASK)<<30)|\
                               ((((p)+2)&UTF8_CHARMASK)<<24)|\
                               ((((p)+3)&UTF8_CHARMASK)<<18)|\
                               ((((p)+4)&UTF8_CHARMASK)<<12)|\
                               ((((p)+5)&UTF8_CHARMASK)<<6)|\
                               ((p)+6)&UTF8_CHARMASK)

//*******************************************************************
static ucsf_t utf8nextcharfull(utf8fp *buf)
// Akkor hívja, amikor buf-ban biztosan van 8 hely.
{
   utf8char_t h;
   ucsf_t ucsx;

   switch(lenFromUtfHead[h=*buf->buf])
   {
   case 0: buf->status=UTF8FS_INVALID;return UTF8F_CHECK; // Invalid az uft8f kód. 
   case 1: utf8fpNext(buf,1);return h;
   case 2: ucsx=getUcsxFromUtf8f_2(buf-buf); utf8fpNext(buf,2); return ucsx;
   case 3: ucsx=getUcsxFromUtf8f_3(buf-buf); utf8fpNext(buf,3); return ucsx;
   case 4: ucsx=getUcsxFromUtf8f_4(buf-buf); utf8fpNext(buf,4); return ucsx;
   case 5: ucsx=getUcsxFromUtf8f_5(buf-buf); utf8fpNext(buf,5); return ucsx;
   case 6: ucsx=getUcsxFromUtf8f_6(buf-buf); utf8fpNext(buf,6); return ucsx;
   case 7: ucsx=getUcsxFromUtf8f_7(buf-buf); utf8fpNext(buf,7); return ucsx;
   default: buf->status=UTF8FS_INVALID;return UTF8F_CHECK; // Belső hiba.
   }
}


//*******************************************************************
ucsf_t utf8fnextchar(utf8fp *buf)
/* 
 Veszi a következő utf8f karaktert.
 Ha a visszatérési érték nem UTF8F_CHECK, akkor az az buf->ucsf karakter.
 Ha UTF8F_CHECK, akkor a buf->status-ban van, hogy miért állt le:
 buf->status:
  - UTF8FS_INVALID: Az utf8f karakter hibás.
  - UTF8FS_VALID: A karakter érvényes, és az ucsf-ben van.
  - Bármi más:  Az elemző további adatokra vár.: 
 */
{

   switch(buf->status)
   {
   case UTF8FS_INVALID: return UTF8F_CHECK;
   case UTF8FS_VALID  :
      if (buf->l>=8) return utf8nextcharfull(buf);
      buf->status=UTF8FS_S1;
      break;
   //default: // Minden más esetben folytatjuk onnan, ahol vagyunk.
   }
}

//*******************************************************************
