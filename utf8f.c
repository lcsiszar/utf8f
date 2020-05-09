//*******************************************************************
// utf8f.c: utf8f-ucsx konverzió.
//*******************************************************************

/*
 32 bites számokkal kódolt karaktersorozatokat kódol át utf8-ra és vissza
 egyértelműen. A 32 bites karaktereket ucsx karakternek hívja.
 Minden 32 bites számot lekódol.
 Egy utf8f karakter akkor érvényes kód, ha ucsx-re konvertálva, majd
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
#include <stdio.h>

#include "utf8f.h"

const char *utf8fp_version=UTF8F_VERSION;


//*******************************************************************
#define UTF8FMAXBYTELEN 7

//*******************************************************************
// #define UTF8FS_VALID   0
// #define UTF8FS_INVALID 1
// #define UTF8FS_EOB     2 // End Of Buffer

#define UTF8FS_S0      3
#define UTF8FS_S1      4
#define UTF8FS_S2      5
#define UTF8FS_S3      6
#define UTF8FS_S4      7
#define UTF8FS_S5      8
#define UTF8FS_S6      9
#define UTF8FS_S7     10


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

#define UTF8_CHARSIGN     0x80 // 10xxxxxx
#define UTF8_CHARMASK     0x3f // 00111111
#define UTF8_CHARSIGNMASK 0xc0 // 11000000

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
static int utf8headmasks[7]={
   UTF8_HEADMASK_1,UTF8_HEADMASK_2,UTF8_HEADMASK_3,UTF8_HEADMASK_4,
   UTF8_HEADMASK_5,UTF8_HEADMASK_6,UTF8_HEADMASK_7
};

#define UTF8L_1     0x0000007f // 00000000 00000000 00000000 0xxxxxxx
#define UTF8L_2     0x000007ff // 00000000 00000000 00000yyy yyxxxxxx
#define UTF8L_3     0x0000ffff // 00000000 00000000 zzzzyyyy yyxxxxxx
#define UTF8L_4     0x001fffff // 00000000 000wwwzz zzzzyyyy yyxxxxxx
#define UTF8L_5     0x03ffffff // 000000vv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_6     0x7fffffff // 0uvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_7     0xffffffff // uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx

//*******************************************************************
int ucsx2utf8f(uint32_t ucsx,utf8fchar_t *buf, utf8f_size_t buflen)
{
   // printf("ucsx: %u, UTF8L_1: %u\n",ucsx,UTF8L_1);
   if (ucsx<=UTF8L_1)
   {
      if (buflen<1) return 0;
      buf[0]=(utf8fchar_t)ucsx;
      return 1;
   }
   else if (ucsx<=UTF8L_2)
   {
      if (buflen<2) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>6)|UTF8_HEADSIGN_2);
      buf[1]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 2;
   }
   else if (ucsx<=UTF8L_3)
   {
      if (buflen<3) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>12)|UTF8_HEADSIGN_3);
      buf[1]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 3;
   }
   else if (ucsx<=UTF8L_4)
   {
      if (buflen<4) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>18)|UTF8_HEADSIGN_4);
      buf[1]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 4;
   }
   else if (ucsx<=UTF8L_5)
   {
      if (buflen<5) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>24)|UTF8_HEADSIGN_5);
      buf[1]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 5;
   }
   else if (ucsx<=UTF8L_6)
   {
      if (buflen<6) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>30)|UTF8_HEADSIGN_6);
      buf[1]=(utf8fchar_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[5]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 6;
   }
   
   if (buflen<7) return 0;
   buf[0]=UTF8_HEADSIGN_7;
   buf[1]=(utf8fchar_t)(((ucsx>>30)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[2]=(utf8fchar_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[3]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[4]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[5]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
   buf[6]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
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
#define utf8fpNext(up,i) ((up)->ibuf+=i)

//*******************************************************************
#define utf8fpCheck_1i(p,i) (UTF8_CHARSIGN==((p)[i]&UTF8_CHARSIGNMASK))

#define utf8fpCheck_2(p) utf8fpCheck_1i(p,1)
#define utf8fpCheck_3(p) (utf8fpCheck_2(p) && utf8fpCheck_1i(p,2))
#define utf8fpCheck_4(p) (utf8fpCheck_3(p) && utf8fpCheck_1i(p,3))
#define utf8fpCheck_5(p) (utf8fpCheck_4(p) && utf8fpCheck_1i(p,4))
#define utf8fpCheck_6(p) (utf8fpCheck_5(p) && utf8fpCheck_1i(p,5))

// Mj.: 7 hosszú utf8-nál a második bájt középső 6 bitje nulla kell legyen.
#define utf8fpCheck_7(p) (utf8fpCheck_6(p) && utf8fpCheck_1i(p,6) && ((p)[1]&0x3c)==0)

//*******************************************************************
// A getUCSxFromUtf8f_n(): felteszi, hogy van elég hely és a közbenső
// (10xxxxxx bájtokban) a fejlécek rendben vannak.
#define getUcsxFromUtf8f_2(p) (((((p)[0])&UTF8_HEADMASK_2)<< 6)|(((p)[1])&UTF8_CHARMASK))
#define getUcsxFromUtf8f_3(p) (((((p)[0])&UTF8_HEADMASK_3)<<12)|   \
                               ((((p)[1])&UTF8_CHARMASK)<<6)|\
                                (((p)[2])&UTF8_CHARMASK))
#define getUcsxFromUtf8f_4(p) (((((p)[0])&UTF8_HEADMASK_4)<<18)|   \
                               ((((p)[1])&UTF8_CHARMASK)<<12)|\
                               ((((p)[2])&UTF8_CHARMASK)<<6)|\
                                (((p)[3])&UTF8_CHARMASK))

#define getUcsxFromUtf8f_5(p) (((((p)[0])&UTF8_HEADMASK_5)<<24)|   \
                               ((((p)[1])&UTF8_CHARMASK)<<18)|\
                               ((((p)[2])&UTF8_CHARMASK)<<12)|\
                               ((((p)[3])&UTF8_CHARMASK)<<6)|\
                                (((p)[4])&UTF8_CHARMASK))

#define getUcsxFromUtf8f_6(p) (((((p)[0])&UTF8_HEADMASK_6)<<30)|   \
                               ((((p)[1])&UTF8_CHARMASK)<<24)|\
                               ((((p)[2])&UTF8_CHARMASK)<<18)|\
                               ((((p)[3])&UTF8_CHARMASK)<<12)|\
                               ((((p)[4])&UTF8_CHARMASK)<<6)|\
                                (((p)[5])&UTF8_CHARMASK))
#define getUcsxFromUtf8f_7(p) (0/*((((p)[0])&UTF8_HEADMASK_7)<<36)*/|   \
                               ((((p)[1])&UTF8_CHARMASK)<<30)|\
                               ((((p)[2])&UTF8_CHARMASK)<<24)|\
                               ((((p)[3])&UTF8_CHARMASK)<<18)|\
                               ((((p)[4])&UTF8_CHARMASK)<<12)|\
                               ((((p)[5])&UTF8_CHARMASK)<<6)|\
                                (((p)[6])&UTF8_CHARMASK))

