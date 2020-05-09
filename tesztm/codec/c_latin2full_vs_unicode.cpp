//*******************************************************************
// c_latin2full_vs_ucsx.cpp: Create latin2full<->ucsx C tables.
//*******************************************************************

//*******************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <error.h>

typedef uint32_t ucsx_t;

//*******************************************************************
#define MAX_UCSX_TABLE   0xffff
#define MAX_BYTE_TABLE 256

//*******************************************************************
typedef struct _uchar_unicode_pair {
   unsigned char c;
   uint32_t unicode;
} uchar_unicode_pair;

//*******************************************************************
uchar_unicode_pair latin2_full_unicode_pairs[]={
// A teljes latin2-unicode megfeleles (csak a kulonbozo koduak).
   {0xa1, 0x104}, // 0xa1(161), 0x104(260)
   {0xa2, 0x2d8}, // 0xa2(162), 0x2d8(728)
   {0xa3, 0x141}, // 0xa3(163), 0x141(321)
   {0xa5, 0x13d}, // 0xa5(165), 0x13d(317)
   {0xa6, 0x15a}, // 0xa6(166), 0x15a(346)
   {0xa9, 0x160}, // 0xa9(169), 0x160(352)
   {0xaa, 0x15e}, // 0xaa(170), 0x15e(350)
   {0xab, 0x164}, // 0xab(171), 0x164(356)
   {0xac, 0x179}, // 0xac(172), 0x179(377)
   {0xae, 0x17d}, // 0xae(174), 0x17d(381)
   {0xaf, 0x17b}, // 0xaf(175), 0x17b(379)
   {0xb1, 0x105}, // 0xb1(177), 0x105(261)
   {0xb2, 0x2db}, // 0xb2(178), 0x2db(731)
   {0xb3, 0x142}, // 0xb3(179), 0x142(322)
   {0xb5, 0x13e}, // 0xb5(181), 0x13e(318)
   {0xb6, 0x15b}, // 0xb6(182), 0x15b(347)
   {0xb7, 0x2c7}, // 0xb7(183), 0x2c7(711)
   {0xb9, 0x161}, // 0xb9(185), 0x161(353)
   {0xba, 0x15f}, // 0xba(186), 0x15f(351)
   {0xbb, 0x165}, // 0xbb(187), 0x165(357)
   {0xbc, 0x17a}, // 0xbc(188), 0x17a(378)
   {0xbd, 0x2dd}, // 0xbd(189), 0x2dd(733)
   {0xbe, 0x17e}, // 0xbe(190), 0x17e(382)
   {0xbf, 0x17c}, // 0xbf(191), 0x17c(380)
   {0xc0, 0x154}, // 0xc0(192), 0x154(340)
   {0xc3, 0x102}, // 0xc3(195), 0x102(258)
   {0xc5, 0x139}, // 0xc5(197), 0x139(313)
   {0xc6, 0x106}, // 0xc6(198), 0x106(262)
   {0xc8, 0x10c}, // 0xc8(200), 0x10c(268)
   {0xca, 0x118}, // 0xca(202), 0x118(280)
   {0xcc, 0x11a}, // 0xcc(204), 0x11a(282)
   {0xcf, 0x10e}, // 0xcf(207), 0x10e(270)
   {0xd0, 0x110}, // 0xd0(208), 0x110(272)
   {0xd1, 0x143}, // 0xd1(209), 0x143(323)
   {0xd2, 0x147}, // 0xd2(210), 0x147(327)
   {0xd5, 0x150}, // 0xd5(213), 0x150(336)
   {0xd8, 0x158}, // 0xd8(216), 0x158(344)
   {0xd9, 0x16e}, // 0xd9(217), 0x16e(366)
   {0xdb, 0x170}, // 0xdb(219), 0x170(368)
   {0xde, 0x162}, // 0xde(222), 0x162(354)
   {0xe0, 0x155}, // 0xe0(224), 0x155(341)
   {0xe3, 0x103}, // 0xe3(227), 0x103(259)
   {0xe5, 0x13a}, // 0xe5(229), 0x13a(314)
   {0xe6, 0x107}, // 0xe6(230), 0x107(263)
   {0xe8, 0x10d}, // 0xe8(232), 0x10d(269)
   {0xea, 0x119}, // 0xea(234), 0x119(281)
   {0xec, 0x11b}, // 0xec(236), 0x11b(283)
   {0xef, 0x10f}, // 0xef(239), 0x10f(271)
   {0xf0, 0x111}, // 0xf0(240), 0x111(273)
   {0xf1, 0x144}, // 0xf1(241), 0x144(324)
   {0xf2, 0x148}, // 0xf2(242), 0x148(328)
   {0xf5, 0x151}, // 0xf5(245), 0x151(337)
   {0xf8, 0x159}, // 0xf8(248), 0x159(345)
   {0xf9, 0x16f}, // 0xf9(249), 0x16f(367)
   {0xfb, 0x171}, // 0xfb(251), 0x171(369)
   {0xfe, 0x163}, // 0xfe(254), 0x163(355)
   {0xff, 0x2d9}, // 0xff(255), 0x2d9(729)

   {0,0}

};

