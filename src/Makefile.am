include $(top_srcdir)/config/common.am

############################### Core  ##############################

AM_CPPFLAGS += -I$(top_srcdir)/src $(BOOST_INCLUDES) $(ENCHANT_CFLAGS) $(HUNSPELL_CFLAGS)
AM_CPPFLAGS += $(QT_CPPFLAGS) $(QT_CORE_INCLUDES)

if BUILD_CLIENT_SUBDIR
CLIENT = client
endif

SUBDIRS = support frontends . $(CLIENT) tex2lyx

EXTRA_DIST = lyx_commit_hash.h.in \
	CMakeLists.txt \
	graphics/CMakeLists.txt \
	insets/CMakeLists.txt \
	mathed/CMakeLists.txt \
	tests/CMakeLists.txt

OTHERLIBS = $(BOOST_LIBS) $(MYTHES_LIBS) $(ENCHANT_LIBS) $(HUNSPELL_LIBS) \
            @LIBS@ $(SOCKET_LIBS) $(LIBSHLWAPI) $(LIBPSAPI)

noinst_LIBRARIES = liblyxcore.a
bin_PROGRAMS = lyx

lyx_LDADD = \
	liblyxcore.a \
	liblyxmathed.a \
	liblyxinsets.a \
	frontends/liblyxfrontends.a \
	frontends/qt4/liblyxqt4.a \
	liblyxgraphics.a \
	support/liblyxsupport.a \
	$(OTHERLIBS) \
	$(QT_LDFLAGS) \
	$(QT_LIB)

if LYX_WIN_RESOURCE
.rc.o:
	cp $(top_srcdir)/development/Win32/packaging/icons/lyx_*32x32.ico .
	windres -I$(top_builddir) --preprocessor "$(CPP) -xc-header -DRC_INVOKED" $< -o $@
endif

if INSTALL_MACOSX
lyx_LDFLAGS = -framework AppKit -framework ApplicationServices \
	-Wl,-rpath,@loader_path/../Frameworks \
	-Wl,-rpath,@executable_path/../Frameworks
endif

#lyx_LDFLAGS=-Wl,-O1

if INSTALL_MACOSX
APPLESPELL = AppleSpellChecker.cpp AppleSpellChecker.h
endif

if USE_ASPELL
ASPELL = AspellChecker.cpp AspellChecker.h
PWL = PersonalWordList.cpp PersonalWordList.h
endif

if USE_ENCHANT
ENCHANT = EnchantChecker.cpp EnchantChecker.h
PWL = PersonalWordList.cpp PersonalWordList.h
endif

if USE_HUNSPELL
HUNSPELL = HunspellChecker.cpp HunspellChecker.h
PWL = PersonalWordList.cpp PersonalWordList.h
endif

# These four objects are linked as object files as they are not
# referenced within the core and therefore are not picked up
# by the linker without looping over libs. We do not want that,
# and in fact libtools seems not able to do that.
lyx_SOURCES = \
	main.cpp \
	$(APPLESPELL) \
	$(ASPELL) \
	BiblioInfo.h \
	BiblioInfo.cpp \
	Box.cpp \
	Box.h \
	Compare.cpp \
	Compare.h \
	Dimension.cpp \
	Dimension.h \
	$(ENCHANT) \
	$(HUNSPELL) \
	$(PWL) \
	LaTeXFonts.cpp \
	LaTeXFonts.h \
	PrinterParams.cpp \
	PrinterParams.h \
	Thesaurus.cpp \
	Thesaurus.h

if LYX_WIN_RESOURCE
lyx_SOURCES += lyxwinres.rc
endif