//*******************************************************************
static ucsx_t getUcsxFromCode8(utf8fp *up,utf8fchar_t c)
{
#ifdef OLD
   if (c<128) return c;
   if (up->codetable==NULL) return c;
   return up->codetable[c-128];
#endif
   if (up->codetable==NULL) return c;
   return up->codetable[c];
}

//*******************************************************************
static ucsx_t convucsx(utf8fp *up,ucsx_t ucsx)
// ucsx karater konvertálás.

// Mj.: A 8 bites karaktereket nem ezzel konvertáljuk, hanem a
// codetable-vel.

{
   if (up->ucsxconverter==NULL) return ucsx;
   return (*up->ucsxconverter)(ucsx,up);
}

//*******************************************************************
static ucsx_t invalidutf8f_skip(utf8fp *up,utf8fchar_t c, int step)
// Amikor invalid az utf8 szekvencia következő bájtja és csak egyet vagy
// nullát kell lépni.
{
   // fprintf(stderr,"invalidutf8f_skip: mode: %d,c: %d(0x%x),step: %d\n",up->mode,c,c,step);
   switch(up->mode)
   {
   case UTF8FM_UTF8FFIXED:
      up->state=UTF8FS_INVALID;
      up->ucsx=c;
   return UCSX_CHECK;
   case UTF8FM_UTF8FMIXCODE8:
      // fprintf(stderr,"mixcode8: step: %d\n",step);
      up->state=UTF8FS_VALID;
      up->ucsx=getUcsxFromCode8(up,c);
      utf8fpNext(up,step);
      return up->ucsx;
   case UTF8FM_CODE8:
   case UTF8FM_CODE8FIXED:
      up->code8=UTF8FCMCR_CODE8;
      up->state=UTF8FS_VALID;
      up->ucsx=getUcsxFromCode8(up,c);
      utf8fpNext(up,step);
      return up->ucsx;
   }
   // Ide nem jöhet.
   up->state=UTF8FS_INVALID;
   up->ucsx=c;
   return UCSX_CHECK;
}

