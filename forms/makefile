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

SRCS = bibforms.fd credits_form.fd form1.fd include_form.fd \
	log_form.fd layout_forms.fd lyx.fd math_forms.fd \
	print_form.fd sp_form.fd

OBJS = bibforms.c credits_form.c form1.c include_form.c \
	log_form.c layout_forms.c lyx.c math_forms.c \
	print_form.c sp_form.c

COBJS = bibforms.C credits_form.C form1.C include_form.C \
	log_form.C layout_forms.C lyx.C math_forms.C \
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
# -e 's,\(generated with fdesign\)\(.*\)\(\*\*/\),\1 \3,';\
#
# Clean up the line with the date in it.  This means we'll have fewer false
# updates (where the only thing that changed was the date).
#
e: c
	for hfil in *.h ; do \
		if [ -f "$$hfil.patch" ] ; then \
			(echo "Patching $$hfil with $$hfil.patch" ; \
			patch $$hfil < $$hfil.patch) \
		fi; \
		sed < $$hfil > $$hfil.tmp \
			-e 's/\(extern\) \(void.*FL_OBJECT\)/\1 "C" \2/' \
			-e 's,\(generated with fdesign\)\(.*\)\(\*\*/\),\1 \3,';\
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
			cmp -s $$fil ../src/mathed/$$fil || cp $$fil ../src/mathed ; \
		else \
			cmp -s $$fil ../src/$$fil || cp $$fil ../src ; \
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
