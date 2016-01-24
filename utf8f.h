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
   int status;
   utf8char_t *buf;
   utf8char_t *ibuf; // A kövekező, még feldolgozatlan karakterre mutat.
   utf8char_t *ebuf; // A buffer utolsó karaktere után mutat.
   ucsx_t ucsx; // Itt (is) adja az eredményt, illetve itt tárolja a feldolgozás közbeni részeredményt.
} utf8fp;


//*******************************************************************
#define UTF8F_CHECK  ((ucsx_t)-1) // Lehetne 0 is.

#define UTF8FS_VALID   0 // Fix, nem lehet változtatni.

extern int ucsx2utf8f(uint32_t ucsx,utf8char_t *buf, utf8f_size_t buflen);

//*******************************************************************
extern void utf8fp_start(utf8fp *up,utf8char_t *buf,utf8f_size_t l);
extern ucsx_t utf8fnextchar(utf8fp *up);

//*******************************************************************
#endif // _UTF8F_H_
//*******************************************************************

