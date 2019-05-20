include $(top_srcdir)/config/common.am

EXTRA_DIST = $(TEST_FILES)

TEST_FILES = \
	export/xhtml/IncludeMissingEndTagDiv.lyx \
	export/xhtml/MissingEndTagDiv.lyx \
	export/latex/languages/ja-listings-uncodable-error.lyx \
	export/MissingEndLayoutBetweenTables.lyx \
	export/latex/inputenc-utf8-plain.lyx \
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
