include $(top_srcdir)/config/common.am

man1_MANS = dt2dv.man dv2dt.man

bin_PROGRAMS = dt2dv dv2dt

EXTRA_DIST = \
	ChangeLog-LyX \
	CMakeLists.txt \
	dtl.doc \
	dvi.doc \
	edited.txt \
	example.tex \
	hello.tex \
	man2ps \
	README \
	tripvdu.tex

HEADERFILES = dtl.h

dt2dv_SOURCES = dt2dv.c $(HEADERFILES)

dv2dt_SOURCES = dv2dt.c $(HEADERFILES)

