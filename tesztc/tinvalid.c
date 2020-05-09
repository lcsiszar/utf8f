//*******************************************************************
// tinvalid.c: Invalid utf8 sequence test.
//*******************************************************************

//*******************************************************************
#define DEBUG(x)
// #define DEBUG(x) x

//*******************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>  
#include <strings.h>
#include <unistd.h>
#include <string.h>
 
//*******************************************************************
#include "utf8f.h"
static int ucsx2utf8f_len(uint32_t ucsx,utf8f_size_t seqlen, utf8fchar_t *buf, utf8f_size_t buflen);
  
//*******************************************************************
#define UTF8L_1     0x0000007f // 00000000 00000000 00000000 0xxxxxxx
#define UTF8L_2     0x000007ff // 00000000 00000000 00000yyy yyxxxxxx
#define UTF8L_3     0x0000ffff // 00000000 00000000 zzzzyyyy yyxxxxxx
#define UTF8L_4     0x001fffff // 00000000 000wwwzz zzzzyyyy yyxxxxxx
#define UTF8L_5     0x03ffffff // 000000vv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_6     0x7fffffff // 0uvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx
#define UTF8L_7     0xffffffff // uuvvvvvv wwwwwwzz zzzzyyyy yyxxxxxx

//*******************************************************************
void printVMsg(const char *msg,const char *id, ucsx_t ucsx,int isInvalid)
{
   if (isInvalid)
      ;// fprintf(stderr,"%s-%s(right): %u,0x%x\n",msg,id,ucsx,ucsx);
   else
      fprintf(stderr,"%s-%s(error): %u,0x%x\n",msg,id,ucsx,ucsx);
}

//*******************************************************************
void printVMsgf(const char *msg,const char *id, ucsx_t ucsx,int isInvalid)
{
   if (isInvalid)
      fprintf(stderr,"%s-%s(right): %d\n",msg,id,ucsx);
   else
      fprintf(stderr,"%s-%s(error): %d\n",msg,id,ucsx);
}

//*******************************************************************
static int chkSeq_buf(const char *id,utf8fp *u,ucsx_t ucsx,utf8fchar_t *rbuf,utf8f_size_t l,int isInvalid)
{
   if (u->ibuf<rbuf+l)
   {
      // Valahol megállt középen.
      printVMsg("Not reach the end of sequence",id,ucsx,isInvalid);
      return 1;
   }
   else if (u->ibuf>rbuf+l)
   {
      // Túlfutott.
      printVMsg("Out of sequence",id,ucsx,isInvalid);
      return 1;
   }
   return 0;
}

//*******************************************************************
static int chkSeq_utf8flen(const char *id,utf8fp *u,ucsx_t ucsx,utf8fchar_t *rbuf,utf8f_size_t l,int isInvalid)
{
   if (!u->state==UTF8FS_VALID) return 0;
   if (u->utf8flen<l)
   {
      // Valahol megállt középen.
      printVMsg("Not reach the end of sequence",id,ucsx,isInvalid);
      return 1;
   }
   else if (u->utf8flen>l)
   {
      // Túlfutott.
      printVMsg("Out of sequence",id,ucsx,isInvalid);
      return 1;
   }
   return 0;
}

//*******************************************************************
static int chkSeq(const char *id,utf8fp *u,ucsx_t ucsx,utf8fchar_t *rbuf,utf8f_size_t l,int isInvalid)
{
   if (u->state==UTF8FS_VALID) return chkSeq_utf8flen(id,u,ucsx,rbuf,l,isInvalid);

   return chkSeq_buf(id,u,ucsx,rbuf,l,isInvalid);
}