//*******************************************************************
void init_table(ucsx_t *uchar_to_ucsx_table)
{
   int i;

   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      uchar_to_ucsx_table[i]=i;
   }
   
}

//*******************************************************************
void c_pairs_to_table(uchar_unicode_pair *p, ucsx_t *uchar_to_ucsx_table)
{
   int i;

   for(i=0;p[i].c;i++)
   {
      if (uchar_to_ucsx_table[p[i].c]!=p[i].c)
      {
         error(1,0,"Nem egyertelmu kodolas: i: %d, c: %u(0x%x), unicode: %u(0x%x)",
               i,p[i].c,p[i].c,p[i].unicode,p[i].unicode);
      }
      uchar_to_ucsx_table[p[i].c]=p[i].unicode;
   }
}

//*******************************************************************
void p_ucsx_table(ucsx_t *ucsx_table, const char *name)
{
   int i;

   FILE *f;

   char fname[1024];

   if (sizeof(fname)<=snprintf(fname,sizeof(fname),"%s.txt",name)) error(1,0,"p_ucsx_table: fname out of buf");
   fname[sizeof(fname)-1]='\0';

   if (NULL==(f=fopen(fname,"w")))
   {
      error(1,errno,"%s: create error",fname);
   }
   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      fprintf(f,"%.3d(0x%.3x): %.4d(0x%.4x)\n",i,i,ucsx_table[i],ucsx_table[i]);
   }
   fclose(f);
}

//*******************************************************************
void w_prog_to_ucsx(const char *name, ucsx_t *ucsx_table)
{
   int i;

   FILE *f;

   char fname[1024];

   p_ucsx_table(ucsx_table,name);

   if (sizeof(fname)<=snprintf(fname,sizeof(fname),"%s.h",name)) error(1,0,"w_prog: fname out of buf");
   fname[sizeof(fname)-1]='\0';

   if (NULL==(f=fopen(fname,"w")))
   {
      error(1,errno,"%s: open error",fname);
   }

   fprintf(f,"\nuint32_t _table_%s[%d]={\n",name,MAX_BYTE_TABLE);

   int sorhossz=8;
   int isor=0;
   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      if (isor==0) fprintf(f,"   ");
      fprintf(f,"0x%.4x",ucsx_table[i]);
      fprintf(f,"%c",i==MAX_BYTE_TABLE-1?' ':',');
      if (++isor==sorhossz)
      {
         fprintf(f," // %d(0x%x)-%d(0x%x)\n",i-sorhossz+1,i-sorhossz+1,i,i);
         isor=0;
      }
   }
   if (isor>0)
   {
      fprintf(f," // %d(0x%x)-%d(0x%x)\n",i-isor,i-isor,i-1,i-1);
   }
   fprintf(f,"};\n");
   fprintf(f,"#define %s(c) (_table_%s[(unsigned)(c)])\n",
              name,
              name);
   fclose(f);
}

