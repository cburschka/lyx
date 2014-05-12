include $(top_srcdir)/config/common.am

CLEANFILES += *.pyc *.pyo

EXTRA_DIST = lyx2lyx_version.py.in

CHMOD = chmod

lyx2lyxdir = $(pkgdatadir)/lyx2lyx
dist_lyx2lyx_PYTHON = \
	lyx2lyx \
	lyx2lyx_version.py \
	lyx2lyx_lang.py \
	generate_encoding_info.py \
	parser_tools.py \
	lyx2lyx_tools.py \
	unicode_symbols.py \
	LyX.py \
	lyx_0_06.py \
	lyx_0_08.py \
	lyx_0_10.py \
	lyx_0_12.py \
	lyx_1_0.py \
	lyx_1_1.py \
	lyx_1_1_5.py \
	lyx_1_1_6_0.py \
	lyx_1_1_6_3.py \
	lyx_1_2.py \
	lyx_1_3.py \
	lyx_1_4.py \
	lyx_1_5.py \
	lyx_1_6.py \
	lyx_2_0.py \
	lyx_2_1.py \
	lyx_2_2.py \
	profiling.py \
	test_parser_tools.py

install-data-hook:
	$(CHMOD) 755 $(DESTDIR)$(lyx2lyxdir)/lyx2lyx