//*******************************************************************
int chkucsx_full(ucsx_t oucsx,int isInvalid,utf8fchar_t *rbuf,utf8f_size_t l,utf8f_size_t bufsize)
// isInvalid: Az rbuf-ban levő utf8 karaktersorozat invalid-e vagy sem.
// Egy lépésben megy és a teljes buffert használja.
{
   utf8fp u;
   ucsx_t ucsx;

   utf8fp_setup(&u,NULL);

   utf8fp_cont(&u,rbuf,bufsize);
   // fprintf(stderr,"chkucsx_full: l: %ld, bufsize: %ld, isInvalid: %d\n",l,bufsize,isInvalid);
   if (UCSX_CHECK==(ucsx=utf8fp_nextchar_stream(&u)) && u.state!=UTF8FS_VALID)
   {
      if (u.state==UTF8FS_INVALID)
      {
         // if (!chkSeq("fi",&u,u.ucsx,rbuf,l,isInvalid))
         {
            printVMsg("Invalid byte","fi",ucsx,isInvalid);
         }
      }
      else
      {
         // Valahol megállt és rájött, hogy az utf8 szekvencia hibás.
         // if (!chkSeq("fx",&u,u.ucsx,rbuf,l,isInvalid))
         {
            printVMsg("Inner stop","fix",ucsx,isInvalid);
         }
      }
      return 1;
   }
   else
   {
      // Szerinte jó.
      DEBUG(fprintf(stderr,"norm: 0x%x\n",ucsx);)
      if (chkSeq("fv",&u,ucsx,rbuf,l,isInvalid))
      {
         // Nem az utf8 szekvencia végén áll.
         return 1;
      }
      
      if (ucsx==oucsx)
      {
         printVMsg("Normal ucsx is equal","fv",ucsx,!isInvalid);
         return 0;
      }
      else
      {
         printVMsg("Normal ucsx is different","fv",ucsx,isInvalid);
      }
      return 1;
   }
   return 0;
}

//*******************************************************************
int chkucsx_step(ucsx_t oucsx,int isInvalid,utf8fchar_t *rbuf,utf8f_size_t l,utf8f_size_t step)
// isInvalid: Az rbuf-ban levő utf8 karaktersorozat invalid-e vagy sem.
// step lépésenként megy végig a bufferen.
{
   utf8fp u;
   int i;
   ucsx_t ucsx=0;

   utf8fp_setup(&u,NULL);
   DEBUG(fprintf(stderr,"chkucsx_step: l: %ld, step: %ld, isInvalid: %d\n",l,step,isInvalid);)

   for(i=0;i<l;i+=step)
   {
      int rs=step<l-i?step:l-i;

      utf8fp_cont(&u,rbuf+i,rs);
      DEBUG(fprintf(stderr,"chkucsx_step(f): l: %ld, i:%d, rs: %d\n",l,i,rs);)
      while(1)
      {
         if (UCSX_CHECK==(ucsx=utf8fp_nextchar_stream(&u)) && u.state!=UTF8FS_VALID)
         {
            if (u.state==UTF8FS_INVALID)
            {
               printVMsg("Invalid byte","si",ucsx,isInvalid);
               return 1;
            }
            else
            {
               // Tovább kell olvasni, jöhet a következő 'step'.
               break;
            }
         }
         else
         {
            // Szerinte jó.
            DEBUG(fprintf(stderr,"norm: 0x%x, utf8len: %d, l: %ld\n",ucsx,u.utf8flen,l);)
            // if (u.ibuf<u.ebuf || i+rs<l)
            // if (u.utf8flen!=l)
            if (chkSeq("sv",&u,ucsx,rbuf,l,isInvalid))
            {
               // Szerinte jó, de nem értünk le a puffer (szekvencia) végére.
               // printVMsg("Not reach the end of buffer","sv",ucsx,isInvalid);
               return 1;
            }

            if (ucsx==oucsx)
            {
               printVMsg("Normal ucsx is equal","sv",ucsx,!isInvalid);
               return 0;
            }
            else
            {
               printVMsg("Normal ucsx is different","sv",ucsx,isInvalid);
            }
            return 1;
         }
      } 
   }
   DEBUG(fprintf(stderr,"State: %d,%x\n",u.state,ucsx);)
   if (u.state!=UTF8FS_EOB)
   {
      // Nem ért el a puffer végéig.
      printVMsg("Unexpected end of utf8 sequence","si",ucsx,isInvalid);
      return 1;
   }

   // Elért a puffer végéig.  Szerinte a puffer vége után is folytatódik az
   // utf8 szekvencia.
   printVMsg("Half broken utf8 sequence","sh",ucsx,isInvalid);
   fprintf(stderr,"   state: %d\n",u.state);
   return 1;
}

