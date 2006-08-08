include $(top_srcdir)/config/common.am

CLEANFILES += *.pyc *.pyo

EXTRA_DIST = lyx2lyx_version.py.in

lyx2lyxdir = $(pkgdatadir)/lyx2lyx
# We cannot use dist_lyx2lyx_SCRIPTS for lyx2lyx, since a possible
# version-suffix would get appended to the names. So we use dist_scripts_DATA
# and chmod manually in install-data-hook.
dist_lyx2lyx_PYTHON = \
	lyx2lyx \
	lyx2lyx_version.py \
	lyx2lyx_lang.py \
	generate_enconding_info.py \
	parser_tools.py \
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
	profiling.py \
	test_parser_tools.py
