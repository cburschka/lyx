include $(top_srcdir)/config/common.am

DISTCLEANFILES += $(tocfiles)

EXTRA_DIST = depend.py doc_toc.py README.Documentation $(DEPENDFILE) $(docfiles)

clipartdocdir = $(pkgdatadir)/doc/clipart
dist_dadoc_DATA = \
	escher-lsd.eps \
	mobius.eps \
	platypus.eps \
	BoxInsetDefaultQt4.png \
	CommentNoteImageQt4.png \
	endnotes.pdf \
	floatQt4.png \
	footnoteQt4.png \
	FramedNoteImageQt4.png \
	GreyedOutNoteImageQt4.png \
	labelQt4.png \
	LyXNoteImageQt4.png \
	referenceQt4.png \
	ShadedNoteImageQt4.png \
	without_fntright.pdf \
	with_fntright.pdf

csdocdir = $(pkgdatadir)/doc/cs
dist_csdoc_DATA = \
	cs/Tutorial.lyx

dadocdir = $(pkgdatadir)/doc/da
dist_dadoc_DATA = \
	da/Intro.lyx

dedocdir = $(pkgdatadir)/doc/de
dist_dedoc_DATA = \
	de/Customization.lyx \
	de/Extended.lyx \
	de/FAQ.lyx \
	de/Intro.lyx \
	de/Tutorial.lyx \
	de/UserGuide.lyx

esdocdir = $(pkgdatadir)/doc/es
dist_esdoc_DATA = \
	es/Intro.lyx \
	es/Tutorial.lyx \
	es/EmbeddedObjects.lyx

esdocdir = $(pkgdatadir)/doc/es/clipart
dist_esdoc_DATA = \
	ComentNotaImagenQt4.png \
	CuadroMinipagQt4.png \
	etiquetaQt4.png \
	flotanteQt4.png \
	GrisNotaImagenQt4.png \
	NotaEnmarcadaImg.png \
	NotaLyXImagenQt4.png \
	notapieQt4.png \
	NotaSombreadaImg.png \
	referenciaQt4.png

eudocdir = $(pkgdatadir)/doc/eu
dist_eudoc_DATA = \
	eu/Customization.lyx \
	eu/Extended.lyx \
	eu/FAQ.lyx \
	eu/Intro.lyx \
	eu/Tutorial.lyx \
	eu/UserGuide.lyx

frdocdir = $(pkgdatadir)/doc/fr
dist_frdoc_DATA = \
	fr/Customization.lyx \
	fr/Extended.lyx \
	fr/FAQ.lyx \
	fr/Intro.lyx \
	fr/Tutorial.lyx \
	fr/UserGuide.lyx

gldocdir = $(pkgdatadir)/doc/gl
dist_gldoc_DATA = \
	gl/Intro.lyx \
	gl/Tutorial.lyx

hedocdir = $(pkgdatadir)/doc/he
dist_hedoc_DATA = \
	he/Intro.lyx \
	he/Tutorial.lyx

hudocdir = $(pkgdatadir)/doc/hu
dist_hudoc_DATA = \
	hu/Intro.lyx \
	hu/Tutorial.lyx

itdocdir = $(pkgdatadir)/doc/it
dist_itdoc_DATA = \
	it/Customization.lyx \
	it/Intro.lyx \
	it/Tutorial.lyx \
	it/UserGuide.lyx

nldocdir = $(pkgdatadir)/doc/nl
dist_nldoc_DATA = \
	nl/Intro.lyx \
	nl/Tutorial.lyx

nbdocdir = $(pkgdatadir)/doc/nb
dist_nbdoc_DATA = \
	nb/Intro.lyx

pldocdir = $(pkgdatadir)/doc/pl
dist_pldoc_DATA = \
	pl/Extended.lyx \
	pl/Intro.lyx \
	pl/Tutorial.lyx

ptdocdir = $(pkgdatadir)/doc/pt
dist_ptdoc_DATA = \
	pt/Intro.lyx \
	pt/Tutorial.lyx

rodocdir = $(pkgdatadir)/doc/ro
dist_rodoc_DATA = \
	ro/Intro.lyx

rudocdir = $(pkgdatadir)/doc/ru
dist_rudoc_DATA = \
	ru/FAQ.lyx \
	ru/Intro.lyx \
	ru/Tutorial.lyx

skdocdir = $(pkgdatadir)/doc/sk
dist_skdoc_DATA = \
	sk/Tutorial.lyx \
	sk/UserGuide.lyx

sldocdir = $(pkgdatadir)/doc/sl
dist_sldoc_DATA = \
	sl/Intro.lyx \
	sl/Tutorial.lyx

svdocdir = $(pkgdatadir)/doc/sv
dist_svdoc_DATA = \
	sv/Intro.lyx \
	sv/Tutorial.lyx

docdir = $(pkgdatadir)/doc
dist_doc_DATA = \
	Customization.lyx \
	DocStyle.lyx \
	EmbeddedObjects.lyx \
	Extended.lyx \
	FAQ.lyx \
	Intro.lyx \
	LaTeXConfig.lyx.in \
	Reference.lyx \
	Tutorial.lyx \
	UserGuide.lyx

doc_DATA = $(docfiles) 

DEPENDFILE = $(srcdir)/Makefile.depend
# include $(DEPENDFILE) does not work because automake is too limited.
include $(srcdir)/Makefile.depend

TOCs : $(DEPENDFILE) $(tocfiles)
	@echo Made TOCs succesfully.

# Force regeneration of $(DEPENDFILE) when Makefile.am changes because
# new doc files might have been added
$(DEPENDFILE): $(srcdir)/Makefile.am $(srcdir)/depend.py
	python $(srcdir)/depend.py > $(DEPENDFILE)

# The TOCs are not built for the install and dist targets if they don't exist
# for some weird reason.
# Make complains although we have rules for them in $(DEPENDFILE), so we
# must not include the TOCs in $(docfiles) and have to use the install and
# dist hooks below.
dist-hook: $(tocfiles)
	for i in $(tocfiles); \
	do \
		if test -f "$(srcdir)/$$i"; then file="$(srcdir)/$$i"; \
		else file="$$i"; fi; \
		cp -p "$$file" "$(distdir)"; \
	done
install-data-hook: $(tocfiles)
	for i in $(tocfiles); \
	do \
		if test -f "$(srcdir)/$$i"; then file="$(srcdir)/$$i"; \
		else file="$$i"; fi; \
		$(docDATA_INSTALL) "$$file" "$(DESTDIR)$(docdir)/$$i"; \
	done

uninstall-local:
	for i in $(tocfiles); \
	do \
		$(RM) "$(DESTDIR)$(docdir)/$$i" ; \
	done

.PHONY: TOCs
