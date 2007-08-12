include $(top_srcdir)/config/common.am

############################### Core  ##############################

DISTCLEANFILES += config.h libintl.h version.cpp stamp-version version.cpp-tmp

MAINTAINERCLEANFILES += $(srcdir)/config.h.in

SUBDIRS = support frontends .

EXTRA_DIST = config.h.in stamp-h.in version.cpp.in \
	Section.h \
	Section.cpp \
	Variables.cpp \
	Variables.h \
	paper.h \
	pch.h

LYX_PRE_LIBS = liblyxmathed.la liblyxinsets.la \
	frontends/liblyxfrontends.la

LYX_POST_LIBS = frontends/controllers/liblyxcontrollers.la \
	liblyxgraphics.la \
	support/liblyxsupport.la

OTHERLIBS = $(BOOST_LIBS) $(INTLLIBS) $(AIKSAURUS_LIBS) @LIBS@ $(SOCKET_LIBS)

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

pkglib_LTLIBRARIES = liblyxcore.la
noinst_PROGRAMS = $(FRONTENDS_PROGS)
EXTRA_PROGRAMS = lyx-qt4

lyx_qt4_LDADD = liblyxcore.la  $(LYX_PRE_LIBS) \
	frontends/qt4/liblyxqt4.la $(LYX_POST_LIBS) $(OTHERLIBS) $(QT4_LIB) 
lyx_qt4_SOURCES = main.cpp

lyx$(EXEEXT): $(FRONTENDS_PROGS)
	rm -f $@
	$(LN_S) $< $@

#lyx_LDFLAGS=-Wl,-O1

BUILT_SOURCES = $(PCH_FILE) version.cpp

if USE_ASPELL
ASPELL = ASpell.cpp ASpell_local.h
endif
if USE_PSPELL
PSPELL = PSpell.cpp PSpell.h
endif
if USE_ISPELL
ISPELL = ISpell.cpp ISpell.h
endif


