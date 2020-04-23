
distdir:=$(shell if [ -f ../.sourcedir ]; then echo -n "../";cat ../.sourcedir; fi)
export distdir

exe_ucsx2utf8f=bin/ucsx2utf8f
exe_utf8f2ucsx=bin/utf8f2ucsx
exe_tcr=bin/tcr
exe_tcrutf8=bin/tcrutf8
exe_tinvalid=bin/tinvalid

exes=$(exe_ucsx2utf8f) \
      $(exe_utf8f2ucsx) \
      $(exe_tcr) $(exe_tcrutf8) $(exe_tinvalid)

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

$(exe_ucsx2utf8f): ucsx2utf8f.o $(objects)
	echo "Objects: " $(objects) $^
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

$(exe_utf8f2ucsx): utf8f2ucsx.o $(objects)
	echo "Objects: " $(objects) $^
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

$(exe_tcr): tcr.o $(objects)
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

$(exe_tcrutf8): tcrutf8.o $(objects)
	$(CC) $(CCFLAGS) -o $@ $^ $(CCLIB)

$(exe_tinvalid): tinvalid.o $(objects)
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
utf8f.o: ../utf8f.c ../utf8f.h
tcr.o: tcr.c ../utf8f.h
tcrutf8.o: tcrutf8.c ../utf8f.h
tinvalid.o: tinvalid.c ../utf8f.h
ucsx2utf8f.o: ucsx2utf8f.c ../utf8f.h
utf8f2ucsx.o: utf8f2ucsx.c ../utf8f.h ucsx_latin2_to_unicode.h
# End depends by mdep2make
