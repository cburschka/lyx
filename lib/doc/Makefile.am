include $(top_srcdir)/config/common.am

DISTCLEANFILES += LyXConfig.lyx LaTeXConfig.lyx $(tocfiles)

EXTRA_DIST = depend.py doc_toc.py LyXConfig.lyx.in README.Documentation $(DEPENDFILE) $(docfiles)

docfiles = \
	cs_Tutorial.lyx \
	Customization.lyx \
	da_Intro.lyx \
	de_Customization.lyx \
	de_Extended.lyx \
	de_FAQ.lyx \
	de_Intro.lyx \
	de_Tutorial.lyx \
	de_UserGuide.lyx \
	DocStyle.lyx \
	es_Intro.lyx \
	es_Tutorial.lyx \
	eu_Customization.lyx \
	eu_Extended.lyx \
	eu_FAQ.lyx \
	eu_Intro.lyx \
	eu_Tutorial.lyx \
	eu_UserGuide.lyx \
	Extended.lyx \
	FAQ.lyx \
	fr_Customization.lyx \
	fr_Extended.lyx \
	fr_FAQ.lyx \
	fr_Intro.lyx \
	fr_Tutorial.lyx \
	fr_UserGuide.lyx \
	he_Intro.lyx \
	he_Tutorial.lyx \
	hu_Intro.lyx \
	Intro.lyx \
	it_Customization.lyx \
	it_Intro.lyx \
	it_Tutorial.lyx \
	it_UserGuide.lyx \
	LaTeXConfig.lyx.in \
	nl_Intro.lyx \
	nl_Tutorial.lyx \
	no_Intro.lyx \
	pl_Extended.lyx \
	pl_Intro.lyx \
	pl_Tutorial.lyx \
	pt_Intro.lyx \
	pt_Tutorial.lyx \
	Reference.lyx \
	ro_Intro.lyx \
	ru_FAQ.lyx \
	ru_Intro.lyx \
	ru_Tutorial.lyx \
	sk_Tutorial.lyx \
	sk_UserGuide.lyx \
	sl_Intro.lyx \
	sl_Tutorial.lyx \
	sv_Intro.lyx \
	sv_Tutorial.lyx \
	Tutorial.lyx \
	UserGuide.lyx \
	escher-lsd.eps \
	mobius.eps \
	platypus.eps

docdir = $(pkgdatadir)/doc
doc_DATA = \
	$(docfiles) \
	LaTeXConfig.lyx

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