liblyxcore_la_SOURCES = \
	$(ASPELL) $(PSPELL) $(ISPELL) SpellBase.cpp \
	Author.cpp \
	Author.h \
	Bidi.cpp \
	Bidi.h \
	boost.cpp \
	Box.cpp \
	Box.h \
	BranchList.cpp \
	BranchList.h \
	Buffer.cpp \
	buffer_funcs.cpp \
	buffer_funcs.h \
	Buffer.h \
	BufferList.cpp \
	BufferList.h \
	BufferParams.cpp \
	BufferParams.h \
	BufferView.cpp \
	bufferview_funcs.cpp \
	bufferview_funcs.h \
	BufferView.h \
	Bullet.cpp \
	Bullet.h \
	Changes.cpp \
	Changes.h \
	Chktex.cpp \
	Chktex.h \
	Color.cpp \
	Color.h \
	config.h.in \
	ConverterCache.cpp \
	ConverterCache.h \
	Converter.cpp \
	Converter.h \
	CoordCache.cpp \
	CoordCache.h \
	Counters.cpp \
	Counters.h \
	Cursor.cpp \
	Cursor.h \
	CursorSlice.cpp \
	CursorSlice.h \
	CutAndPaste.cpp \
	CutAndPaste.h \
	debug.cpp \
	debug.h \
	DepTable.cpp \
	DepTable.h \
	Dimension.cpp \
	Dimension.h \
	DispatchResult.h \
	DocIterator.cpp \
	DocIterator.h \
	Encoding.cpp \
	Encoding.h \
	ErrorList.cpp \
	ErrorList.h \
	Exporter.cpp \
	Exporter.h \
	factory.cpp \
	factory.h \
	Floating.cpp \
	Floating.h \
	FloatList.cpp \
	FloatList.h \
	Font.cpp \
	Font.h \
	FontIterator.cpp \
	FontIterator.h \
	Format.cpp \
	Format.h \
	FuncRequest.cpp \
	FuncRequest.h \
	FuncStatus.cpp \
	FuncStatus.h \
	gettext.cpp \
	gettext.h \
	Graph.cpp \
	Graph.h \
	Importer.cpp \
	Importer.h \
	InsetIterator.cpp \
	InsetIterator.h \
	InsetList.cpp \
	InsetList.h \
	Intl.cpp \
	Intl.h \
	KeyMap.cpp \
	KeyMap.h \
	KeySequence.cpp \
	KeySequence.h \
	Language.cpp \
	Language.h \
	LaTeX.cpp \
	LaTeXFeatures.cpp \
	LaTeXFeatures.h \
	LaTeX.h \
	Layout.cpp \
	Layout.h \
	Length.cpp \
	Length.h \
	lengthcommon.cpp \
	lengthcommon.h \
	Lexer.cpp \
	Lexer.h \
	lfuns.h \
	LyXAction.cpp \
	LyXAction.h \
	callback.cpp \
	callback.h \
	LyX.cpp \
	lyxfind.cpp \
	lyxfind.h \
	LyXFunc.cpp \
	LyXFunc.h \
	LyX.h \
	lyxlayout_ptr_fwd.h \
	LyXRC.cpp \
	LyXRC.h \
	Server.cpp \
	Server.h \
	ServerSocket.cpp \
	ServerSocket.h \
	LyXVC.cpp \
	LyXVC.h \
	MenuBackend.cpp \
	MenuBackend.h \
	Messages.cpp \
	Messages.h \
	MetricsInfo.cpp \
	MetricsInfo.h \
	Mover.cpp \
	Mover.h \
	output.cpp \
	output_docbook.cpp \
	output_docbook.h \
	output.h \
	output_latex.cpp \
	output_latex.h \
	OutputParams.cpp \
	OutputParams.h \
	output_plaintext.cpp \
	output_plaintext.h \
	paper.h \
	Paragraph.cpp \
	paragraph_funcs.cpp \
	paragraph_funcs.h \
	Paragraph.h \
	ParagraphList.h \
	ParagraphMetrics.cpp \
	ParagraphMetrics.h \
	ParagraphParameters.cpp \
	ParagraphParameters.h \
	ParIterator.cpp \
	ParIterator.h \
	PrinterParams.cpp \
	PrinterParams.h \
	Row.cpp \
	Row.h \
	rowpainter.cpp \
	rowpainter.h \
	Session.cpp \
	Session.h \
	sgml.cpp \
	sgml.h \
	Spacing.cpp \
	Spacing.h \
	SpellBase.h \
	TexRow.cpp \
	TexRow.h \
	TexStream.cpp \
	TexStream.h \
	Text.h \
	Text.cpp \
	Text2.cpp \
	Text3.cpp \
	TextClass.cpp \
	TextClass.h \
	TextClassList.cpp \
	TextClassList.h \
	TextMetrics.cpp \
	TextMetrics.h \
	Thesaurus.cpp \
	Thesaurus.h \
	TocBackend.cpp \
	TocBackend.h \
	toc.cpp \
	toc.h \
	ToolbarBackend.cpp \
	ToolbarBackend.h \
	Trans.cpp \
	Trans.h \
	Undo.cpp \
	Undo.h \
	update_flags.h \
	VCBackend.cpp \
	VCBackend.h \
	version.cpp \
	version.h \
	WordLangTuple.h \
	VSpace.cpp \
	VSpace.h

version.cpp: stamp-version
	@:

stamp-version: version.cpp-tmp
	if cmp -s $< version.cpp ; then \
		: ;\
	else \
		rm -f version.cpp ;\
		cp $< version.cpp ;\
	fi
	echo timestamp > $@


############################### Graphics ##############################

pkglib_LTLIBRARIES += liblyxgraphics.la

