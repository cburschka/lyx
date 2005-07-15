include $(top_srcdir)/config/common.am

CLEANFILES += *.pyc *.pyo

lyx2lyxdir = $(pkgdatadir)/lyx2lyx
# We cannot use dist_lyx2lyx_SCRIPTS for lyx2lyx, since a possible
# version-suffix would get appended to the names. So we use dist_scripts_DATA
# and chmod manually in install-data-hook.
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
