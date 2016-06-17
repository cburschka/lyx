include $(top_srcdir)/config/common.am

EXTRA_DIST = $(TEST_FILES)

TEST_FILES = \
	export/IncludeMissingEndTagDiv.lyx \
	export/ja/wrong_auto_encoding.lyx \
	export/languagenesting1.lyx \
	export/languagenesting2.lyx \
	export/LongestLabelWithUnderscore.lyx \
	export/MissingEndLayoutBetweenTables.lyx \
	export/MissingEndTagDiv.lyx \
	export/TableErrorIfSlovakTextInserted.lyx \
	export/utf8-plain-with-tex-fonts.lyx \
	export/WrongDfnTagHandling.lyx \
	mathmacros/architecture.lyx \
	mathmacros/architecture.lyx \
	mathmacros/Spacechild1.lyx \
	mathmacros/Spacechild2.lyx \
	mathmacros/Spacechild3.lyx \
	mathmacros/testcases_basic.lyx \
	mathmacros/testcases_environments.lyx \
	mathmacros/testcases_speed.lyx

alltests: check alltests-recursive

.PHONY: alltests alltests-recursive