liblyxgraphics_la_SOURCES = \
	graphics/GraphicsCache.h \
	graphics/GraphicsCache.cpp \
	graphics/GraphicsCacheItem.h \
	graphics/GraphicsCacheItem.cpp \
	graphics/GraphicsConverter.h \
	graphics/GraphicsConverter.cpp \
	graphics/GraphicsImage.h \
	graphics/GraphicsImage.cpp \
	graphics/GraphicsLoader.h \
	graphics/GraphicsLoader.cpp \
	graphics/GraphicsParams.cpp \
	graphics/GraphicsParams.h \
	graphics/LoaderQueue.h \
	graphics/LoaderQueue.cpp \
	graphics/GraphicsTypes.h \
	graphics/GraphicsTypes.cpp \
	graphics/PreviewImage.h \
	graphics/PreviewImage.cpp \
	graphics/PreviewLoader.h \
	graphics/PreviewLoader.cpp \
	graphics/Previews.h \
	graphics/Previews.cpp


############################### Mathed  ##############################

EXTRA_DIST += mathed/InsetFormulaMacro.cpp

pkglib_LTLIBRARIES += liblyxmathed.la

liblyxmathed_la_SOURCES = \
	mathed/InsetMathAMSArray.cpp \
	mathed/InsetMathAMSArray.h \
	mathed/InsetMathArray.cpp \
	mathed/InsetMathArray.h \
	mathed/InsetMathBig.cpp \
	mathed/InsetMathBig.h \
	mathed/InsetMathBinom.cpp \
	mathed/InsetMathBinom.h \
	mathed/InsetMathBoldSymbol.cpp \
	mathed/InsetMathBoldSymbol.h \
	mathed/InsetMathBox.cpp \
	mathed/InsetMathBoxed.cpp \
	mathed/InsetMathBoxed.h \
	mathed/InsetMathBox.h \
	mathed/InsetMathBrace.cpp \
	mathed/InsetMathBrace.h \
	mathed/InsetMath.cpp \
	mathed/InsetMathCases.cpp \
	mathed/InsetMathCases.h \
	mathed/InsetMathChar.cpp \
	mathed/InsetMathChar.h \
	mathed/InsetMathColor.cpp \
	mathed/InsetMathColor.h \
	mathed/CommandInset.cpp \
	mathed/CommandInset.h \
	mathed/InsetMathComment.cpp \
	mathed/InsetMathComment.h \
	mathed/InsetMathDecoration.cpp \
	mathed/InsetMathDecoration.h \
	mathed/InsetMathDelim.cpp \
	mathed/InsetMathDelim.h \
	mathed/InsetMathDFrac.cpp \
	mathed/InsetMathDFrac.h \
	mathed/InsetMathDiff.cpp \
	mathed/InsetMathDiff.h \
	mathed/InsetMathDots.cpp \
	mathed/InsetMathDots.h \
	mathed/InsetMathEnv.cpp \
	mathed/InsetMathEnv.h \
	mathed/InsetMathExFunc.cpp \
	mathed/InsetMathExFunc.h \
	mathed/InsetMathExInt.cpp \
	mathed/InsetMathExInt.h \
	mathed/InsetMathFBox.cpp \
	mathed/InsetMathFBox.h \
	mathed/InsetMathFont.cpp \
	mathed/InsetMathFont.h \
	mathed/InsetMathFontOld.cpp \
	mathed/InsetMathFontOld.h \
	mathed/InsetMathFracBase.cpp \
	mathed/InsetMathFracBase.h \
	mathed/InsetMathFrac.cpp \
	mathed/InsetMathFrac.h \
	mathed/InsetMathFrameBox.cpp \
	mathed/InsetMathFrameBox.h \
	mathed/InsetMathGrid.cpp \
	mathed/InsetMathGrid.h \
	mathed/InsetMath.h \
	mathed/InsetMathHull.cpp \
	mathed/InsetMathHull.h \
	mathed/InsetMathKern.cpp \
	mathed/InsetMathKern.h \
	mathed/InsetMathLefteqn.cpp \
	mathed/InsetMathLefteqn.h \
	mathed/InsetMathLim.cpp \
	mathed/InsetMathLim.h \
	mathed/MathMacro.cpp \
	mathed/MathMacro.h \
	mathed/InsetMathMakebox.cpp \
	mathed/InsetMathMakebox.h \
	mathed/InsetMathMatrix.cpp \
	mathed/InsetMathMatrix.h \
	mathed/InsetMathNest.cpp \
	mathed/InsetMathNest.h \
	mathed/InsetMathNumber.cpp \
	mathed/InsetMathNumber.h \
	mathed/InsetMathOverset.cpp \
	mathed/InsetMathOverset.h \
	mathed/InsetMathPar.cpp \
	mathed/InsetMathPar.h \
	mathed/InsetMathPhantom.cpp \
	mathed/InsetMathPhantom.h \
	mathed/InsetMathRef.cpp \
	mathed/InsetMathRef.h \
	mathed/InsetMathRoot.cpp \
	mathed/InsetMathRoot.h \
	mathed/InsetMathScript.cpp \
	mathed/InsetMathScript.h \
	mathed/InsetMathSize.cpp \
	mathed/InsetMathSize.h \
	mathed/InsetMathSpace.cpp \
	mathed/InsetMathSpace.h \
	mathed/InsetMathSplit.cpp \
	mathed/InsetMathSplit.h \
	mathed/InsetMathSqrt.cpp \
	mathed/InsetMathSqrt.h \
	mathed/InsetMathStackrel.cpp \
	mathed/InsetMathStackrel.h \
	mathed/InsetMathString.cpp \
	mathed/InsetMathString.h \
	mathed/InsetMathSubstack.cpp \
	mathed/InsetMathSubstack.h \
	mathed/InsetMathSymbol.cpp \
	mathed/InsetMathSymbol.h \
	mathed/InsetMathTabular.cpp \
	mathed/InsetMathTabular.h \
	mathed/InsetMathTFrac.cpp \
	mathed/InsetMathTFrac.h \
	mathed/InsetMathUnderset.cpp \
	mathed/InsetMathUnderset.h \
	mathed/InsetMathUnknown.cpp \
	mathed/InsetMathUnknown.h \
	mathed/InsetMathXArrow.cpp \
	mathed/InsetMathXArrow.h \
	mathed/InsetMathXYMatrix.cpp \
	mathed/InsetMathXYMatrix.h \
	mathed/MathAtom.cpp \
	mathed/MathAtom.h \
	mathed/MathAutoCorrect.cpp \
	mathed/MathAutoCorrect.h \
	mathed/MathData.cpp \
	mathed/MathData.h \
	mathed/MathExtern.cpp \
	mathed/MathExtern.h \
	mathed/MathFactory.cpp \
	mathed/MathFactory.h \
	mathed/MathGridInfo.h \
	mathed/MathMacroArgument.cpp \
	mathed/MathMacroArgument.h \
	mathed/MacroTable.cpp \
	mathed/MacroTable.h \
	mathed/MathMacroTemplate.cpp \
	mathed/MathMacroTemplate.h \
	mathed/MathParser.cpp \
	mathed/MathParser.h \
	mathed/ReplaceData.h \
	mathed/MathStream.cpp \
	mathed/MathStream.h \
	mathed/MathSupport.cpp \
	mathed/MathSupport.h \
	mathed/TextPainter.cpp \
	mathed/TextPainter.h 