//*******************************************************************
void r1_full(ucsx_t ucsx)

// Előállítja az ucsx utf8 kódját, elrontja a bájtokat (egyenként) minden
// lehetséges módon és megnézi, hogy az elemző észreveszi-e.  full bufferes,
// azaz a buffer mindig 7 hosszú.

{
   static int bufsize=7;
   utf8fchar_t obuf[bufsize];
   utf8f_size_t l;
   int i,b;
   static int onlyFirst2bit=0;

   if (ucsx<128) return;

   memset(obuf,'\0',bufsize);

   if (0==(l=ucsx2utf8f(ucsx,obuf,sizeof(obuf)))) perror("error: Buffer overflow");

   DEBUG(fprintf(stderr,"utf8f(full):");)
   for(i=0;i<l;i++) {DEBUG(fprintf(stderr," %x",obuf[i]));}
   DEBUG(fprintf(stderr,"\n");)
   for(i=0;i<l;i++)
   {
      if (onlyFirst2bit)
      {
         for(b=0;b<4;b++)
         {
            utf8fchar_t rbuf[bufsize];
            memcpy(rbuf,obuf,bufsize);

            rbuf[i]=(obuf[i]&0x3f)|(((unsigned int)b)<<6);
            {
               // utf8fp u;

               // utf8fp_setup(&u,rbuf);
               DEBUG(fprintf(stderr,"ucsx(f2b): %u, i: %u, b: %u, obuf[i]: 0x%02x,rbuf[i]: 0x%02x\n",
                  ucsx,i,b,(unsigned)obuf[i],(unsigned)rbuf[i]);)
               // chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
               // if (rbuf[i]==obuf[i]) rbuf[i]=(rbuf[i]&1)?rbuf[i]-1:rbuf[i]+1;
               // if (i==1 && b==3 && bs==7)
               chkucsx_full(ucsx,rbuf[i]!=obuf[i],rbuf,l,bufsize);
            }
         }
      }
      else
      {
         for(b=0;b<256;b++)
         {
            utf8fchar_t rbuf[bufsize];
            memcpy(rbuf,obuf,bufsize);

            rbuf[i]=b;
            {
               // utf8fp u;

               // utf8fp_setup(&u,rbuf);
               
               DEBUG(fprintf(stderr,"utf8f(full):");)
               for(int j=0;j<l;j++) {DEBUG(fprintf(stderr," %x",obuf[j]));}
               DEBUG(fprintf(stderr,"\n");)
               DEBUG(fprintf(stderr,"utf8f(full) rbuf:");)
               for(int j=0;j<l;j++) {DEBUG(fprintf(stderr," %x",rbuf[j]));}
               DEBUG(fprintf(stderr,"\n");)
               DEBUG(fprintf(stderr,"ucsx(f8b): %u, 0x%x, i: %u, b: %u, obuf[i]: 0x%02x,rbuf[i]: 0x%02x\n",
                  ucsx,ucsx,i,b,(unsigned)obuf[i],(unsigned)rbuf[i]);)
               // chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
               // if (rbuf[i]==obuf[i]) rbuf[i]=(rbuf[i]&1)?rbuf[i]-1:rbuf[i]+1;
               // if (i==1 && b==3 && bs==7)
               chkucsx_full(ucsx,rbuf[i]!=obuf[i],rbuf,l,bufsize);
            }
         }
      }
   }
}