SOURCEFILESCORE = \
	Author.cpp \
	boost.cpp \
	BranchList.cpp \
	Buffer.cpp \
	buffer_funcs.cpp \
	BufferList.cpp \
	BufferParams.cpp \
	BufferView.cpp \
	Bullet.cpp \
	Changes.cpp \
	Chktex.cpp \
	CmdDef.cpp \
	Color.cpp \
	ConverterCache.cpp \
	Converter.cpp \
	CoordCache.cpp \
	Counters.cpp \
	Cursor.cpp \
	CursorSlice.cpp \
	CutAndPaste.cpp \
	DepTable.cpp \
	DocIterator.cpp \
	Encoding.cpp \
	BufferEncodings.cpp \
	ErrorList.cpp \
	Exporter.cpp \
	factory.cpp \
	Floating.cpp \
	FloatList.cpp \
	FontInfo.cpp \
	FontList.cpp \
	Font.cpp \
	Format.cpp \
	FuncRequest.cpp \
	FuncStatus.cpp \
	Graph.cpp \
	HSpace.cpp \
	IndicesList.cpp \
	InsetIterator.cpp \
	InsetList.cpp \
	Intl.cpp \
	KeyMap.cpp \
	KeySequence.cpp \
	Language.cpp \
	LaTeX.cpp \
	LaTeXFeatures.cpp \
	LaTeXPackages.cpp \
	LayoutFile.cpp \
	LayoutModuleList.cpp \
	Length.cpp \
	lengthcommon.cpp \
	Lexer.cpp \
	LyX.cpp \
	LyXAction.cpp \
	lyxfind.cpp \
	LyXRC.cpp \
	LyXVC.cpp \
	MetricsInfo.cpp \
	ModuleList.cpp \
	Mover.cpp \
	output_docbook.cpp \
	output.cpp \
	output_latex.cpp \
	output_xhtml.cpp \
	OutputParams.cpp \
	output_plaintext.cpp \
	Paragraph.cpp \
	ParagraphMetrics.cpp \
	ParagraphParameters.cpp \
	ParIterator.cpp \
	PDFOptions.cpp \
	Row.cpp \
	RowPainter.cpp \
	Server.cpp \
	ServerSocket.cpp \
	sgml.cpp \
	Session.cpp \
	Spacing.cpp \
	TexRow.cpp \
	texstream.cpp \
	Text.cpp \
	Text2.cpp \
	Text3.cpp \
	TextClass.cpp \
	TextMetrics.cpp \
	TocBackend.cpp \
	Trans.cpp \
	Undo.cpp \
	VCBackend.cpp \
	version.cpp \
	VSpace.cpp \
	WordList.cpp

HEADERFILESCORE = \
	Author.h \
	BranchList.h \
	buffer_funcs.h \
	Buffer.h \
	BufferEncodings.h \
	BufferList.h \
	BufferParams.h \
	BufferView.h \
	Bullet.h \
	Citation.h \
	Changes.h \
	Chktex.h \
	CmdDef.h \
	ColorCode.h \
	Color.h \
	ColorSet.h \
	CompletionList.h \
	ConverterCache.h \
	Converter.h \
	CoordCache.h \
	Counters.h \
	Cursor.h \
	CursorSlice.h \
	CutAndPaste.h \
	DepTable.h \
	DispatchResult.h \
	DocIterator.h \
	DocumentClassPtr.h \
	Encoding.h \
	ErrorList.h \
	Exporter.h \
	factory.h \
	Floating.h \
	FloatList.h \
	Font.h \
	FontEnums.h \
	FontInfo.h \
	FontList.h \
	Format.h \
	FuncCode.h \
	FuncRequest.h \
	FuncStatus.h \
	Graph.h \
	HSpace.h \
	IndicesList.h \
	InsetIterator.h \
	InsetList.h \
	Intl.h \
	KeyMap.h \
	KeySequence.h \
	Language.h \
	LaTeXFeatures.h \
	LaTeXPackages.h \
	LaTeX.h \
	Layout.h \
	LayoutEnums.h \
	LayoutFile.h \
	LayoutModuleList.h \
	Length.h \
	Lexer.h \
	LyXAction.h \
	lyxfind.h \
	LyX.h \
	LyXRC.h \
	LyXVC.h \
	MetricsInfo.h \
	ModuleList.h \
	Mover.h \
	output_docbook.h \
	output.h \
	OutputEnums.h \
	output_latex.h \
	output_xhtml.h \
	OutputParams.h \
	output_plaintext.h \
	paper.h \
	Paragraph.h \
	ParagraphList.h \
	ParagraphMetrics.h \
	ParagraphParameters.h \
	ParIterator.h \
	PDFOptions.h \
	Row.h \
	RowPainter.h \
	Server.h \
	ServerSocket.h \
	Session.h \
	sgml.h \
	Spacing.h \
	SpellChecker.h \
	TexRow.h \
	texstream.h \
	Text.h \
	TextClass.h \
	TextMetrics.h \
	TocBackend.h \
	Trans.h \
	Undo.h \
	update_flags.h \
	VCBackend.h \
	version.h \
	VSpace.h \
	WordLangTuple.h \
	WordList.h

