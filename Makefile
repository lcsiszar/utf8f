
distdir:=$(shell if [ -f ../.sourcedir ]; then echo -n "../";cat ../.sourcedir; fi)
export distdir

lib_utf8f=lib/libutf8f.a
exes=
scripts=
libs=$(lib_utf8f)

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


sources:=utf8f

objects := $(addsuffix .o, $(sources))

# EXEC:=$(shell echo "sources: " $(sources) "Objects: " $(objects) 1>&2)

all: $(exes) $(libs)

$(lib_utf8f): $(objects)
	$(AR) q $@ $(objects)


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
utf8f.o: utf8f.c utf8f.h
# End depends by mdep2make