//*******************************************************************
void r1_len(ucsx_t ucsx)

// Előállítja az ucsx utf8 kódját, elrontja a bytokat minden
// lehetséges módon és megnézi, hogy az elemző észreveszi-e.

// len bufferes, azaz a buffer hossza mindig azonos az utf8 szekvencia
// hosszával.

{
   static int bufsize=7;
   utf8fchar_t obuf[bufsize];
   utf8f_size_t l;
   int i,b,bs;
   static int onlyFirst2bit=0;

   if (ucsx<128) return;

   memset(obuf,'\0',bufsize);

   if (0==(l=ucsx2utf8f(ucsx,obuf,sizeof(obuf)))) perror("error: Buffer overflow");

   DEBUG(fprintf(stderr,"utf8f(len):");)
   for(i=0;i<l;i++) {DEBUG(fprintf(stderr," %x",obuf[i]);)}
   DEBUG(fprintf(stderr,"\n");)
   if (onlyFirst2bit)
   {
      for(i=0;i<l;i++)
      {
         for(b=0;b<4;b++)
         {
            utf8fchar_t rbuf[bufsize];
            memcpy(rbuf,obuf,bufsize);

            rbuf[i]=(obuf[i]&0x3f)|(((unsigned int)b)<<6);
            for(bs=1;bs<=7;bs++)
            {
               // utf8fp u;

               // utf8fp_setup(&u,rbuf);
               DEBUG(fprintf(stderr,"ucsx: %u, i: %u, b: %u, bs: %u, obuf[i]: 0x%02x,rbuf[i]: 0x%02x\n",
                  ucsx,i,b,bs,(unsigned)obuf[i],(unsigned)rbuf[i]);)
               // chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
               // if (rbuf[i]==obuf[i]) rbuf[i]=(rbuf[i]&1)?rbuf[i]-1:rbuf[i]+1;
               // if (i==1 && b==3 && bs==7)
               chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
            }
         }
      }
   }
   else
   {
      for(i=0;i<l;i++)
      {
         for(b=0;b<256;b++)
         {
            utf8fchar_t rbuf[bufsize];
            memcpy(rbuf,obuf,bufsize);

            rbuf[i]=b;
            for(bs=1;bs<=7;bs++)
            {
               // utf8fp u;

               // utf8fp_setup(&u,rbuf);
               // fprintf(stderr,"b: %d\n",b);
               DEBUG(fprintf(stderr,"ucsx: %u, i: %u, b: %u, bs: %u, obuf[i]: 0x%02x,rbuf[i]: 0x%02x\n",
                  ucsx,i,b,bs,(unsigned)obuf[i],(unsigned)rbuf[i]);)
               // chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
               // if (rbuf[i]==obuf[i]) rbuf[i]=(rbuf[i]&1)?rbuf[i]-1:rbuf[i]+1;
               if (l==7 && i==1 && bs==1 && b==0x87)
               {
                  #define DEBUGX(x) DEBUG(x)
                  DEBUGX(fprintf(stderr,"utf8f(step,f8b):");)
                  for(int j=0;j<l;j++) {DEBUGX(fprintf(stderr," %x",obuf[j]));}
                  DEBUGX(fprintf(stderr,"\n");)
                  DEBUGX(fprintf(stderr,"utf8f(step,f8b) rbuf:");)
                  for(int j=0;j<l;j++) {DEBUGX(fprintf(stderr," %x",rbuf[j]));}
                  DEBUGX(fprintf(stderr,"\n");)
                  DEBUGX(fprintf(stderr,"ucsx(f8b,f8b): %u, 0x%x, i: %u, b: %u, obuf[i]: 0x%02x,rbuf[i]: 0x%02x\n",
                     ucsx,ucsx,i,b,(unsigned)obuf[i],(unsigned)rbuf[i]);)
                  #undef DEBUGX
               }
               chkucsx_step(ucsx,rbuf[i]!=obuf[i],rbuf,l,bs);
            }
         }
      }
   }

}