STANDALONEFILES = \
	Layout.cpp


.PHONY: update_commit_hash

LCH_V_GEN = $(lch__v_GEN_@AM_V@)
lch__v_GEN_ = $(lch__v_GEN_@AM_DEFAULT_V@)
lch__v_GEN_0 = @echo "  GEN      lyx_commit_hash.h";
lch__v_GEN_1 =

update_commit_hash:
	rm -f hash-temp \
	$(LCH_V_GEN)hash=`cd "$(top_srcdir)" && git log -1 --pretty=format:%H 2>/dev/null || echo none` ; \
	sed s/@LYX_GIT_COMMIT_HASH@/$$hash/ "$(srcdir)"/lyx_commit_hash.h.in >hash-temp ; \
	cmp -s lyx_commit_hash.h hash-temp || cp hash-temp lyx_commit_hash.h ; \
	rm -f hash-temp

lyx_commit_hash.h: update_commit_hash

BUILT_SOURCES = lyx_commit_hash.h
CLEANFILES = lyx_commit_hash.h

lyxcore.cpp:
	$(AM_V_GEN)for file in $(SOURCEFILESCORE) ; do echo '#include "'$${file}'"' ; done >$@

if MONOLITHIC_CORE

BUILT_SOURCES += lyxcore.cpp $(STANDALONEFILES)
CLEANFILES += lyxcore.cpp

liblyxcore_a_SOURCES = lyxcore.cpp $(STANDALONEFILES) $(HEADERFILESCORE)

else

liblyxcore_a_SOURCES = $(SOURCEFILESCORE) $(STANDALONEFILES) $(HEADERFILESCORE)

endif

#########################  Qt stuff  ##############################

MOCHEADER = Compare.h PreviewLoader.h

if INSTALL_WINDOWS

MOCHEADER += Server.h
MOCFLAG = -D_WIN32

endif

MOCEDFILES = $(MOCHEADER:%.h=moc_%.cpp)

BUILT_SOURCES += $(MOCEDFILES)
CLEANFILES += $(MOCEDFILES)

moc_PreviewLoader.cpp: graphics/PreviewLoader.h
	$(AM_V_GEN)$(QT_MOC) $(MOCFLAG) -o $@ $<

moc_%.cpp: %.h
	$(AM_V_GEN)$(QT_MOC) $(MOCFLAG) -o $@ $<

liblyxcore_a_DEPENDENCIES = $(MOCEDFILES)

############################### Graphics ##############################

noinst_LIBRARIES += liblyxgraphics.a

liblyxgraphics_a_SOURCES = \
	graphics/epstools.h \
	graphics/epstools.cpp \
	graphics/GraphicsCache.h \
	graphics/GraphicsCache.cpp \
	graphics/GraphicsCacheItem.h \
	graphics/GraphicsCacheItem.cpp \
	graphics/GraphicsConverter.h \
	graphics/GraphicsConverter.cpp \
	graphics/GraphicsImage.h \
	graphics/GraphicsLoader.h \
	graphics/GraphicsLoader.cpp \
	graphics/GraphicsParams.cpp \
	graphics/GraphicsParams.h \
	graphics/GraphicsTypes.h \
	graphics/PreviewImage.h \
	graphics/PreviewImage.cpp \
	graphics/PreviewLoader.h \
	graphics/PreviewLoader.cpp