#	mathed/InsetMathMBox.cpp
#	mathed/InsetMathMBox.h

############################### Insets  ##############################

pkglib_LTLIBRARIES += liblyxinsets.la

EXTRA_DIST += \
	insets/InsetTheorem.cpp \
	insets/InsetTheorem.h

liblyxinsets_la_SOURCES = \
	insets/MailInset.cpp \
	insets/MailInset.h \
	insets/ExternalSupport.cpp \
	insets/ExternalSupport.h \
	insets/ExternalTemplate.cpp \
	insets/ExternalTemplate.h \
	insets/ExternalTransforms.cpp \
	insets/ExternalTransforms.h \
	insets/RenderBase.h \
	insets/RenderButton.cpp \
	insets/RenderButton.h \
	insets/RenderGraphic.cpp \
	insets/RenderGraphic.h \
	insets/RenderPreview.cpp \
	insets/RenderPreview.h \
	insets/Inset.h \
	insets/Inset.cpp \
	insets/InsetBibitem.cpp \
	insets/InsetBibitem.h \
	insets/InsetBibtex.cpp \
	insets/InsetBibtex.h \
	insets/InsetBox.cpp \
	insets/InsetBox.h \
	insets/InsetBranch.cpp \
	insets/InsetBranch.h \
	insets/InsetCaption.cpp \
	insets/InsetCaption.h \
	insets/InsetCharStyle.cpp \
	insets/InsetCharStyle.h \
	insets/InsetCitation.cpp \
	insets/InsetCitation.h \
	insets/InsetCollapsable.cpp \
	insets/InsetCollapsable.h \
	insets/InsetCommand.cpp \
	insets/InsetCommand.h \
	insets/InsetCommandParams.cpp \
	insets/InsetCommandParams.h \
	insets/InsetEnvironment.cpp \
	insets/InsetEnvironment.h \
	insets/InsetERT.cpp \
	insets/InsetERT.h \
	insets/InsetExternal.cpp \
	insets/InsetExternal.h \
	insets/InsetFloat.h \
	insets/InsetFloat.cpp \
	insets/InsetFloatList.cpp \
	insets/InsetFloatList.h \
	insets/InsetFoot.cpp \
	insets/InsetFoot.h \
	insets/InsetFootlike.cpp \
	insets/InsetFootlike.h \
	insets/InsetGraphicsParams.h \
	insets/InsetGraphicsParams.cpp \
	insets/InsetGraphics.cpp \
	insets/InsetGraphics.h \
	insets/InsetHFill.cpp \
	insets/InsetHFill.h \
	insets/InsetInclude.cpp \
	insets/InsetInclude.h \
	insets/InsetIndex.cpp \
	insets/InsetIndex.h \
	insets/InsetLabel.cpp \
	insets/InsetLabel.h \
	insets/InsetLine.cpp \
	insets/InsetLine.h \
	insets/InsetListings.h \
	insets/InsetListings.cpp \
	insets/InsetListingsParams.h \
	insets/InsetListingsParams.cpp \
	insets/InsetMarginal.h \
	insets/InsetMarginal.cpp \
	insets/InsetNewline.cpp \
	insets/InsetNewline.h \
	insets/InsetNomencl.cpp \
	insets/InsetNomencl.h \
	insets/InsetNote.cpp \
	insets/InsetNote.h \
	insets/InsetOptArg.cpp \
	insets/InsetOptArg.h \
	insets/InsetPagebreak.cpp \
	insets/InsetPagebreak.h \
	insets/InsetQuotes.cpp \
	insets/InsetQuotes.h \
	insets/InsetRef.cpp \
	insets/InsetRef.h \
	insets/InsetSpace.cpp \
	insets/InsetSpace.h \
	insets/InsetSpecialChar.cpp \
	insets/InsetSpecialChar.h \
	insets/InsetTabular.cpp \
	insets/InsetTabular.h \
	insets/InsetText.cpp \
	insets/InsetText.h \
	insets/InsetTOC.cpp \
	insets/InsetTOC.h \
	insets/InsetUrl.cpp \
	insets/InsetUrl.h \
	insets/InsetVSpace.cpp \
	insets/InsetVSpace.h \
	insets/InsetWrap.h \
	insets/InsetWrap.cpp

#	insets/insetlist.C \
#	insets/insetlist.h \
#	insets/insetsection.h \
#	insets/insetsection.C \
#	insets/InsetTheorem.cpp \
#	insets/InsetTheorem.h

