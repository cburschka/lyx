###/* This file is part of
###* ======================================================
###*
###*           LyX, the High Level Word Processor
###*
###*           Copyright (C) 1995-99 The LyX Team
###*
###*======================================================*/
###
###
### Makefile by Lars Gullik Bjønnes (larsbj@ifi.uio.no)
###  

SHELL = /bin/sh
.SUFFIXES: .fd .c

# Various commands
FDESIGN = fdesign

SRCS = bullet_forms.fd cite.fd credits_form.fd form1.fd \
	latexoptions.fd layout_forms.fd lyx.fd lyx_sendfax.fd math_forms.fd \
	print_form.fd sp_form.fd

OBJS = bullet_forms.c cite.c credits_form.c form1.c \
	latexoptions.c layout_forms.c lyx.c lyx_sendfax.c math_forms.c \
	print_form.c sp_form.c

COBJS = bullet_forms.C cite.C credits_form.C form1.C \
	latexoptions.C layout_forms.C lyx.C lyx_sendfax.C math_forms.C \
	print_form.C sp_form.C

all:
	@echo "This is a makefile intended for the maintainer only."
	@echo "Only run it if you are absolutely sure about what you"
	@echo "are doing."

c: $(OBJS)

d: c $(COBJS)

#
# -e 's/\(extern\) \(void.*FL_OBJECT\)/\1 "C" \2/'
#
# We need the callbacks to all be declared as:  extern "C" void
#
e: c
	for hfil in *.h ; do \
		if [ -f "$$hfil.patch" ] ; then \
			(echo "Patching $$hfil with $$hfil.patch" ; \
			patch -s $$hfil < $$hfil.patch) \
		fi; \
		sed < $$hfil > $$hfil.tmp \
			-e 's/\(extern\) \(void.*FL_OBJECT\)/\1 "C" \2/' ; \
		mv $$hfil.tmp $$hfil; \
	done

updatesrc: d e
	@echo
	@echo "Beware some files may be installed in the wrong directory!" >&2
	@echo "In particular be careful with gui-indep forms.  Make sure">&2
	@echo "they have been removed from the files here otherwise we'll">&2
	@echo "get junk code and errors at compile time.">&2
	for fil in *.C *.h ; do \
		if [ $$fil = "math_forms.C" -o $$fil = "math_forms.h" ]; then \
			cp $$fil ../src/mathed ; \
		else \
			cp $$fil ../src ; \
		fi \
	done

.fd.c:
	$(FDESIGN) -convert $<

.c.C:
	./fdfix.sh $< $@

clean:
	rm -f *.[hcC] *.bak

distclean: clean
	rm -f *.orig *.rej *~