############################### Mathed  ##############################

noinst_LIBRARIES += liblyxmathed.a

SOURCEFILESMATHED = \
	mathed/CommandInset.cpp \
	mathed/InsetMathAMSArray.cpp \
	mathed/InsetMathArray.cpp \
	mathed/InsetMathBig.cpp \
	mathed/InsetMathBoldSymbol.cpp \
	mathed/InsetMathBox.cpp \
	mathed/InsetMathBrace.cpp \
	mathed/InsetMath.cpp \
	mathed/InsetMathCases.cpp \
	mathed/InsetMathChar.cpp \
	mathed/InsetMathColor.cpp \
	mathed/InsetMathComment.cpp \
	mathed/InsetMathDecoration.cpp \
	mathed/InsetMathDecoration.h \
	mathed/InsetMathDelim.cpp \
	mathed/InsetMathDiff.cpp \
	mathed/InsetMathDots.cpp \
	mathed/InsetMathEnsureMath.cpp \
	mathed/InsetMathEnv.cpp \
	mathed/InsetMathExFunc.cpp \
	mathed/InsetMathExInt.cpp \
	mathed/InsetMathFont.cpp \
	mathed/InsetMathFontOld.cpp \
	mathed/InsetMathFrac.cpp \
	mathed/InsetMathGrid.cpp \
	mathed/InsetMathHull.cpp \
	mathed/InsetMathKern.cpp \
	mathed/InsetMathLefteqn.cpp \
	mathed/InsetMathLim.cpp \
	mathed/InsetMathCancel.cpp \
	mathed/InsetMathCancelto.cpp \
	mathed/InsetMathMatrix.cpp \
	mathed/InsetMathNest.cpp \
	mathed/InsetMathNumber.cpp \
	mathed/InsetMathOverset.cpp \
	mathed/InsetMathPar.cpp \
	mathed/InsetMathPhantom.cpp \
	mathed/InsetMathRef.cpp \
	mathed/InsetMathRoot.cpp \
	mathed/InsetMathScript.cpp \
	mathed/InsetMathSideset.cpp \
	mathed/InsetMathSize.cpp \
	mathed/InsetMathSpace.cpp \
	mathed/InsetMathSpecialChar.cpp \
	mathed/InsetMathSplit.cpp \
	mathed/InsetMathSqrt.cpp \
	mathed/InsetMathStackrel.cpp \
	mathed/InsetMathString.cpp \
	mathed/InsetMathSubstack.cpp \
	mathed/InsetMathSymbol.cpp \
	mathed/InsetMathTabular.cpp \
	mathed/InsetMathUnderset.cpp \
	mathed/InsetMathUnknown.cpp \
	mathed/InsetMathXArrow.cpp \
	mathed/InsetMathXYMatrix.cpp \
	mathed/InsetMathDiagram.cpp \
	mathed/MathAtom.cpp \
	mathed/MathAutoCorrect.cpp \
	mathed/MathData.cpp \
	mathed/MathExtern.cpp \
	mathed/MathFactory.cpp \
	mathed/MathMacro.cpp \
	mathed/MathMacroArgument.cpp \
	mathed/MacroTable.cpp \
	mathed/MathMacroTemplate.cpp \
	mathed/MathParser.cpp \
	mathed/MathStream.cpp \
	mathed/MathSupport.cpp \
	mathed/TextPainter.cpp