//*******************************************************************
void r1_full_notminimal(ucsx_t ucsx)

// Előállítja az ucsx utf8 kódját nem minimális kódolással
// és megnézi, hogy az elemző észreveszi-e.
// full bufferes, azaz a buffer mindig 7 hosszú.

{
   static int bufsize=7;
   utf8fchar_t mbuf[bufsize];
   utf8fchar_t obuf[bufsize];
   utf8f_size_t l,minl;
   int seqlen;

   memset(obuf,'\0',bufsize);

   if (0==(minl=ucsx2utf8f(ucsx,mbuf,sizeof(mbuf)))) perror("error: Buffer overflow");
   for(seqlen=minl+1;seqlen<=7;seqlen++)
   {
      if (0==(l=ucsx2utf8f_len(ucsx,seqlen,obuf,sizeof(obuf)))) perror("error: Buffer overflow");
      if (l!=minl)
      {
         DEBUG(fprintf(stderr,"utf8f(full,notminimal): ucsx: %d, l: %ld, minl: %ld",ucsx,l,minl);)
         DEBUG(for(int j=0;j<l;j++) fprintf(stderr," %x",obuf[j]);)
         DEBUG(fprintf(stderr,"\n");)

         chkucsx_full(ucsx,1,obuf,l,bufsize);
      }
   }
}

//*******************************************************************
void r1_len_notminimal(ucsx_t ucsx)

// Előállítja az ucsx utf8 kódját nem minimális kódolással
// és megnézi, hogy az elemző észreveszi-e.

// len bufferes, azaz a buffer hossza mindig azonos az utf8 szekvencia
// hosszával.

{
   static int bufsize=7;
   utf8fchar_t mbuf[bufsize];
   utf8fchar_t obuf[bufsize];
   utf8f_size_t l,minl;
   int seqlen;

   memset(obuf,'\0',bufsize);

   if (0==(minl=ucsx2utf8f(ucsx,mbuf,sizeof(mbuf)))) perror("error: Buffer overflow");
   for(seqlen=minl+1;seqlen<=7;seqlen++)
   {
      if (0==(l=ucsx2utf8f_len(ucsx,seqlen,obuf,sizeof(obuf)))) perror("error: Buffer overflow");
      if (l!=minl)
      {
         for(int bs=1;bs<=7;bs++)
         {
            DEBUG(fprintf(stderr,"utf8f(len,notminimal): ucsx: %d, l: %ld, minl: %ld",ucsx,l,minl);)
            DEBUG(for(int j=0;j<l;j++) fprintf(stderr," %x",obuf[j]);)
            DEBUG(fprintf(stderr,"\n");)

            chkucsx_step(ucsx,1,obuf,l,bs);
         }
      }
   }
}


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

