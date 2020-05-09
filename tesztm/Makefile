
distdir:=$(shell if [ -f ../.sourcedir ]; then echo -n "../";cat ../.sourcedir; fi)
export distdir

exe_utf8fmixedlatin2full2latin2full=bin/utf8fmixedlatin2full2latin2full
exe_utf8fmixedlatin2full2utf8f=bin/utf8fmixedlatin2full2utf8f

exes=$(exe_utf8fmixedlatin2full2latin2full) \
     $(exe_utf8fmixedlatin2full2utf8f) 

scripts=

prefix=tmpdir
install_dir=$(prefix)/usr/bin

installprog_file=install -s -m 755
installprog_dir=install -d

#installprog_file=cp -af
#installprog_dir=mkdir -p

                        
CCDEBUG=-g
CCWARNING=-Wall
CCOPTFLAGS=-O6

CCPLATFORMFLAGS=#-malign-jumps=4 -malign-functions=4 -mpentium

CCEXTRA=
CCINCLUDE=-I..
CCDEFINE=-D_UNIX_
CCLIB=

CCFLAGS=$(CCDEBUG) $(CCOPTFLAGS) $(CCPLATFORMFLAGS) $(CCEXTRA) \
         $(CCWARNING) $(CCINCLUDE) $(CCDEFINE)

%.o: %.c
	$(CC) -o $@ -c $< $(CCFLAGS)


sources:=../utf8f

objects := $(addsuffix .o, $(sources))

# EXEC:=$(shell echo "sources: " $(sources) "Objects: " $(objects) 1>&2)

all: $(exes)

$(exe_utf8fmixedlatin2full2latin2full): utf8fmixedlatin2full2latin2full.o $(objects)
	echo "Objects: " $(objects) $^
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

$(exe_utf8fmixedlatin2full2utf8f): utf8fmixedlatin2full2utf8f.o $(objects)
	echo "Objects: " $(objects) $^
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

tmpclean:
	find . -name '*~' -o -name '*.bak' | xargs rm -f 

clean: tmpclean
	rm -f *.o *.log bin/* lib/*
	rm -rf tmpdir

realclean: clean
	rm -f $(exes)

distclean: realclean
	rm -f .crstate_*

install: $(exes) $(scripts)
	install -d $(install_dir)
	install $(exes) $(scripts) $(install_dir)

# Start depends by mdep2make
utf8fmixedlatin2full2latin2full.o: utf8fmixedlatin2full2latin2full.c \
 ../utf8f.h unicode_to_latin2full.h
utf8fmixedlatin2full2utf8f.o: utf8fmixedlatin2full2utf8f.c ../utf8f.h \
 latin2full_to_unicode.h
# End depends by mdep2make
