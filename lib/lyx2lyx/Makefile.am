include $(top_srcdir)/config/common.am

CLEANFILES += *.pyc *.pyo

CHMOD = chmod

lyx2lyxdir = $(pkgdatadir)/lyx2lyx
dist_lyx2lyx_PYTHON = \
	lyx2lyx \
	parser_tools.py \
	LyX.py \
	lyx_0_12.py \
	lyx_1_0_0.py \
	lyx_1_0_1.py \
	lyx_1_1_4.py \
	lyx_1_1_5.py \
	lyx_1_1_6.py \
	lyx_1_1_6fix3.py \
	lyx_1_2.py \
	lyx_1_3.py \
	lyx_1_4.py \
	profiling.py

install-data-hook:
	$(CHMOD) 755 $(DESTDIR)$(lyx2lyxdir)/lyx2lyx