//*******************************************************************
static int ucsx2utf8f_len(uint32_t ucsx,utf8f_size_t seqlen, utf8fchar_t *buf, utf8f_size_t buflen)
// Megadott hosszra kódolja az utf8 szekvenciát.
{
   // printf("ucsx: %u, UTF8L_1: %u\n",ucsx,UTF8L_1);
   if (buflen<seqlen) return 0;

   switch(seqlen)
   {
   case 1:
      buf[0]=(utf8fchar_t)ucsx;
      return 1;
   case 2:
      buf[0]=(utf8fchar_t)((ucsx>>6)|UTF8_HEADSIGN_2);
      buf[1]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 2;
   case 3:
      if (buflen<3) return 0;
      buf[0]=(utf8fchar_t)((ucsx>>12)|UTF8_HEADSIGN_3);
      buf[1]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 3;
   case 4:
      buf[0]=(utf8fchar_t)((ucsx>>18)|UTF8_HEADSIGN_4);
      buf[1]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 4;
   case 5:
      buf[0]=(utf8fchar_t)((ucsx>>24)|UTF8_HEADSIGN_5);
      buf[1]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 5;
   case 6:
      buf[0]=(utf8fchar_t)((ucsx>>30)|UTF8_HEADSIGN_6);
      buf[1]=(utf8fchar_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[5]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 6;
   case 7:
      buf[0]=UTF8_HEADSIGN_7;
      buf[1]=(utf8fchar_t)(((ucsx>>30)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[2]=(utf8fchar_t)(((ucsx>>24)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[3]=(utf8fchar_t)(((ucsx>>18)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[4]=(utf8fchar_t)(((ucsx>>12)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[5]=(utf8fchar_t)(((ucsx>>6)&UTF8_CHARMASK)|UTF8_CHARSIGN);
      buf[6]=(utf8fchar_t)((ucsx&UTF8_CHARMASK)|UTF8_CHARSIGN);
      return 7;
   }
   return 0;
}

//*******************************************************************
void r3_full(ucsx_t ucsx)
{
   r1_full(ucsx-1);
   r1_full(ucsx);
   r1_full(ucsx+1);
}

//*******************************************************************
void r3_len(ucsx_t ucsx)
{
   r1_len(ucsx-1);
   r1_len(ucsx);
   r1_len(ucsx+1);
}

//*******************************************************************
void f1_full()
{
   r1_full(UTF8L_1);
   r1_full(UTF8L_2);
   r1_full(UTF8L_3);
   r1_full(UTF8L_4);
   r1_full(UTF8L_5);
   r1_full(UTF8L_6);
   r1_full(UTF8L_7);
}

//*******************************************************************
void f3_full()
{
   r3_full(UTF8L_1);
   r3_full(UTF8L_2);
   r3_full(UTF8L_3);
   r3_full(UTF8L_4);
   r3_full(UTF8L_5);
   r3_full(UTF8L_6);
   r3_full(UTF8L_7);
}

//*******************************************************************
void f1_full_notminimal()
{
   r1_full_notminimal(UTF8L_1);
   r1_full_notminimal(UTF8L_2);
   r1_full_notminimal(UTF8L_3);
   r1_full_notminimal(UTF8L_4);
   r1_full_notminimal(UTF8L_5);
   r1_full_notminimal(UTF8L_6);
   r1_full_notminimal(UTF8L_7);
}

//*******************************************************************
void f1_len_notminimal()
{
   r1_len_notminimal(UTF8L_1);
   r1_len_notminimal(UTF8L_2);
   r1_len_notminimal(UTF8L_3);
   r1_len_notminimal(UTF8L_4);
   r1_len_notminimal(UTF8L_5);
   r1_len_notminimal(UTF8L_6);
   r1_len_notminimal(UTF8L_7);
}

//*******************************************************************
void f1_len()
{
   r1_len(UTF8L_1);
   r1_len(UTF8L_2);
   r1_len(UTF8L_3);
   r1_len(UTF8L_4);
   r1_len(UTF8L_5);
   r1_len(UTF8L_6);
   r1_len(UTF8L_7);
}

//*******************************************************************
void f3_len()
{
   r3_len(UTF8L_1);
   r3_len(UTF8L_2);
   r3_len(UTF8L_3);
   r3_len(UTF8L_4);
   r3_len(UTF8L_5);
   r3_len(UTF8L_6);
   r3_len(UTF8L_7);
}

//*******************************************************************
int main()
{
   fprintf(stderr,"Teszt begin.\n");
   // r1_len(UTF8L_7);return 0;

   f1_full_notminimal();
   f1_len_notminimal();
   f3_full();
   f3_len();
   fprintf(stderr,"Teszt end.\n");
   return 0;
}

//*******************************************************************
