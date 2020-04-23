//*******************************************************************
// utf8f.h: utf8f header
//*******************************************************************

//*******************************************************************
#ifndef _UTF8F_H_
#define _UTF8F_H_
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#define UTF8F_VERSION "v0.0.10"
extern const char *utf8fp_version;

//*******************************************************************
extern const char *utf8fp_version;

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
typedef unsigned char utf8fchar_t;
typedef uint32_t ucsx_t;

//*******************************************************************
#define UTF8FMAXBYTELEN 7

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

#define UTF8FM_UTF8FFIXED    0 // Csak utf8 karakter lehet.  Ha nem utf8
                               // karakter jön, az hiba.

#define UTF8FM_UTF8FMIXCODE8 1 // Ha nem utf8 karakter jön, akkor a bájtot
                               // átkódolja ucsx-re a kódtábla segítségével.

#define UTF8FM_CODE8         2 // Ha nem utf8 karakter jön, akkor az az utáni
                               // bájtokat a stream-ben 8bites karakterként
                               // kezeli és a kód táblával átkonvertálja.
#define UTF8FM_CODE8FIXED    3 // A bájtokat a stream-ben 8bites karakterként
                               // kezeli és a kód táblával átkonvertálja.

// A karakter olvasás módja.
#define UTF8FCMCR_UTF8F          0 // utf8 mód
#define UTF8FCMCR_CODE8          1 // 8bit mód
#define UTF8FCMCR_UTF8FCBUF      2 // cbuf mód, ha a cbuf elfogy UTF8F-re vált.
#define UTF8FCMCR_UTF8FCODE8     3 // cbuf mód, ha a cbuf elfogy CODE8-ra vált.

#define UTF8FCRLFMODE_NOCRLFCONV  0
#define UTF8FCRLFMODE_CRLF2LF     1
#define UTF8FCRLFMODE_LF2CRLF     2

// crlfstate állapotok.
#define UTF8FCS_NONE    0
#define UTF8FCS_UCSX    1
#define UTF8FCS_CR      2
#define UTF8FCS_INVALID 3
#define UTF8FCS_EOB     4


typedef struct _utf8fp
{
   int state;       // UTF8FS_xxx

   int mode;        // UTF8FM_xxx

   int code8;       // Belső változó: A karater olvasás módja.  UTF8FCMCR_xxx

   int crlfmode;    // Hogyan bánjon a sorvégjelekkel: UTF8FCRLFMODE_xxx
   int crlfstate;   // Belső változó: UTF8FCS_xxx

   int crlfucsx;    // Belső változó: UTF8FCS_UCSX állapot esetén adandó
                    // ucsx karakter.

   int cbufpos;
   int cbuflen;

   utf8fchar_t cbuf[8]; // Itt tárolja a még feldolgozatlan karaktereket,
                        // amit a 8btes konverzióhoz használ.

   ucsx_t *codetable;  // Egy 128 ucsx karakter tartalmazó tábla.  A 128 és
                       // az annál nagyobb 8bites karaktereket ezzel kódolja
                       // ucsx-re, amennyiben a mode ezt lehetővé teszi.

   utf8fchar_t *buf;  // A stream buffere.
   utf8fchar_t *ibuf; // A kövekező, még feldolgozatlan karakterre mutat.
   utf8fchar_t *ebuf; // A buf utolsó karaktere után mutat.

   ucsx_t ucsx;      // Itt (is) adja az eredményt, illetve itt tárolja a
                     // feldolgozás közbeni részeredményt.

   int utf8flen;     // Az elemzés alatt álló utf8f karakter hossza.  Csak
                     // akkor van helyesen kitöltve, ha a status
                     // UTF8FS_VALID vagy UTF8FS_VALID.
} utf8fp;


//*******************************************************************
#define UCSX_CHECK  ((ucsx_t)-1) // Lehetne 0 is.

#define UTF8FS_VALID   0 // Fix, nem lehet változtatni.
#define UTF8FS_INVALID 1 // Fix, nem lehet változtatni.
#define UTF8FS_EOB     2 // End Of Buffer. Fix, nem lehet változtatni.

//*******************************************************************
extern int ucsx2utf8f(uint32_t ucsx,utf8fchar_t *buf, utf8f_size_t buflen);
// Átkonvertál egy ucsx kódot utf8f bájtsorozatra.
// Return: Az utf8f bájtsorozat hossza, ha befért a bufferbe, 0, ha nem fért
// be.

//*******************************************************************
extern void utf8fp_setmode(utf8fp *up,int mode,ucsx_t codetable[128]);
// Ha a mode nem létezik, akkor nem állítja be.
// A kódtáblát csak akkor állítja be, ha a mode-hez az szükséges.

//*******************************************************************
extern void utf8fp_setcrlfmode(utf8fp *up,int crlfmode);

//*******************************************************************
extern void utf8fp_setup(utf8fp *up,utf8fchar_t *buf);
// Inicializál egy utf8fp kódolót az olvasott stream elejére.

/* Két eset van:

   Mindig ugyanabba a pufferben adjuk meg a stream következő bájtjait
   ------------------------------------------------------------------

   Ekkor a setup-nak megadjuk a buf-ot, amibe olvasni fogunk és az
   utf8fp_cont_l()-el mindig beállítjuk mennyi bájt van a pufferben.

   A stream-ből mindig különböző pufferbe jönnek a bájtok
   ------------------------------------------------------

   Ekkor itt NULL-t adunk buf-nak és az utf8f_cont()-al mindg megadjuk a
   puffer-t és a hosszát is.

*/

//*******************************************************************
extern void utf8fp_cont(utf8fp *up,utf8fchar_t *buf,utf8f_size_t l);
// Beállít egy új buffert folytatásnak.  Csak akkor szabad hívni, amikor
// az előző pufferből elfogytak a bájtok.

//*******************************************************************
extern void utf8fp_cont_l(utf8fp *up,utf8f_size_t l);
// Ugyanaz, mint az utf9fp_cont(), csak a buf marad a régi.  Csak akkor
// szabad hívni, amikor a pufferből elfogytak a bájtok.

extern ucsx_t utf8fp_nextchar_stream(utf8fp *up); // Get next uft8f char. No crlf handle.
extern ucsx_t utf8fp_nextchar_line(utf8fp *up); // Get next uft8f char. Handle crlf.
extern void utf8fp_nextbyte(utf8fp *up); // Jump an invalid byte.

#ifdef __cplusplus
}
#endif // __cplusplus
//*******************************************************************
#endif // _UTF8F_H_
//*******************************************************************
