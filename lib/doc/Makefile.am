include $(top_srcdir)/config/common.am

DISTCLEANFILES += $(tocfiles)

EXTRA_DIST = depend.py doc_toc.py README.Documentation $(DEPENDFILE) $(docfiles)

clipartdocdir = $(pkgdatadir)/doc/clipart
dist_clipartdoc_DATA = \
	clipart/escher-lsd.eps \
	clipart/mobius.eps \
	clipart/platypus.eps \
	clipart/BoxInsetDefaultQt4.png \
	clipart/ChildDocumentQt4.png \
	clipart/CommentNoteImageQt4.png \
	clipart/endnotes.pdf \
	clipart/ExternalMaterialQt4.png \
	clipart/floatQt4.png \
	clipart/footnoteQt4.png \
	clipart/FramedNoteImageQt4.png \
	clipart/GreyedOutNoteImageQt4.png \
	clipart/labelQt4.png \
	clipart/LyXNoteImageQt4.png \
	clipart/referenceQt4.png \
	clipart/ShadedNoteImageQt4.png \
	clipart/without_fntright.pdf \
	clipart/with_fntright.pdf

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

declipartdocdir = $(pkgdatadir)/doc/de/clipart
dist_declipartdoc_DATA = \
	de/ExternesMaterialQt4.png \
	de/FussnoteQt4.png \
	de/GerahmteNotizQt4.png \
	de/GleitobjektQt4.png \
	de/GrauschriftNotizQt4.png \
	de/KommentarQt4.png \
	de/LyXNotizQt4.png \
	de/Marke.png \
	de/Querverweis.png \
	de/SchattierteNotizQt4.png \
	de/StandardBoxQt4.png \
	de/UnterdokumentQt4.png

esdocdir = $(pkgdatadir)/doc/es
dist_esdoc_DATA = \
        es/DocumentoPostizo1.lyx \
        es/DocumentoPostizo2.lyx \
	es/DocumentoTextoPostizo.txt \
	es/EmbeddedObjects.lyx \
	es/Extended.lyx \
	es/Intro.lyx \
	es/Tutorial.lyx

esclipartdocdir = $(pkgdatadir)/doc/es/clipart
dist_esclipartdoc_DATA = \
	es/clipart/ComentNotaImagenQt4.png \
	es/clipart/CuadroMinipagQt4.png \
	es/clipart/DocumentoHijoQt4.png \
	es/clipart/etiquetaQt4.png \
	es/clipart/flotanteQt4.png \
	es/clipart/GrisNotaImagenQt4.png \
	es/clipart/MaterialExternoQt4.png \
	es/clipart/NotaEnmarcadaImg.png \
	es/clipart/NotaLyXImagenQt4.png \
	es/clipart/notapieQt4.png \
	es/clipart/NotaSombreadaImg.png \
	es/clipart/referenciaQt4.png

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
	DummyTextDocument.txt \
	DummyDocument1.lyx \
	DummyDocument2.lyx \
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