//*******************************************************************
static ucsx_t invalidutf8f_pos(utf8fp *up,utf8fchar_t c,int pos)
// Amikor invalid az egész utf8 szekvencia és a pos-nál derül ki.
// Az ibuf a pos-ra mutat.
{
   // fprintf(stderr,"invalidutf8f_pos: mode: %d,c: %d(0x%x),pos: %d\n",up->mode,c,c,pos);
   switch(up->mode)
   {
   case UTF8FM_UTF8FFIXED:
      up->state=UTF8FS_INVALID;
      up->ucsx=c;
   return UCSX_CHECK;
   case UTF8FM_UTF8FMIXCODE8:
      up->state=UTF8FS_VALID;
      if (pos==0)
      {
         up->ucsx=getUcsxFromCode8(up,c);
         utf8fpNext(up,1);
         return up->ucsx;
      }
      up->code8=UTF8FCMCR_UTF8FCBUF;
      up->ucsx=getUcsxFromCode8(up,up->cbuf[0]);
      up->cbufpos=1;
      up->cbuflen=pos;
      return up->ucsx;
   case UTF8FM_CODE8:
   case UTF8FM_CODE8FIXED:
      up->state=UTF8FS_VALID;
      if (pos==0)
      {
         up->ucsx=getUcsxFromCode8(up,c);
         utf8fpNext(up,1);
         up->code8=UTF8FCMCR_CODE8;
         return up->ucsx;
      }
      up->code8=UTF8FCMCR_UTF8FCODE8;
      up->ucsx=getUcsxFromCode8(up,up->cbuf[0]);
      up->cbufpos=1;
      up->cbuflen=pos;
      return up->ucsx;
   }
   // Ide nem jöhet.
   up->state=UTF8FS_INVALID;
   up->ucsx=c;
   return UCSX_CHECK;
}

//*******************************************************************
// #define P_UTF8FP_NEXTCHARFULL
#ifndef P_UTF8FP_NEXTCHARFULL

#define UTF8FP_NEXTCHARFULL utf8fp_nextcharfull

#else

#define UTF8FP_NEXTCHARFULL _utf8fp_nextcharfull
#define X_UTF8FP_NEXTCHARFULL utf8fp_nextcharfull

static ucsx_t UTF8FP_NEXTCHARFULL(utf8fp *up);
//*******************************************************************
ucsx_t X_UTF8FP_NEXTCHARFULL(utf8fp *up)
{
   ucsx_t w;

   fprintf(stderr,"utf8fp_nextcharfull(e): m: %d, s: %d, code8: %d\n",
      up->mode,up->state,up->code8);
   w=UTF8FP_NEXTCHARFULL(up);
   fprintf(stderr,"utf8fp_nextcharfull(u): m: %d, s: %d, code8: %d,ucsx: %d(0x%x)\n",
      up->mode,up->state,up->code8,w,w);
   return w;
}
#endif // P_UTF8FP_NEXTCHARFULL