HEADERFILESMATHED = \
	mathed/CommandInset.h \
	mathed/InsetMathAMSArray.h \
	mathed/InsetMathArray.h \
	mathed/InsetMathBig.h \
	mathed/InsetMathBoldSymbol.h \
	mathed/InsetMathBox.h \
	mathed/InsetMathBrace.h \
	mathed/InsetMathCancel.h \
	mathed/InsetMathCancelto.h \
	mathed/InsetMathCases.h \
	mathed/InsetMathChar.h \
	mathed/InsetMathColor.h \
	mathed/InsetMathComment.h \
	mathed/InsetMathDelim.h \
	mathed/InsetMathDiff.h \
	mathed/InsetMathDots.h \
	mathed/InsetMathEnsureMath.h \
	mathed/InsetMathEnv.h \
	mathed/InsetMathExFunc.h \
	mathed/InsetMathExInt.h \
	mathed/InsetMathFont.h \
	mathed/InsetMathFontOld.h \
	mathed/InsetMathFrac.h \
	mathed/InsetMathGrid.h \
	mathed/InsetMath.h \
	mathed/InsetMathHull.h \
	mathed/InsetMathKern.h \
	mathed/InsetMathLefteqn.h \
	mathed/InsetMathLim.h \
	mathed/InsetMathMatrix.h \
	mathed/InsetMathNest.h \
	mathed/InsetMathNumber.h \
	mathed/InsetMathOverset.h \
	mathed/InsetMathPar.h \
	mathed/InsetMathPhantom.h \
	mathed/InsetMathRef.h \
	mathed/InsetMathRoot.h \
	mathed/InsetMathScript.h \
	mathed/InsetMathSideset.h \
	mathed/InsetMathSize.h \
	mathed/InsetMathSpace.h \
	mathed/InsetMathSpecialChar.h \
	mathed/InsetMathSplit.h \
	mathed/InsetMathSqrt.h \
	mathed/InsetMathStackrel.h \
	mathed/InsetMathString.h \
	mathed/InsetMathSubstack.h \
	mathed/InsetMathSymbol.h \
	mathed/InsetMathTabular.h \
	mathed/InsetMathUnderset.h \
	mathed/InsetMathUnknown.h \
	mathed/InsetMathXArrow.h \
	mathed/InsetMathXYMatrix.h \
	mathed/InsetMathDiagram.h \
	mathed/MathAtom.h \
	mathed/MathAutoCorrect.h \
	mathed/MathData.h \
	mathed/MathCompletionList.h \
	mathed/MathExtern.h \
	mathed/MathFactory.h \
	mathed/MathMacro.h \
	mathed/MathMacroArgument.h \
	mathed/MacroTable.h \
	mathed/MathMacroTemplate.h \
	mathed/MathParser.h \
	mathed/MathParser_flags.h \
	mathed/ReplaceData.h \
	mathed/MathStream.h \
	mathed/MathSupport.h \
	mathed/TextPainter.h

lyxmathed.cpp:
	$(AM_V_GEN)for file in $(SOURCEFILESMATHED) ; do echo '#include "'$${file}'"' ; done >$@

if MONOLITHIC_MATHED

BUILT_SOURCES += lyxmathed.cpp
CLEANFILES += lyxmathed.cpp

liblyxmathed_a_SOURCES = lyxmathed.cpp $(HEADERFILESMATHED)

else

liblyxmathed_a_SOURCES = $(SOURCEFILESMATHED) $(HEADERFILESMATHED)

endif

############################### Insets  ##############################

noinst_LIBRARIES += liblyxinsets.a

