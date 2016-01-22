
distdir:=$(shell if [ -f ../.sourcedir ]; then echo -n "../";cat ../.sourcedir; fi)
export distdir

exe_tutf8f=bin/tutf8f

exes=$(exe_tutf8f)
scripts=

prefix=tmpdir
install_dir=$(prefix)/usr/bin

installprog_file=install -s -m 755
installprog_dir=install -d

#installprog_file=cp -af
#installprog_dir=mkdir -p

                        
#CCDEBUG=-g
CCWARNING=-Wall
CCOPTFLAGS=-O6

CCPLATFORMFLAGS=#-malign-jumps=4 -malign-functions=4 -mpentium

CCEXTRA=
CCINCLUDE=
CCDEFINE=-D_UNIX_
CCLIB=

CCFLAGS=$(CCDEBUG) $(CCOPTFLAGS) $(CCPLATFORMFLAGS) $(CCEXTRA) \
         $(CCWARNING) $(CCINCLUDE) $(CCDEFINE)

%.o: %.c
	$(CC) -o $@ -c $< $(CCFLAGS)


sources:=utf8f tutf8f

objects := $(addsuffix .o, $(sources))

# EXEC:=$(shell echo "sources: " $(sources) "Objects: " $(objects) 1>&2)

all: $(exes)

$(exe_tutf8f): $(objects)
	$(CC) -o $@ $(objects) $(CCLIB)


tmpclean:
	find . -name '*~' -o -name '*.bak' | xargs rm -f 
	rm -rf test

clean: tmpclean
	rm -f *.o *.log
	rm -rf tmpdir

realclean: clean
	rm -f $(exes)

distclean: realclean
	rm -f .crstate_*

install: $(exes) $(scripts)
	install -d $(install_dir)
	install $(exes) $(scripts) $(install_dir)

dist-fdiff:
	dist/ddsync-dist --fdiff .

dist-diff:
	dist/ddsync-dist --diff .

dist-save:
	dist/ddsync-dist --save .

dist-update:
	dist/ddsync-dist --update .

# Start depends by mdep2make
tutf8f.o: tutf8f.c utf8f.h
utf8f.o: utf8f.c utf8f.h
# End depends by mdep2make
