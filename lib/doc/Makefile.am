include $(top_srcdir)/config/common.am

DISTCLEANFILES += LyXConfig.lyx LaTeXConfig.lyx Makefile.depend

EXTRA_DIST = depend.py doc_toc.py LyXConfig.lyx.in README.Documentation $(docfiles)

docfiles = \
	cs_TOC.lyx \
	cs_Tutorial.lyx \
	Customization.lyx \
	da_Intro.lyx \
	da_TOC.lyx \
	de_Customization.lyx \
	de_Extended.lyx \
	de_FAQ.lyx \
	de_Intro.lyx \
	de_TOC.lyx \
	de_Tutorial.lyx \
	de_UserGuide.lyx \
	DocStyle.lyx \
	es_Intro.lyx \
	es_TOC.lyx \
	es_Tutorial.lyx \
	eu_Customization.lyx \
	eu_Extended.lyx \
	eu_FAQ.lyx \
	eu_Intro.lyx \
	eu_TOC.lyx \
	eu_Tutorial.lyx \
	eu_UserGuide.lyx \
	Extended.lyx \
	FAQ.lyx \
	fr_Customization.lyx \
	fr_Extended.lyx \
	fr_FAQ.lyx \
	fr_Intro.lyx \
	fr_TOC.lyx \
	fr_Tutorial.lyx \
	fr_UserGuide.lyx \
	he_Intro.lyx \
	he_TOC.lyx \
	he_Tutorial.lyx \
	hu_Intro.lyx \
	hu_TOC.lyx \
	Intro.lyx \
	it_Customization.lyx \
	it_Intro.lyx \
	it_TOC.lyx \
	it_Tutorial.lyx \
	it_UserGuide.lyx \
	LaTeXConfig.lyx.in \
	nl_Intro.lyx \
	nl_TOC.lyx \
	nl_Tutorial.lyx \
	no_Intro.lyx \
	no_TOC.lyx \
	pl_Extended.lyx \
	pl_Intro.lyx \
	pl_TOC.lyx \
	pl_Tutorial.lyx \
	pt_Intro.lyx \
	pt_TOC.lyx \
	pt_Tutorial.lyx \
	Reference.lyx \
	ro_Intro.lyx \
	ro_TOC.lyx \
	ru_FAQ.lyx \
	ru_Intro.lyx \
	ru_TOC.lyx \
	ru_Tutorial.lyx \
	sk_TOC.lyx \
	sk_Tutorial.lyx \
	sk_UserGuide.lyx \
	sl_Intro.lyx \
	sl_TOC.lyx \
	sl_Tutorial.lyx \
	sv_Intro.lyx \
	sv_TOC.lyx \
	sv_Tutorial.lyx \
	TOC.lyx \
	Tutorial.lyx \
	UserGuide.lyx \
	escher-lsd.eps \
	mobius.eps \
	platypus.eps

docdir = $(pkgdatadir)/doc
doc_DATA = \
	$(docfiles) \
	LaTeXConfig.lyx

# TODO can we instead just #include the DEPENDFILE in this Makefile?
# problem with this is that the DEPENDFILE that's included won't be the updated
# one. JMarc should know how to handle this.

DEPENDFILE = Makefile.depend

TOCs : depend
	make -f $(DEPENDFILE) TOCs

depend:
	python depend.py > $(DEPENDFILE)

.PHONY: depend