SOURCEFILESINSETS = \
	insets/ExternalSupport.cpp \
	insets/ExternalTemplate.cpp \
	insets/ExternalTransforms.cpp \
	insets/RenderButton.cpp \
	insets/RenderGraphic.cpp \
	insets/RenderPreview.cpp \
	insets/Inset.cpp \
	insets/InsetArgument.cpp \
	insets/InsetBibitem.cpp \
	insets/InsetBibtex.cpp \
	insets/InsetBox.cpp \
	insets/InsetBranch.cpp \
	insets/InsetCaption.cpp \
	insets/InsetCaptionable.cpp \
	insets/InsetCitation.cpp \
	insets/InsetCollapsable.cpp \
	insets/InsetCommand.cpp \
	insets/InsetCommandParams.cpp \
	insets/InsetERT.cpp \
	insets/InsetExternal.cpp \
	insets/InsetFlex.cpp \
	insets/InsetFloat.cpp \
	insets/InsetFloatList.cpp \
	insets/InsetFoot.cpp \
	insets/InsetFootlike.cpp \
	insets/InsetGraphicsParams.cpp \
	insets/InsetGraphics.cpp \
	insets/InsetHyperlink.cpp \
	insets/InsetInclude.cpp \
	insets/InsetIndex.cpp \
	insets/InsetInfo.cpp \
	insets/InsetIPA.cpp \
	insets/InsetIPAMacro.cpp \
	insets/InsetLabel.cpp \
	insets/InsetLayout.cpp \
	insets/InsetLine.cpp \
	insets/InsetListings.cpp \
	insets/InsetListingsParams.cpp \
	insets/InsetMarginal.cpp \
	insets/InsetNewline.cpp \
	insets/InsetNewpage.cpp \
	insets/InsetNomencl.cpp \
	insets/InsetNote.cpp \
	insets/InsetPhantom.cpp \
	insets/InsetPreview.cpp \
	insets/InsetQuotes.cpp \
	insets/InsetRef.cpp \
	insets/InsetScript.cpp \
	insets/InsetSeparator.cpp \
	insets/InsetSpace.cpp \
	insets/InsetSpecialChar.cpp \
	insets/InsetTabular.cpp \
	insets/InsetText.cpp \
	insets/InsetTOC.cpp \
	insets/InsetVSpace.cpp \
	insets/InsetWrap.cpp

HEADERFILESINSETS = \
	insets/ExternalSupport.h \
	insets/ExternalTemplate.h \
	insets/ExternalTransforms.h \
	insets/RenderBase.h \
	insets/RenderButton.h \
	insets/RenderGraphic.h \
	insets/RenderPreview.h \
	insets/Inset.h \
	insets/InsetArgument.h \
	insets/InsetBibitem.h \
	insets/InsetBibtex.h \
	insets/InsetBox.h \
	insets/InsetBranch.h \
	insets/InsetCaption.h \
	insets/InsetCaptionable.h \
	insets/InsetCitation.h \
	insets/InsetCode.h \
	insets/InsetCollapsable.h \
	insets/InsetCommand.h \
	insets/InsetCommandParams.h \
	insets/InsetERT.h \
	insets/InsetExternal.h \
	insets/InsetFlex.h \
	insets/InsetFloat.h \
	insets/InsetFoot.h \
	insets/InsetFloatList.h \
	insets/InsetFootlike.h \
	insets/InsetGraphicsParams.h \
	insets/InsetGraphics.h \
	insets/InsetHyperlink.h \
	insets/InsetInclude.h \
	insets/InsetIndex.h \
	insets/InsetInfo.h \
	insets/InsetIPA.h \
	insets/InsetIPAMacro.h \
	insets/InsetPreview.h \
	insets/InsetLabel.h \
	insets/InsetLayout.h \
	insets/InsetLine.h \
	insets/InsetListings.h \
	insets/InsetListingsParams.h \
	insets/InsetMarginal.h \
	insets/InsetNewline.h \
	insets/InsetNewpage.h \
	insets/InsetNomencl.h \
	insets/InsetNote.h \
	insets/InsetPhantom.h \
	insets/InsetQuotes.h \
	insets/InsetRef.h \
	insets/InsetScript.h \
	insets/InsetSeparator.h \
	insets/InsetSpace.h \
	insets/InsetSpecialChar.h \
	insets/InsetTabular.h \
	insets/InsetText.h \
	insets/InsetTOC.h \
	insets/InsetVSpace.h \
	insets/InsetWrap.h

#	insets/InsetList.cpp \
#	insets/InsetList.h \
#	insets/InsetSection.h \
#	insets/InsetSection.cpp

lyxinsets.cpp:
	$(AM_V_GEN)for file in $(SOURCEFILESINSETS) ; do echo '#include "'$${file}'"' ; done >$@

if MONOLITHIC_INSETS