//*******************************************************************
void w_prog_from_ucsx(const char *name, ucsx_t *ucsx_table)
{
   int i;

   FILE *f;

   char fname[1024];

   p_ucsx_table(ucsx_table,name);

   if (sizeof(fname)<=snprintf(fname,sizeof(fname),"%s.h",name)) error(1,0,"w_prog: fname out of buf");
   fname[sizeof(fname)-1]='\0';

   ucsx_t max_ucsx=0;
   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      if (ucsx_table[i]>max_ucsx) max_ucsx=ucsx_table[i];
   }

   if (max_ucsx>MAX_UCSX_TABLE) error(1,0,"Out of ucsx table: %d,%d",max_ucsx,MAX_UCSX_TABLE);

   unsigned int ucsx2byte[max_ucsx+1];

   for(i=0;i<max_ucsx+1;i++)
   {
      ucsx2byte[i]=0;
   }

   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      // ucsx2byte[i]=i;
   }

   for(i=0;i<MAX_BYTE_TABLE;i++)
   {
      // fprintf(stderr,"i: %d(0x%x), ucsx: %d(0x%x), byte: %d(%x)\n",i,i,ucsx_table[i],ucsx_table[i],ucsx2byte[ucsx_table[i]],ucsx2byte[ucsx_table[i]]);
      if (ucsx2byte[ucsx_table[i]])
      {
         if (i==ucsx_table[i])
         {
            // Nem kell csinalni semmit
         }
         else
         {
            error(1,0,"Multiple ucsx->bit8 definition: i: %d(0x%x), ucsx: %d(0x%x), c: %d(0x%x)",
                      i,i,ucsx_table[i],ucsx_table[i],ucsx2byte[ucsx_table[i]],ucsx2byte[ucsx_table[i]]);
         }
      }
      else
      {
         ucsx2byte[ucsx_table[i]]=i;
      }
   }

   if (NULL==(f=fopen(fname,"w")))
   {
      error(1,errno,"%s: open error",fname);
   }

   fprintf(f,"\nunsigned char _table_%s[%d]={\n",name,max_ucsx+1);

   int sorhossz=8;
   int isor=0;
   for(i=0;i<max_ucsx+1;i++)
   {
      if (isor==0) fprintf(f,"   ");
      fprintf(f,"0x%.2x",ucsx2byte[i]);
      fprintf(f,"%c",i==max_ucsx+1-1?' ':',');
      if (++isor==sorhossz)
      {
         fprintf(f," // %d(0x%x)-%d(0x%x)\n",i-sorhossz+1,i-sorhossz+1,i,i);
         isor=0;
      }
   }
   if (isor>0)
   {
      fprintf(f," // %d(0x%x)-%d(0x%x)\n",i-isor,i-isor,i-1,i-1);
   }
   fprintf(f,"};\n");
   fprintf(f,"#define %s(ucsx,def) (((unsigned)(ucsx))>=sizeof(_table_%s)?def:(_table_%s[(unsigned)(ucsx)])==0?def:(_table_%s[(unsigned)(ucsx)]))\n",
              name,
              name,name,name);
   fclose(f);
}

//*******************************************************************
void w_prog_latin2full_to_unicode()
{
   // latin2full->unicode.

   ucsx_t table[MAX_BYTE_TABLE];

   init_table(table);
   c_pairs_to_table(latin2_full_unicode_pairs,table);

   w_prog_to_ucsx("latin2full_to_unicode",table);
}

//*******************************************************************
void w_prog_latin2full_from_unicode()
{
   // latin2full<-unicode.

   ucsx_t table[MAX_BYTE_TABLE];

   init_table(table);
   c_pairs_to_table(latin2_full_unicode_pairs,table);

   w_prog_from_ucsx("unicode_to_latin2full",table);
}

//*******************************************************************
int main()
{
   w_prog_latin2full_to_unicode();
   w_prog_latin2full_from_unicode();
}
//*******************************************************************
