include $(top_srcdir)/config/common.am

EXTRA_DIST = Doxyfile.in

doxydoc: Doxyfile
	doxygen ./Doxyfile

distclean-local:
	rm -rf $(srcdir)/html $(srcdir)/latex $(srcdir)/main

.PHONY: doxydoc