//*******************************************************************
static ucsx_t UTF8FP_NEXTCHARFULL(utf8fp *up)
// Akkor hívja, amikor buf-ban biztosan van 7 hely.
// A state-nek UTF8FS_VALID-nak kell lennie.
{
   utf8fchar_t h;
   ucsx_t ucsx;

   // fprintf(stderr,"nextcharfull: %x, l: %d,check: %d, 1: %x, chs: %x\n",
   //    *up->ibuf,lenFromUtfHead[h=*up->ibuf],
   //    utf8fpCheck_2(up->ibuf),
   //    up->ibuf[1],
   //    (up->ibuf[1])&UTF8_CHARSIGN);
   switch(up->utf8flen=lenFromUtfHead[h=*up->ibuf])
   {
   case 0: goto invalid;
   case 1: utf8fpNext(up,1);return h;
   case 2: if(!utf8fpCheck_2(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_2(up->ibuf);
           if (!(ucsx&~UTF8L_1)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,2);
   return convucsx(up,ucsx);
   case 3: if(!utf8fpCheck_3(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_3(up->ibuf);
           if (!(ucsx&~UTF8L_2)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,3);
   return convucsx(up,ucsx);
   case 4: if(!utf8fpCheck_4(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_4(up->ibuf);
           if (!(ucsx&~UTF8L_3)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,4);
   return convucsx(up,ucsx);
   case 5: if(!utf8fpCheck_5(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_5(up->ibuf);
           if (!(ucsx&~UTF8L_4)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,5);
   return convucsx(up,ucsx);
   case 6: if(!utf8fpCheck_6(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_6(up->ibuf);
           if (!(ucsx&~UTF8L_5)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,6);
   return convucsx(up,ucsx);
   case 7: if(!utf8fpCheck_7(up->ibuf)) goto invalid;
           ucsx=getUcsxFromUtf8f_7(up->ibuf);
           if (!(ucsx&~UTF8L_6)) goto invalid; // Nem minimál kódolás.
           utf8fpNext(up,7); 
           if (ucsx==UCSX_CHECK) up->ucsx=ucsx;
   return convucsx(up,ucsx);
   }
   // Belső hiba.
   return invalidutf8f_skip(up,0,1);

   invalid: // Invalid az uft8f kód.
   return invalidutf8f_skip(up,*up->ibuf,1);
}

//*******************************************************************
#define store2cbuf(up,pos,c) ((up)->cbuf[(pos)]=(c))

//*******************************************************************
// #define P_UTF8FP_NEXTCHARPART
#ifndef P_UTF8FP_NEXTCHARPART

#define UTF8FP_NEXTCHARPART utf8fp_nextcharpart

#else

#define UTF8FP_NEXTCHARPART _utf8fp_nextcharpart
#define X_UTF8FP_NEXTCHARPART utf8fp_nextcharpart

static ucsx_t UTF8FP_NEXTCHARPART(utf8fp *up, int pos);
//*******************************************************************
ucsx_t X_UTF8FP_NEXTCHARPART(utf8fp *up, int pos)
{
   ucsx_t w;

   fprintf(stderr,"utf8fp_nextcharpart(e): m: %d,s: %d,code8: %d,pos: %d,l: %lu\n",
      up->mode,up->state,up->code8,pos,up->ebuf-up->ibuf);
   w=UTF8FP_NEXTCHARPART(up,pos);
   fprintf(stderr,"utf8fp_nextcharpart(u): m: %d,s: %d,code8: %d,pos: %d,l: %ld,ucsx: %d(0x%x)\n",
      up->mode,up->state,up->code8,pos,up->ebuf-up->ibuf,w,w);
   return w;
}
#endif // P_UTF8FP_NEXTCHARPART

//*******************************************************************
static ucsx_t UTF8FP_NEXTCHARPART(utf8fp *up, int pos)
// Az utf kódban a pos pozíciónál tartunk.
// Ha a pos nem nulla, akkor az up->utf8flen ki van töltve.
{

   if (up->ibuf>=up->ebuf) // No space
   {
      if (up->state==UTF8FS_VALID)
      {
         up->state=UTF8FS_EOB;
         return UCSX_CHECK;
      }
      // fprintf(stderr,"no space to cbuf: pos: %d\n",pos);
      // Át kell kapcsolni cbuf-ra.
      
      return invalidutf8f_pos(up,0,pos);
      // return UCSX_CHECK;
   }

   if (pos==0)
   {
      if (0==(up->utf8flen=lenFromUtfHead[*up->ibuf]))
      {
         return invalidutf8f_skip(up,*up->ibuf,1);
      }
      store2cbuf(up,pos,*up->ibuf);
      up->ucsx=(*up->ibuf)&(utf8headmasks[up->utf8flen-1]);
      pos++;
      up->ibuf++;
   }
   
   // Itt nem lehet else if(!)
   if (pos==1 && up->utf8flen==7 && ((*up->ibuf)&0x3c)!=0)
   {
      // 7 hosszú utf8-nál a második bájt középső 6 bitje nulla kell legyen.

      return invalidutf8f_skip(up,up->cbuf[0],0); // Ezt csak azért lehet
                                                  // megcsinálni, mert pos==1

   }
      
   for(;pos<up->utf8flen;)
   {
      // fprintf(stderr,"pos: %d\n",pos);
      if (up->ibuf>=up->ebuf) // No space
      {
         // fprintf(stderr,"no space: pos: %d\n",pos);
         up->state=UTF8FS_S0+pos;
         return UCSX_CHECK;
      }
      // if (up->utf8flen==7) fprintf(stderr,"utf8fp_nextcharpart: pos: %d, ibuf, 0x%x, %d\n",pos,*up->ibuf,utf8fpCheck_1i(up->ibuf,0));
      store2cbuf(up,pos,*up->ibuf);
      if (!utf8fpCheck_1i(up->ibuf,0))
      {
         // fprintf(stderr,"not enough len: pos: %d\n",pos);
         return invalidutf8f_pos(up,0,pos);
      }
      up->ucsx=((up->ucsx)<<6)|((*up->ibuf)&UTF8_CHARMASK);
      pos++;
      up->ibuf++;
   }
   // if (up->utf8flen==7) fprintf(stderr,"utf8fp_nextcharpart: ucsx: %u, 0x%x\n",up->ucsx,up->ucsx);

   if (up->utf8flen>1)
   {
      static unsigned masks[]={UTF8L_1,UTF8L_2,UTF8L_3,UTF8L_4,UTF8L_5,UTF8L_6};

      // fprintf(stderr,"len(masks): %d, utf8flen-2: %d\n",sizeof(masks)/sizeof(*masks),up->utf8flen-2);
      if (!(up->ucsx&~(masks[up->utf8flen-2])))
      { // Nem minimál kódolás.
         return invalidutf8f_pos(up,0,pos);
      } 
   }

   up->state=UTF8FS_VALID;
   return convucsx(up,up->ucsx);
}

//*******************************************************************
#ifdef NEMKELL
static ucsx_t utf8fp_nextcharpart(utf8fp *up, int pos)
{
   ucsx_t w;
   fprintf(stderr,"utf8fp_nextcharpart(s): s: %d,pos: %d, ibuf: %p, ebuf: %p\n",
          up->state,pos,up->ibuf,up->ebuf);

   w=_utf8fp_nextcharpart(up,pos);
   fprintf(stderr,"utf8fp_nextcharpart(e): r: %xu, s: %d,pos: %d, ibuf: %p, ebuf: %p\n",
          w,up->state,pos,up->ibuf,up->ebuf);

   return w;
}
#endif

//*******************************************************************
void utf8fp_setup(utf8fp *up,utf8fchar_t *buf)
{
   up->state=UTF8FS_EOB;
   up->mode=UTF8FM_UTF8FFIXED;
   up->code8=UTF8FCMCR_UTF8F;
   up->crlfmode=UTF8FCRLFMODE_NOCRLFCONV;
   up->crlfstate=UTF8FCS_NONE;
   up->codetable=NULL;
   up->ucsxconverter=NULL;
   up->ucsxconverterdata=NULL;
   up->ucsx=0;

   up->buf=up->ibuf=buf;
   up->ebuf=buf;

   up->cbufpos=0;
   up->cbuflen=0;

}

//*******************************************************************
void utf8fp_setmode(utf8fp *up,int mode,ucsx_t codetable[256])
{

   switch(mode)
   {
   case UTF8FM_UTF8FFIXED: up->mode=UTF8FM_UTF8FFIXED; break;
   case UTF8FM_UTF8FMIXCODE8:
      up->mode=UTF8FM_UTF8FMIXCODE8;
      up->codetable=codetable;
   break;
   case UTF8FM_CODE8:
      up->mode=UTF8FM_CODE8;
      up->codetable=codetable;
   break;
   case UTF8FM_CODE8FIXED:
      up->code8=UTF8FCMCR_CODE8;
      up->mode=UTF8FM_CODE8;
      up->codetable=codetable;
   break;
   }
}

//*******************************************************************
void utf8fp_setucsxconverter(utf8fp *up,utf8f_ucsxconverter ucsxconverter)
{
   up->ucsxconverter=ucsxconverter;
}

//*******************************************************************
void *utf8fp_getucsxconverterdata(utf8fp *up)
{
   return up->ucsxconverterdata;
}

//*******************************************************************
void utf8fp_setucsxconverterdata(utf8fp *up,void *ucsxconverterdata)
{
   up->ucsxconverterdata=ucsxconverterdata;
}

//*******************************************************************
void utf8fp_setcrlfmode(utf8fp *up,int crlfmode)
// Ha a crlfmode nem létezik, nem állítja be.
{
   switch(crlfmode)
   {
   case UTF8FCRLFMODE_NOCRLFCONV:
   case UTF8FCRLFMODE_CRLF2LF   :
   case UTF8FCRLFMODE_LF2CRLF   :
      up->crlfmode=crlfmode;
   }

}

//*******************************************************************
void utf8fp_cont(utf8fp *up,utf8fchar_t *buf,utf8f_size_t l)
{
   up->buf=up->ibuf=buf;
   up->ebuf=buf+l;
}

//*******************************************************************
void utf8fp_cont_l(utf8fp *up,utf8f_size_t l)
{
   up->ebuf=up->buf+l;
   up->ibuf=up->buf;
}

//*******************************************************************
void utf8fp_nextbyte(utf8fp *up) // Jump next byte and reset state.
{
   up->ucsx=0;
   if (up->ibuf<up->ebuf) up->ibuf++;
   up->state=up->ibuf<up->ebuf?UTF8FS_VALID:UTF8FS_EOB;
}

//*******************************************************************
// #define P_UTF8FP_NEXTCHAR_STREAM
#ifndef P_UTF8FP_NEXTCHAR_STREAM

#define UTF8FP_NEXTCHAR_STREAM utf8fp_nextchar_stream

#else

#define UTF8FP_NEXTCHAR_STREAM _utf8fp_nextchar_stream
#define X_UTF8FP_NEXTCHAR_STREAM utf8fp_nextchar_stream

//*******************************************************************
ucsx_t X_UTF8FP_NEXTCHAR_STREAM(utf8fp *up) // Nincs crlf konverzió.
{
   extern ucsx_t _utf8fp_nextchar_stream(utf8fp *up);
   ucsx_t w;

   fprintf(stderr,"utf8fp_nextchar_stream(e): m: %d, s: %d, code8: %d\n",
      up->mode,up->state,up->code8);
   w=UTF8FP_NEXTCHAR_STREAM(up);
   fprintf(stderr,"utf8fp_nextchar_stream(u): m: %d, s: %d, code8: %d,ucsx: %d(0x%x)\n",
      up->mode,up->state,up->code8,w,w);
   return w;
}

#endif // P_UTF8FP_NEXTCHAR_STREAM

//*******************************************************************
ucsx_t UTF8FP_NEXTCHAR_STREAM(utf8fp *up) // Nincs crlf konverzió.
/* 

 Veszi a következő utf8f karaktert.

 Ha a visszatérési érték nem UCSX_CHECK, akkor a visszatérési érték az
 olvasott ucsx karakter.

 Ha UCSX_CHECK, akkor a up->state-ban van, hogy mi a helyzet:

 up->state:
  - UTF8FS_INVALID: Az utf8f karakter hibás.

  - UTF8FS_VALID: A karakter érvényes, olvasott ucsx karakter a
    visszatérési érték, azaz az UCSX_CHECK értéke.  Ugyanez az olvasott
    ucsx karakter benne van az up->ucsx-ben is.

  - UTF8FS_EOB: UTF8 bájtsorozat határán vagyunk, a bufferben nincs több
    adat.

  - Bármi más:  Egy UTF8 bájt sorozat közepén vagyunk, a bufferben nincs
    több adat, az elemző további adatokra vár.

*/
{
   // fprintf(stderr,"utf8fp_nextchar_stream: s: %d, code8: %d\n",up->state,up->code8);
   switch(up->code8)
   {
   case UTF8FCMCR_CODE8:
      code8:
      if (up->ibuf>=up->ebuf) // No space
      {
         if (up->state==UTF8FS_VALID) up->state=UTF8FS_EOB;
         return UCSX_CHECK;
      }
      up->state=UTF8FS_VALID;
      up->ucsx=getUcsxFromCode8(up,*up->ibuf);
      utf8fpNext(up,1);
   return up->ucsx;
   case UTF8FCMCR_UTF8FCBUF:
      if (up->cbufpos<up->cbuflen)
      {
         up->state=UTF8FS_VALID;
         up->ucsx=getUcsxFromCode8(up,up->cbuf[up->cbufpos]);
         up->cbufpos++;
         return up->ucsx;
      }
      up->state=UTF8FS_VALID;
      up->code8=UTF8FCMCR_UTF8F;
   break;
   case UTF8FCMCR_UTF8FCODE8:
      if (up->cbufpos<up->cbuflen)
      {
         up->state=UTF8FS_VALID;
         up->ucsx=getUcsxFromCode8(up,up->cbuf[up->cbufpos]);
         up->cbufpos++;
         return up->ucsx;
      }
      up->state=UTF8FS_VALID;
      up->code8=UTF8FCMCR_CODE8;
   goto code8;
   }

   switch(up->state)
   {
   case UTF8FS_INVALID: return UCSX_CHECK;
   case UTF8FS_VALID  :
      if (up->ebuf-up->ibuf>=UTF8FMAXBYTELEN) return utf8fp_nextcharfull(up);
   return utf8fp_nextcharpart(up,0);
   case UTF8FS_EOB    :
      up->state=UTF8FS_VALID;
      if (up->ebuf-up->ibuf>=UTF8FMAXBYTELEN) return utf8fp_nextcharfull(up);
   return utf8fp_nextcharpart(up,0);
   case UTF8FS_S0: 
   case UTF8FS_S1: 
   case UTF8FS_S2: 
   case UTF8FS_S3: 
   case UTF8FS_S4: 
   case UTF8FS_S5: 
   case UTF8FS_S6: 
   case UTF8FS_S7: 
   return utf8fp_nextcharpart(up,up->state-UTF8FS_S0);
   default: up->state=UTF8FS_INVALID;up->ucsx=0;return UCSX_CHECK; // Belső hiba.
   }
}

//*******************************************************************
#define UCSX_CR 13
#define UCSX_LF 10

//*******************************************************************
ucsx_t utf8fp_nextchar_line(utf8fp *up)
// Hívja az utf8fp_nextchar_stream-t, majd megcsinálja a crlf konverziót.
{
   ucsx_t ucsx;

   // fprintf(stderr,"crlfmode: %d, crlfstate: %d\n",up->crlfmode,up->crlfstate);
   if (up->crlfmode==UTF8FCRLFMODE_NOCRLFCONV) return utf8fp_nextchar_stream(up);

   switch(up->crlfstate)
   {
   case UTF8FCS_UCSX:
      up->crlfstate=UTF8FCS_NONE;
   return up->crlfucsx;
   case UTF8FCS_CR:
      up->crlfstate=UTF8FCS_NONE;
      ucsx=UCSX_CR;
   break;
   case UTF8FCS_INVALID:
      up->crlfstate=UTF8FCS_NONE;
   return UCSX_CHECK;
   case UTF8FCS_EOB:
      up->crlfstate=UTF8FCS_NONE;
      ucsx=UCSX_CR;
   break;
   default:
      ucsx=utf8fp_nextchar_stream(up);
   break;
   }

   // fprintf(stderr,"ucsx: %d, crlfmode: %d, crlfstate: %d\n",ucsx,up->crlfmode,up->crlfstate);

   if (UCSX_CR==ucsx)
   {
      if (UCSX_LF==(ucsx=utf8fp_nextchar_stream(up)))
      {
         // cr,lf
         if (up->crlfmode==UTF8FCRLFMODE_CRLF2LF) return UCSX_LF;
         
         // Adni kell egy cr-t, majd a következő hívásra egy lf-et.
         up->crlfstate=UTF8FCS_UCSX;
         up->crlfucsx=UCSX_LF;
         return UCSX_CR;
      }
      else if (ucsx==UCSX_CR)
      {
         // cr,cr
         if (up->crlfmode==UTF8FCRLFMODE_CRLF2LF)
         {
            up->crlfstate=UTF8FCS_CR;
            return UCSX_CR;
         }
         else // if (up->crlfmode==UTF8CRLFMODE_LF2CRLF)
         {
            up->crlfstate=UTF8FCS_CR;
            return UCSX_CR;
         }
      }
      else if (ucsx==UCSX_CHECK)
      {
         if (up->state==UTF8FS_VALID)
         {
            up->crlfstate=UTF8FCS_UCSX;
            up->crlfucsx=ucsx;
            return UCSX_CR;
         }
         else if (up->state==UTF8FS_INVALID)
         {
            up->crlfstate=UTF8FCS_INVALID;
            // up->crlfucsx=ucsx;
            return UCSX_CR;
         }
         else if (up->state==UTF8FS_EOB)
         {
            up->crlfstate=UTF8FCS_EOB;
            up->crlfucsx=UCSX_CR;
            return UCSX_CHECK;
         }
         return ucsx;
      }
      else // Normál ucsx karakter.
      {
         up->crlfstate=UTF8FCS_UCSX;
         up->crlfucsx=ucsx;
         return UCSX_CR;
      }
   }
   else if (UCSX_LF==ucsx)
   {
      if (up->crlfmode==UTF8FCRLFMODE_CRLF2LF)
      {
         return UCSX_LF;
      }
      else // if (up->crlfmode==UTF8FCRLFMODE_LF2CRLF)
      {
         up->crlfstate=UTF8FCS_UCSX;
         up->crlfucsx=UCSX_LF;
         return UCSX_CR;
      }
   }
   return ucsx;
}

#ifdef KESOBB
//*******************************************************************
// #define P_UTF8FP_NEXTCHAR8_STREAM
#ifndef P_UTF8FP_NEXTCHAR8_STREAM

#define UTF8FP_NEXTCHAR8_STREAM utf8fp_nextchar8_stream

#else

#define UTF8FP_NEXTCHAR8_STREAM _utf8fp_nextchar8_stream
#define X_UTF8FP_NEXTCHAR8_STREAM utf8fp_nextchar8_stream

//*******************************************************************
ucsx_t X_UTF8FP_NEXTCHAR8_STREAM(utf8fp *up) // Nincs crlf konverzió.
{
   extern ucsx_t _utf8fp_nextchar8_stream(utf8fp *up);
   ucsx_t w;

   fprintf(stderr,"utf8fp_nextchar8_stream(e): m: %d, s: %d, code8: %d\n",
      up->mode,up->state,up->code8);
   w=UTF8FP_NEXTCHAR8_STREAM(up);
   fprintf(stderr,"utf8fp_nextchar8_stream(u): m: %d, s: %d, code8: %d,ucsx: %d(0x%x)\n",
      up->mode,up->state,up->code8,w,w);
   return w;
}

#endif // P_UTF8FP_NEXTCHAR8_STREAM

//*******************************************************************
ucsx_t UTF8FP_NEXTCHAR8_STREAM(utf8fp *up) // Nincs crlf konverzió.
/* 

 Veszi a következő utf8f karaktert.

 Ha a visszatérési érték nem UCSX_CHECK, akkor a visszatérési érték az
 olvasott ucsx karakter.

 Ha UCSX_CHECK, akkor a up->state-ban van, hogy mi a helyzet:

 up->state:
  - UTF8FS_INVALID: Az utf8f karakter hibás.

  - UTF8FS_VALID: A karakter érvényes, olvasott ucsx karakter a
    visszatérési érték, azaz az UCSX_CHECK értéke.  Ugyanez az olvasott
    ucsx karakter benne van az up->ucsx-ben is.

  - UTF8FS_EOB: UTF8 bájtsorozat határán vagyunk, a bufferben nincs több
    adat.

  - Bármi más:  Egy UTF8 bájt sorozat közepén vagyunk, a bufferben nincs
    több adat, az elemző további adatokra vár.

*/
{
   // fprintf(stderr,"utf8fp_nextchar8_stream: s: %d, code8: %d\n",up->state,up->code8);
   switch(up->code8)
   {
   case UTF8FCMCR_CODE8:
      code8:
      if (up->ibuf>=up->ebuf) // No space
      {
         if (up->state==UTF8FS_VALID) up->state=UTF8FS_EOB;
         return UCSX_CHECK;
      }
      up->state=UTF8FS_VALID;
      up->ucsx=getUcsxFromCode8(up,*up->ibuf);
      utf8fpNext(up,1);
   return up->ucsx;
   case UTF8FCMCR_UTF8FCBUF:
      if (up->cbufpos<up->cbuflen)
      {
         up->state=UTF8FS_VALID;
         up->ucsx=getUcsxFromCode8(up,up->cbuf[up->cbufpos]);
         up->cbufpos++;
         return up->ucsx;
      }
      up->state=UTF8FS_VALID;
      up->code8=UTF8FCMCR_UTF8F;
   break;
   case UTF8FCMCR_UTF8FCODE8:
      if (up->cbufpos<up->cbuflen)
      {
         up->state=UTF8FS_VALID;
         up->ucsx=getUcsxFromCode8(up,up->cbuf[up->cbufpos]);
         up->cbufpos++;
         return up->ucsx;
      }
      up->state=UTF8FS_VALID;
      up->code8=UTF8FCMCR_CODE8;
   goto code8;
   }

   switch(up->state)
   {
   case UTF8FS_INVALID: return UCSX_CHECK;
   case UTF8FS_VALID  :
      if (up->ebuf-up->ibuf>=UTF8FMAXBYTELEN) return utf8fp_nextchar8full(up);
   return utf8fp_nextchar8part(up,0);
   case UTF8FS_EOB    :
      up->state=UTF8FS_VALID;
      if (up->ebuf-up->ibuf>=UTF8FMAXBYTELEN) return utf8fp_nextchar8full(up);
   return utf8fp_nextchar8part(up,0);
   case UTF8FS_S0: 
   case UTF8FS_S1: 
   case UTF8FS_S2: 
   case UTF8FS_S3: 
   case UTF8FS_S4: 
   case UTF8FS_S5: 
   case UTF8FS_S6: 
   case UTF8FS_S7: 
   return utf8fp_nextchar8part(up,up->state-UTF8FS_S0);
   default: up->state=UTF8FS_INVALID;up->ucsx=0;return UCSX_CHECK; // Belső hiba.
   }
}
#endif // KESOBB
//*******************************************************************
 
