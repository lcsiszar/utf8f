//*******************************************************************
// utf8f.h: utf8f header
//*******************************************************************

//*******************************************************************
#ifndef _UTF8F_H_
#define _UTF8F_H_

//*******************************************************************
typedef int8_t   utf8f_int8_t;
typedef uint8_t  utf8f_uint8_t;
typedef int16_t  utf8f_int16_t;
typedef uint16_t utf8f_uint16_t;
typedef int32_t  utf8f_int32_t;
typedef uint32_t utf8f_uint32_t;
typedef size_t   utf8f_size_t;
//typedef ssize_t  utf8f_ssize_t; // Az unistd.h-ban van.
// typedef bool     utf8f_bool;
         
//*******************************************************************
typedef unsigned char utf8char_t;
typedef uint32_t ucsx_t;

//*******************************************************************
/*
 Egy mutató struktúra, amit az utf8f puffer elemzéséhez használ.
 Akkor is ezt használja, amikor részletekben jön az input.
 buf: A puffer első bájtja
 ibuf: A puffer első feldolgozalan bájtja.
 ebuf: A puffer utolsó utáni bájtja.
 Hossz: ebuf-buf
 A hátralévő karakterek száma: ebuf-ibuf
*/
typedef struct _utf8fp
{
   int state;
   utf8char_t *buf;
   utf8char_t *ibuf; // A kövekező, még feldolgozatlan karakterre mutat.
   utf8char_t *ebuf; // A buffer utolsó karaktere után mutat.
   ucsx_t ucsx; // Itt (is) adja az eredményt, illetve itt tárolja a feldolgozás közbeni részeredményt.
   int utf8flen; // Az elemzés alatt álló utf8f karakter hossza. Csak akkor van helyesen kitöltve, ha a status UTF8FS_Sx.
} utf8fp;


//*******************************************************************
#define UTF8F_CHECK  ((ucsx_t)-1) // Lehetne 0 is.

#define UTF8FS_VALID   0 // Fix, nem lehet változtatni.
#define UTF8FS_INVALID 1 // Fix, nem lehet változtatni.

extern int ucsx2utf8f(uint32_t ucsx,utf8char_t *buf, utf8f_size_t buflen);

//*******************************************************************
extern void utf8fp_setup(utf8fp *up,utf8char_t *buf,utf8f_size_t l);
extern void utf8fp_cont(utf8fp *up,utf8char_t *buf,utf8f_size_t l);
extern void utf8fp_cont_l(utf8fp *up,utf8f_size_t l);
extern ucsx_t utf8fp_nextchar(utf8fp *up); // Get next uft8f char.
extern void utf8fp_nextbyte(utf8fp *up); // Jump an invalid byte.

//*******************************************************************
#endif // _UTF8F_H_
//*******************************************************************