BUILT_SOURCES += lyxinsets.cpp
CLEANFILES += lyxinsets.cpp

liblyxinsets_a_SOURCES = lyxinsets.cpp $(HEADERFILESINSETS)

else

liblyxinsets_a_SOURCES = $(SOURCEFILESINSETS) $(HEADERFILESINSETS)

endif


############################## Tests ##################################

EXTRA_DIST += \
	tests/test_ExternalTransforms \
	tests/test_ListingsCaption \
	tests/regfiles/ExternalTransforms \
	tests/regfiles/Length \
	tests/regfiles/ListingsCaption \
	tests/test_layout \
	tests/test_Length

TESTS = tests/test_ExternalTransforms tests/test_Length tests/test_ListingsCaption

alltests: check alltests-recursive

alltests-recursive: check_layout
	@srcdir=$(srcdir) $(srcdir)/tests/test_layout; \
	if test $$? -eq 0; then \
		echo -e "====================\nlayout tests passed.\n===================="; \
	else \
		echo -e "====================\nlayout tests failed.\n===================="; \
	fi
	cd tex2lyx; $(MAKE) alltests-recursive

updatetests:
	cd tex2lyx; $(MAKE) updatetests

check_PROGRAMS = \
	check_ExternalTransforms \
	check_Length \
	check_ListingsCaption \
	check_layout

if INSTALL_MACOSX
ADD_FRAMEWORKS = -framework QtGui -framework QtCore -framework AppKit -framework ApplicationServices
endif

check_layout_CPPFLAGS = $(AM_CPPFLAGS)
check_layout_LDADD = support/liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) @LIBS@ $(QT_LIB) $(LIBSHLWAPI)
check_layout_LDFLAGS = $(QT_LDFLAGS) $(ADD_FRAMEWORKS)
check_layout_SOURCES = \
	insets/InsetLayout.cpp \
	Color.cpp \
	Counters.cpp \
	Floating.cpp \
	FloatList.cpp \
	FontInfo.cpp \
	Layout.cpp \
	LayoutFile.cpp \
	Lexer.cpp \
	ModuleList.cpp \
	Spacing.cpp \
	TextClass.cpp \
	tests/check_layout.cpp \
	tests/boost.cpp \
	tests/dummy_functions.cpp

check_ExternalTransforms_CPPFLAGS = $(AM_CPPFLAGS)
check_ExternalTransforms_LDADD = support/liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) @LIBS@ $(QT_LIB) $(LIBSHLWAPI)
check_ExternalTransforms_LDFLAGS = $(QT_LDFLAGS) $(ADD_FRAMEWORKS)
check_ExternalTransforms_SOURCES = \
	graphics/GraphicsParams.cpp \
	insets/ExternalTransforms.cpp \
	Length.cpp \
	lengthcommon.cpp \
	tests/check_ExternalTransforms.cpp \
	tests/boost.cpp \
	tests/dummy_functions.cpp

check_Length_CPPFLAGS = $(AM_CPPFLAGS)
check_Length_LDADD = support/liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) @LIBS@ $(QT_LIB) $(LIBSHLWAPI)
check_Length_LDFLAGS = $(QT_LDFLAGS) $(ADD_FRAMEWORKS)
check_Length_SOURCES = \
	Length.cpp \
	lengthcommon.cpp \
	tests/check_Length.cpp \
	tests/boost.cpp \
	tests/dummy_functions.cpp

check_ListingsCaption_CPPFLAGS = $(AM_CPPFLAGS)
check_ListingsCaption_LDADD = support/liblyxsupport.a $(LIBICONV) $(BOOST_LIBS) @LIBS@ $(QT_LIB) $(LIBSHLWAPI)
check_ListingsCaption_LDFLAGS = $(QT_LDFLAGS) $(ADD_FRAMEWORKS)
check_ListingsCaption_SOURCES = \
	tests/check_ListingsCaption.cpp \
	tests/boost.cpp \
	tests/dummy_functions.cpp

.PHONY: alltests alltests-recursive updatetests
