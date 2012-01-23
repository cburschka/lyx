from SCons.Util import Split

TOP_extra_files = Split('''
    ABOUT-NLS
    ANNOUNCE
    autogen.sh
    config.log
    configure.ac
    COPYING
    INSTALL
    INSTALL.MacOSX
    INSTALL.Win32
    INSTALL.autoconf
    INSTALL.cmake
    INSTALL.scons
    lyx.1in
    Makefile.am
    NEWS
    README
    README.Cygwin
    README.Win32
    README.localization
    RELEASE-NOTES
    rename.sh
    scons_lyx.log
    UPGRADING
''')


src_header_files = Split('''
    AppleSpellChecker.h
    AspellChecker.h
    Author.h
    BiblioInfo.h
    Bidi.h
    Box.h
    BranchList.h
    Buffer.h
    buffer_funcs.h
    BufferList.h
    BufferParams.h
    BufferView.h
    Bullet.h
    Changes.h
    Chktex.h
    Citation.h
    CmdDef.h
    Color.h
    ColorSet.h
    ColorCode.h
    Compare.h
    CompletionList.h
    Converter.h
    ConverterCache.h
    CoordCache.h
    Counters.h
    Cursor.h
    CursorSlice.h
    CutAndPaste.h
    DepTable.h
    Dimension.h
    DispatchResult.h
    DocIterator.h
    EnchantChecker.h
    Encoding.h
    ErrorList.h
    Exporter.h
    factory.h
    FloatList.h
    Floating.h
    Font.h
    FontEnums.h
    FontInfo.h
    FontList.h
    Format.h
    FuncCode.h
    FuncRequest.h
    FuncStatus.h
    Graph.h
    HSpace.h
    HunspellChecker.h
    IndicesList.h
    InsetIterator.h
    InsetList.h
    Intl.h
    KeyMap.h
    KeySequence.h
    LaTeX.h
    LaTeXFeatures.h
    LaTeXPackages.h
    Language.h
    Layout.h
    LayoutEnums.h
    LayoutFile.h
    LayoutModuleList.h
    Length.h
    Lexer.h
    LyX.h
    LyXAction.h
    lyxfind.h
    LyXRC.h
    LyXVC.h
    MetricsInfo.h
    ModuleList.h
    Mover.h
    output.h
    output_docbook.h
    output_latex.h
    output_plaintext.h
    output_xhtml.h
    OutputEnums.h
    OutputParams.h
    paper.h
    ParIterator.h
    Paragraph.h
    ParagraphList.h
    ParagraphMetrics.h
    ParagraphParameters.h
    PDFOptions.h
    PersonalWordList.h
    PrinterParams.h
    Row.h
    rowpainter.h
    Server.h
    ServerSocket.h
    Session.h
    sgml.h
    Spacing.h
    SpellChecker.h
    TexRow.h
    Text.h
    TextClass.h
    TextMetrics.h
    Thesaurus.h
    TocBackend.h
    Trans.h
    Undo.h
    update_flags.h
    VCBackend.h
    VSpace.h
    version.h
    WordLangTuple.h
    WordList.h
''')


src_pre_files = Split('''
    Author.cpp
    Bidi.cpp
    BranchList.cpp
    Buffer.cpp
    BufferList.cpp
    BufferParams.cpp
    BufferView.cpp
    buffer_funcs.cpp
    Bullet.cpp
    Changes.cpp
    Chktex.cpp
    CmdDef.cpp
    Color.cpp
    Compare.cpp
    Converter.cpp
    ConverterCache.cpp
    CoordCache.cpp
    Counters.cpp
    Cursor.cpp
    CursorSlice.cpp
    CutAndPaste.cpp
    DepTable.cpp
    DocIterator.cpp
    Encoding.cpp
    ErrorList.cpp
    Exporter.cpp
    factory.cpp
    FloatList.cpp
    Floating.cpp
    Font.cpp
    FontInfo.cpp
    FontList.cpp
    Format.cpp
    FuncRequest.cpp
    FuncStatus.cpp
    Graph.cpp
    HSpace.cpp
    IndicesList.cpp
    InsetIterator.cpp
    InsetList.cpp
    Intl.cpp
    KeyMap.cpp
    KeySequence.cpp
    LaTeX.cpp
    LaTeXFeatures.cpp
    LaTeXPackages.cpp
    Language.cpp
    Layout.cpp
    LayoutFile.cpp
    LayoutModuleList.cpp
    Length.cpp
    lengthcommon.cpp
    Lexer.cpp
    LyX.cpp
    LyXAction.cpp
    lyxfind.cpp
    LyXRC.cpp
    LyXVC.cpp
    MetricsInfo.cpp
    Mover.cpp
    output.cpp
    OutputParams.cpp
    output_docbook.cpp
    output_latex.cpp
    output_plaintext.cpp
    output_xhtml.cpp
    PDFOptions.cpp
    ParIterator.cpp
    Paragraph.cpp
    ParagraphMetrics.cpp
    ParagraphParameters.cpp
    PersonalWordList.cpp
    Row.cpp
    rowpainter.cpp
    Server.cpp
    ServerSocket.cpp
    Session.cpp
    sgml.cpp
    Spacing.cpp
    TexRow.cpp
    Text.cpp
    Text2.cpp
    Text3.cpp
    TextClass.cpp
    TextMetrics.cpp
    TocBackend.cpp
    Trans.cpp
    Undo.cpp
    VCBackend.cpp
    version.cpp
    VSpace.cpp
    WordList.cpp
''')


src_post_files = Split('''
    BiblioInfo.cpp
    boost.cpp
    Box.cpp
    Dimension.cpp
    ModuleList.cpp
    PrinterParams.cpp
    Thesaurus.cpp
''')


src_extra_src_files = Split('''
    AppleSpellChecker.cpp
    AspellChecker.cpp
    EnchantChecker.cpp
    HunspellChecker.cpp
    main.cpp
''')


src_extra_files = Split('''
    Makefile.am
    pch.h
''')


src_client_header_files = Split('''
    Messages.h
''')


src_client_files = Split('''
    boost.cpp
    client.cpp
    gettext.cpp
    Messages.cpp
''')


src_client_extra_files = Split('''
    lyxclient.1in
    Makefile.am
    pch.h
''')


src_support_header_files = Split('''
    bind.h
    convert.h
    copied_ptr.h
    debug.h
    docstream.h
    docstring.h
    docstring_list.h
    environment.h
    ExceptionMessage.h
    FileMonitor.h
    FileName.h
    filetools.h
    foreach.h
    ForkedCalls.h
    functional.h
    gettext.h
    gzstream.h
    lassert.h
    limited_stack.h
    lstrings.h
    lyxalgo.h
    lyxlib.h
    lyxtime.h
    mutex.h
    Messages.h
    numpunct_lyx_char_type.h
    os.h
    os_win32.h
    Package.h
    Path.h
    ProgressInterface.h
    qstring_helpers.h
    RandomAccessList.h
    regex.h
    shared_ptr.h
    socktools.h
    Systemcall.h
    SystemcallPrivate.h
    textutils.h
    Timeout.h
    Translator.h
    types.h
    unicode.h
    userinfo.h
    mythes/mythes.hxx
''')


src_support_files = Split('''
    convert.cpp
    debug.cpp
    docstream.cpp
    docstring.cpp
    environment.cpp
    FileMonitor.cpp
    FileName.cpp
    filetools.cpp
    ForkedCalls.cpp
    gettext.cpp
    gzstream.cpp
    kill.cpp
    lassert.cpp
    lstrings.cpp
    lyxtime.cpp
    mutex.cpp
    Messages.cpp
    os.cpp
    Package.cpp
    Path.cpp
    qstring_helpers.cpp
    socktools.cpp
    Systemcall.cpp
    Timeout.cpp
    unicode.cpp
    userinfo.cpp
    mythes/mythes.cxx
''')


src_support_extra_header_files = Split('''
    AppleSpeller.h
    AppleSpeller.m
''')


src_support_extra_src_files = Split('''
    atexit.c
    os_cygwin.cpp
    os_unix.cpp
    os_win32.cpp
    strerror.c
''')


src_support_extra_files = Split('''
    Makefile.am
    pch.h
    mythes/license.readme
''')


src_support_tests_extra_files = Split('''
    Makefile.am
    boost.cpp
    convert.cpp
    filetools.cpp
    lstrings.cpp
    pch.h
    test_convert
    test_filetools
    test_lstrings
''')


src_support_tests_regfiles_extra_files = Split('''
    convert
    filetools
    lstrings
''')


src_graphics_header_files = Split('''
    GraphicsCache.h
    GraphicsCacheItem.h
    GraphicsConverter.h
    GraphicsImage.h
    GraphicsLoader.h
    GraphicsParams.h
    GraphicsTypes.h
    PreviewImage.h
    PreviewLoader.h
''')


src_graphics_files = Split('''
    GraphicsCache.cpp
    GraphicsCacheItem.cpp
    GraphicsConverter.cpp
    GraphicsLoader.cpp
    GraphicsParams.cpp
    PreviewImage.cpp
    PreviewLoader.cpp
''')


src_graphics_extra_files = Split('''
    Makefile.am
    pch.h
''')


src_mathed_header_files = Split('''
    CommandInset.h
    InsetMath.h
    InsetMathAMSArray.h
    InsetMathArray.h
    InsetMathBig.h
    InsetMathBoldSymbol.h
    InsetMathBox.h
    InsetMathBrace.h
    InsetMathCancel.h
    InsetMathCancelto.h
    InsetMathCases.h
    InsetMathChar.h
    InsetMathColor.h
    InsetMathComment.h
    InsetMathDecoration.h
    InsetMathDelim.h
    InsetMathDiagram.h
    InsetMathDiff.h
    InsetMathDots.h
    InsetMathEnsureMath.h
    InsetMathEnv.h
    InsetMathExFunc.h
    InsetMathExInt.h
    InsetMathFont.h
    InsetMathFontOld.h
    InsetMathFrac.h
    InsetMathGrid.h
    InsetMathHull.h
    InsetMathKern.h
    InsetMathLefteqn.h
    InsetMathLim.h
    InsetMathMatrix.h
    InsetMathNest.h
    InsetMathNumber.h
    InsetMathOverset.h
    InsetMathPar.h
    InsetMathPhantom.h
    InsetMathRef.h
    InsetMathRoot.h
    InsetMathScript.h
    InsetMathSize.h
    InsetMathSpace.h
    InsetMathSpecialChar.h
    InsetMathSplit.h
    InsetMathSqrt.h
    InsetMathStackrel.h
    InsetMathString.h
    InsetMathSubstack.h
    InsetMathSymbol.h
    InsetMathTabular.h
    InsetMathUnderset.h
    InsetMathUnknown.h
    InsetMathXArrow.h
    InsetMathXYMatrix.h
    MacroTable.h
    MathAtom.h
    MathAutoCorrect.h
    MathCompletionList.h
    MathData.h
    MathExtern.h
    MathFactory.h
    MathGridInfo.h
    MathMacro.h
    MathMacroArgument.h
    MathMacroTemplate.h
    MathParser.h
    MathParser_flags.h
    MathStream.h
    MathSupport.h
    ReplaceData.h
    TextPainter.h
''')


src_mathed_files = Split('''
    CommandInset.cpp
    InsetMath.cpp
    InsetMathAMSArray.cpp
    InsetMathArray.cpp
    InsetMathBig.cpp
    InsetMathBoldSymbol.cpp
    InsetMathBox.cpp
    InsetMathBrace.cpp
    InsetMathCancel.cpp
    InsetMathCancelto.cpp
    InsetMathCases.cpp
    InsetMathChar.cpp
    InsetMathColor.cpp
    InsetMathComment.cpp
    InsetMathDecoration.cpp
    InsetMathDelim.cpp
    InsetMathDiagram.cpp
    InsetMathDiff.cpp
    InsetMathDots.cpp
    InsetMathEnsureMath.cpp
    InsetMathEnv.cpp
    InsetMathExFunc.cpp
    InsetMathExInt.cpp
    InsetMathFont.cpp
    InsetMathFontOld.cpp
    InsetMathFrac.cpp
    InsetMathGrid.cpp
    InsetMathHull.cpp
    InsetMathKern.cpp
    InsetMathLefteqn.cpp
    InsetMathLim.cpp
    InsetMathMatrix.cpp
    InsetMathNest.cpp
    InsetMathNumber.cpp
    InsetMathOverset.cpp
    InsetMathPar.cpp
    InsetMathPhantom.cpp
    InsetMathRef.cpp
    InsetMathRoot.cpp
    InsetMathScript.cpp
    InsetMathSize.cpp
    InsetMathSpace.cpp
    InsetMathSpecialChar.cpp
    InsetMathSplit.cpp
    InsetMathSqrt.cpp
    InsetMathStackrel.cpp
    InsetMathString.cpp
    InsetMathSubstack.cpp
    InsetMathSymbol.cpp
    InsetMathTabular.cpp
    InsetMathUnderset.cpp
    InsetMathUnknown.cpp
    InsetMathXArrow.cpp
    InsetMathXYMatrix.cpp
    MacroTable.cpp
    MathAtom.cpp
    MathAutoCorrect.cpp
    MathData.cpp
    MathExtern.cpp
    MathFactory.cpp
    MathMacro.cpp
    MathMacroArgument.cpp
    MathMacroTemplate.cpp
    MathParser.cpp
    MathStream.cpp
    MathSupport.cpp
    TextPainter.cpp
''')


src_mathed_extra_files = Split('''
    BUGS
    InsetFormulaMacro.cpp
    InsetFormulaMacro.h
    InsetMathMBox.cpp
    InsetMathMBox.h
    InsetMathXYArrow.cpp
    InsetMathXYArrow.h
    Makefile.am
    README
    pch.h
    texify
''')


src_tex2lyx_header_files = Split('''
    Context.h
    Parser.h
    tex2lyx.h
''')


src_tex2lyx_files = Split('''
    boost.cpp
    Context.cpp
    math.cpp
    Parser.cpp
    preamble.cpp
    table.cpp
    tex2lyx.cpp
    text.cpp
''')


src_tex2lyx_copied_header_files = Split('''
    TextClass.h
    graphics/GraphicsParams.h
    graphics/GraphicsTypes.h
    insets/ExternalTemplate.h
    insets/ExternalTransforms.h
    insets/InsetLayout.h
''')


src_tex2lyx_copied_files = Split('''
    Author.cpp
    Color.cpp
    Counters.cpp
    Encoding.cpp
    FloatList.cpp
    Floating.cpp
    FontInfo.cpp
    LaTeXPackages.cpp
    Layout.cpp
    LayoutFile.cpp
    LayoutModuleList.cpp
    Length.cpp
    lengthcommon.cpp
    Lexer.cpp
    ModuleList.cpp
    Spacing.cpp
    TextClass.cpp
    version.cpp
    graphics/GraphicsParams.cpp
    insets/ExternalTemplate.cpp
    insets/ExternalTransforms.cpp
    insets/InsetLayout.cpp
''')


src_tex2lyx_extra_files = Split('''
    Makefile.am
    TODO.txt
    pch.h
    tex2lyx.1in
    test/box-color-size-space-align.tex
    test/DummyDocument.tex
    test/foo.eps
    test/foo.png
    test/test-insets.tex
    test/test.ltx
    test/test-structure.tex
''')


src_frontends_header_files = Split('''
    alert.h
    Application.h
    Clipboard.h
    FontLoader.h
    FontMetrics.h
    KeyModifier.h
    KeySymbol.h
    mouse_state.h
    Painter.h
    Selection.h
    WorkArea.h
    WorkAreaManager.h
''')


src_frontends_files = Split('''
    WorkAreaManager.cpp
''')


src_frontends_extra_files = Split('''
    Makefile.am
    pch.h
''')


src_frontends_controllers_extra_files = Split('''
    Makefile.am
    pch.h
''')


src_frontends_controllers_tests_extra_files = Split('''
    Makefile.am
    biblio.cpp
    boost.cpp
    pch.h
    test_biblio
''')


src_frontends_controllers_tests_regfiles_extra_files = Split('''
    biblio
''')


src_frontends_qt4_header_files = Split('''
    ButtonPolicy.h
    Dialog.h
    Action.h
    BulletsModule.h
    ButtonController.h
    ColorCache.h
    CustomizedWidgets.h
    DialogView.h
    DockView.h
    EmptyTable.h
    FancyLineEdit.h
    FileDialog.h
    FindAndReplace.h
    FloatPlacement.h
    GuiAbout.h
    GuiApplication.h
    GuiBibitem.h
    GuiBibtex.h
    GuiBox.h
    GuiBranches.h
    GuiBranch.h
    GuiChanges.h
    GuiCharacter.h
    GuiCharacter.h
    GuiCitation.h
    GuiClipboard.h
    GuiCommandBuffer.h
    GuiCommandEdit.h
    GuiCompare.h
    GuiCompareHistory.h
    GuiCompleter.h
    GuiDelimiter.h
    GuiDialog.h
    GuiDocument.h
    GuiErrorList.h
    GuiERT.h
    GuiExternal.h
    GuiFontExample.h
    GuiFontLoader.h
    GuiGraphics.h
    GuiGraphicsUi.h
    GuiHSpace.h
    GuiHyperlink.h
    GuiIdListModel.h
    GuiImage.h
    GuiInclude.h
    GuiIndex.h
    GuiIndices.h
    GuiInfo.h
    GuiKeySymbol.h
    GuiLabel.h
    GuiLine.h
    GuiListings.h
    GuiLog.h
    GuiMathMatrix.h
    GuiNomenclature.h
    GuiNote.h
    GuiPainter.h
    GuiParagraph.h
    GuiPhantom.h
    GuiPrefs.h
    GuiPrint.h
    GuiPrintindex.h
    GuiPrintNomencl.h
    GuiProgress.h
    GuiProgressView.h
    GuiRef.h
    GuiSearch.h
    GuiSelection.h
    GuiSelectionManager.h
    GuiSendto.h
    GuiSetBorder.h
    GuiShowFile.h
    GuiSpellchecker.h
    GuiSymbols.h
    GuiTabularCreate.h
    GuiTabular.h
    GuiTexinfo.h
    GuiThesaurus.h
    GuiToc.h
    GuiToolbar.h
    GuiView.h
    GuiViewSource.h
    GuiVSpace.h
    GuiWorkArea.h
    GuiWrap.h
    IconPalette.h
    InGuiThread.h
    InsertTableWidget.h
    InsetParamsDialog.h
    InsetParamsWidget.h
    LaTeXHighlighter.h
    LayoutBox.h
    LengthCombo.h
    LyXFileDialog.h
    Menus.h
    PanelStack.h
    qt_helpers.h
    qt_i18n.h
    TocModel.h
    TocWidget.h
    Toolbars.h
    Validator.h
''')


src_frontends_qt4_files = Split('''
    ButtonPolicy.cpp
    Dialog.cpp
    Action.cpp
    BulletsModule.cpp
    ButtonController.cpp
    ColorCache.cpp
    CustomizedWidgets.cpp
    EmptyTable.cpp
    FancyLineEdit.cpp
    FileDialog.cpp
    FindAndReplace.cpp
    FloatPlacement.cpp
    GuiAbout.cpp
    GuiAlert.cpp
    GuiApplication.cpp
    GuiBibitem.cpp
    GuiBibtex.cpp
    GuiBox.cpp
    GuiBranch.cpp
    GuiBranches.cpp
    GuiChanges.cpp
    GuiCharacter.cpp
    GuiCitation.cpp
    GuiClipboard.cpp
    GuiCommandBuffer.cpp
    GuiCommandEdit.cpp
    GuiCompare.cpp
    GuiCompareHistory.cpp
    GuiCompleter.cpp
    GuiDelimiter.cpp
    GuiDialog.cpp
    GuiDocument.cpp
    GuiErrorList.cpp
    GuiERT.cpp
    GuiExternal.cpp
    GuiFontExample.cpp
    GuiFontLoader.cpp
    GuiFontMetrics.cpp
    GuiGraphics.cpp
    GuiHSpace.cpp
    GuiHyperlink.cpp
    GuiIdListModel.cpp 
    GuiImage.cpp
    GuiInclude.cpp
    GuiIndex.cpp
    GuiIndices.cpp
    GuiInfo.cpp
    GuiKeySymbol.cpp
    GuiLabel.cpp
    GuiLine.cpp
    GuiListings.cpp
    GuiLog.cpp
    GuiMathMatrix.cpp
    GuiNomenclature.cpp
    GuiNote.cpp
    GuiPainter.cpp
    GuiParagraph.cpp
    GuiPhantom.cpp
    GuiPrefs.cpp
    GuiPrint.cpp
    GuiPrintindex.cpp
    GuiPrintNomencl.cpp
    GuiProgress.cpp
    GuiProgressView.cpp
    GuiRef.cpp
    GuiSearch.cpp
    GuiSelection.cpp
    GuiSelectionManager.cpp
    GuiSendto.cpp
    GuiSetBorder.cpp
    GuiShowFile.cpp
    GuiSpellchecker.cpp
    GuiSymbols.cpp
    GuiTabular.cpp
    GuiTabularCreate.cpp
    GuiTexinfo.cpp
    GuiThesaurus.cpp
    GuiToc.cpp
    GuiToolbar.cpp
    GuiView.cpp
    GuiViewSource.cpp
    GuiVSpace.cpp
    GuiWorkArea.cpp
    GuiWrap.cpp
    IconPalette.cpp
    InGuiThread.cpp
    InsertTableWidget.cpp
    InsetParamsDialog.cpp
    InsetParamsWidget.cpp
    LengthCombo.cpp
    LaTeXHighlighter.cpp
    LayoutBox.cpp
    LyXFileDialog.cpp
    Menus.cpp
    PanelStack.cpp
    qt_helpers.cpp
    TocModel.cpp
    TocWidget.cpp
    Toolbars.cpp
    Validator.cpp
''')


src_frontends_qt4_extra_files = Split('''
    GuiFontMetrics.h
    Makefile.am
    pch.h
    README    
''')


src_frontends_qt4_ui_files = Split('''
    AboutUi.ui
    BibitemUi.ui
    BiblioUi.ui
    BibtexAddUi.ui
    BibtexUi.ui
    BoxUi.ui
    BranchUi.ui
    BranchesUi.ui
    BranchesUnknownUi.ui
    BulletsUi.ui
    ChangesUi.ui
    CharacterUi.ui
    CitationUi.ui
    ColorUi.ui
    CompareUi.ui
    CompareHistoryUi.ui
    DelimiterUi.ui
    DocumentUi.ui
    ERTUi.ui
    ErrorListUi.ui
    ExternalUi.ui
    FindAndReplaceUi.ui
    FloatPlacementUi.ui
    FontUi.ui
    GraphicsUi.ui
    HSpaceUi.ui
    HyperlinkUi.ui
    IncludeUi.ui
    IndexUi.ui
    IndicesUi.ui
    InfoUi.ui
    InsetParamsUi.ui
    LabelUi.ui
    LaTeXUi.ui
    LanguageUi.ui
    LineUi.ui
    ListingsUi.ui
    ListingsSettingsUi.ui
    LocalLayoutUi.ui
    LogUi.ui
    MarginsUi.ui
    MasterChildUi.ui
    MathMatrixUi.ui
    MathsUi.ui
    ModulesUi.ui
    NomenclUi.ui
    NoteUi.ui
    NumberingUi.ui
    OutputUi.ui
    PageLayoutUi.ui
    ParagraphUi.ui
    PDFSupportUi.ui
    PhantomUi.ui
    PreambleUi.ui
    PrefColorsUi.ui
    PrefCompletionUi.ui
    PrefConvertersUi.ui
    PrefDisplayUi.ui
    PrefEditUi.ui
    PrefFileformatsUi.ui
    PrefIdentityUi.ui
    PrefInputUi.ui
    PrefLanguageUi.ui
    PrefLatexUi.ui
    PrefOutputUi.ui
    PrefPathsUi.ui
    PrefPrinterUi.ui
    PrefScreenFontsUi.ui
    PrefShortcutsUi.ui
    PrefSpellcheckerUi.ui
    PrefUi.ui
    PrefsUi.ui
    PrintUi.ui
    PrintindexUi.ui
    PrintNomenclUi.ui
    ProgressViewUi.ui
    RefUi.ui
    SearchUi.ui
    SendtoUi.ui
    ShortcutUi.ui
    ShowFileUi.ui
    SpellcheckerUi.ui
    SymbolsUi.ui
    TabularCreateUi.ui
    TabularUi.ui
    TexinfoUi.ui
    TextLayoutUi.ui
    ThesaurusUi.ui
    TocUi.ui
    ToggleWarningUi.ui
    VSpaceUi.ui
    ViewSourceUi.ui
    WrapUi.ui
''')


src_frontends_qt4_ui_extra_files = Split('''
    Makefile.am    
''')


src_insets_header_files = Split('''
    ExternalSupport.h
    ExternalTemplate.h
    ExternalTransforms.h
    Inset.h
    InsetArgument.h
    InsetBibitem.h
    InsetBibtex.h
    InsetBox.h
    InsetBranch.h
    InsetCaption.h
    InsetCitation.h
    InsetCode.h
    InsetCollapsable.h
    InsetCommand.h
    InsetCommandParams.h
    InsetERT.h
    InsetExternal.h
    InsetFlex.h
    InsetFloat.h
    InsetFloatList.h
    InsetFoot.h
    InsetFootlike.h
    InsetGraphics.h
    InsetGraphicsParams.h
    InsetHyperlink.h
    InsetInclude.h
    InsetIndex.h
    InsetInfo.h
    InsetLabel.h
    InsetLayout.h
    InsetLine.h
    InsetListings.h
    InsetListingsParams.h
    InsetMarginal.h
    InsetNewline.h
    InsetNewpage.h
    InsetNomencl.h
    InsetNote.h
    InsetPhantom.h
    InsetPreview.h
    InsetQuotes.h
    InsetRef.h
    InsetScript.h
    InsetSpace.h
    InsetSpecialChar.h
    InsetTOC.h
    InsetTabular.h
    InsetText.h
    InsetVSpace.h
    InsetWrap.h
    RenderBase.h
    RenderButton.h
    RenderGraphic.h
    RenderPreview.h
''')


src_insets_files = Split('''
    ExternalSupport.cpp
    ExternalTemplate.cpp
    ExternalTransforms.cpp
    Inset.cpp
    InsetArgument.cpp
    InsetBibitem.cpp
    InsetBibtex.cpp
    InsetBox.cpp
    InsetBranch.cpp
    InsetCaption.cpp
    InsetCitation.cpp
    InsetCollapsable.cpp
    InsetCommand.cpp
    InsetCommandParams.cpp
    InsetERT.cpp
    InsetExternal.cpp
    InsetFlex.cpp
    InsetFloat.cpp
    InsetFloatList.cpp
    InsetFoot.cpp
    InsetFootlike.cpp
    InsetGraphics.cpp
    InsetGraphicsParams.cpp
    InsetHyperlink.cpp
    InsetInclude.cpp
    InsetIndex.cpp
    InsetInfo.cpp
    InsetLabel.cpp
    InsetLayout.cpp
    InsetLine.cpp
    InsetListings.cpp
    InsetListingsParams.cpp
    InsetMarginal.cpp
    InsetNewline.cpp
    InsetNewpage.cpp
    InsetNomencl.cpp
    InsetNote.cpp
    InsetPhantom.cpp
    InsetPreview.cpp
    InsetQuotes.cpp
    InsetRef.cpp
    InsetScript.cpp
    InsetSpace.cpp
    InsetSpecialChar.cpp
    InsetTOC.cpp
    InsetTabular.cpp
    InsetText.cpp
    InsetVSpace.cpp
    InsetWrap.cpp
    RenderButton.cpp
    RenderGraphic.cpp
    RenderPreview.cpp
''')


src_insets_extra_files = Split('''
    Makefile.am
    pch.h
''')


intl_header_files = Split('''
    eval-plural.h
    gettextP.h
    gmo.h
    hash-string.h
    loadinfo.h
    localcharset.h
    os2compat.h
    plural-exp.h
    printf-args.h
    printf-parse.h
    relocatable.h
    vasnprintf.h
    vasnwprintf.h
    wprintf-parse.h
    xsize.h
''')


intl_files = Split('''
    bindtextdom.c
    dcgettext.c
    dcigettext.c
    dcngettext.c
    dgettext.c
    dngettext.c
    explodename.c
    finddomain.c
    gettext.c
    intl-compat.c
    l10nflist.c
    langprefs.c
    loadmsgcat.c
    localcharset.c
    localealias.c
    localename.c
    log.c
    ngettext.c
    osdep.c
    plural-exp.c
    plural.c
    printf.c
    relocatable.c
    textdomain.c
''')


intl_extra_files = Split('''
    config.charset
    libgnuintl.h.in
    locale.alias
    os2compat.c
    plural.y
    printf-args.c
    printf-parse.c
    ref-add.sin
    ref-del.sin
    vasnprintf.c
    VERSION
''')


config_extra_files = Split('''
    common.am
    config.guess
    config.rpath
    config.sub
    depcomp
    install-sh
    ltmain.sh
    lyxinclude.m4
    Makefile.am
    missing
    mkinstalldirs
    pkg.m4
    py-compile
    qt4.m4
    spell.m4
''')


sourcedoc_extra_files = Split('''
    Doxyfile.in
    Makefile.am
''')


po_extra_files = Split('''
    ar.po
    bg.po
    boldquot.sed
    ca.po
    cs.po
    da.po
    de.po
    en@boldquot.header
    en@quot.header
    el.po
    en.po
    es.po
    eu.po
    fi.po
    fr.po
    gl.po
    he.po
    hu.po
    insert-header.sin
    id.po
    it.po
    ja.po
    ko.po
    LINGUAS
    lyx_pot.py
    Makefile.in.in
    Makevars
    nb.po
    nl.po
    nn.po
    pl.po
    pocheck.pl
    POTFILES.in
    postats.sh
    pt.po
    quot.sed
    README
    remove-potcdate.sin
    ro.po
    ru.po
    Rules-quot
    sk.po
    sl.po
    sr.po
    sv.po
    tr.po
    uk.po
    wa.po
    zh_CN.po
    zh_TW.po
''')


lib_files = Split('''
    autocorrect
    chkconfig.ltx
    configure.py
    CREDITS
    encodings
    external_templates
    languages
    layouttranslations
    symbols
    syntax.default
    unicodesymbols
''')


lib_extra_files = Split('''
    autocorrect
    build-listerrors
    generate_contributions.py
    Makefile.am
''')


lib_kbd_files = Split('''
    american-2.kmap
    american.kmap
    arabic.kmap
    bg-bds-1251.kmap
    brazil.kmap
    brazil2.kmap
    czech-prg.kmap
    czech.kmap
    espanol.kmap
    european.kmap
    farsi.kmap
    francais.kmap
    french.kmap
    german-2.kmap
    german-3.kmap
    german.kmap
    greek.kmap
    hebrew.kmap
    koi8-r.kmap
    koi8-u.kmap
    latvian.kmap
    magyar-2.kmap
    magyar-3.kmap
    magyar.kmap
    null.kmap
    polish.kmap
    polski.kmap
    portuges.kmap
    romanian.kmap
    serbian.kmap
    serbocroatian.kmap
    sf.kmap
    sg.kmap
    slovak.kmap
    slovene.kmap
    thai-kedmanee.kmap
    transilvanian.kmap
    turkish-f.kmap
    turkish.kmap
''')


lib_templates_files = Split('''
    aa.lyx
    aastex.lyx
    ACM-siggraph.lyx
    ACM-sigplan.lyx
    AEA.lyx
    AGUTeX.lyx
    AGU_article.lyx
    APA.lyx
    beamer-conference-ornate-20min.lyx
    ctex.lyx
    de_beamer-conference-ornate-20min.lyx
    dinbrief.lyx
    DocBook_article.lyx
    ectaart.lyx
    elsarticle.lyx
    es_beamer-conference-ornate-20min.lyx
    fr_beamer-conference-ornate-20min.lyx
    frletter.lyx
    g-brief-de.lyx
    g-brief-en.lyx
    g-brief2.lyx
    hollywood.lyx
    IEEEtran.lyx
    IJMPC.lyx
    IJMPD.lyx
    IOP-article.lyx
    JASA.lyx
    JSS-article.lyx
    kluwer.lyx
    koma-letter2.lyx
    letter.lyx
    lettre.lyx
    README.new_templates
    revtex4.lyx
    revtex4-1.lyx
    slides.lyx
''')


lib_templates_springer_files = Split('''
    svjour3.lyx
    svmono_acknow.lyx
    svmono_acronym.lyx
    svmono_appendix.lyx
    svmono_book-master.lyx
    svmono_chapter.lyx
    svmono_dedication.lyx
    svmono_foreword.lyx
    svmono_glossary.lyx
    svmono_part.lyx
    svmono_preface.lyx
    svmono_referenc.lyx
    svmono_solutions.lyx
    svmult_acknow.lyx
    svmult_acronym.lyx
    svmult_appendix.lyx
    svmult_author.lyx
    svmult_cblist.lyx
    svmult_dedication.lyx
    svmult_editor-master.lyx
    svmult_foreword.lyx
    svmult_glossary.lyx
    svmult_part.lyx
    svmult_preface.lyx
    svmult_referenc.lyx
''')


lib_templates_thesis_files = Split('''
    Acknowledgments.lyx
    Appendix.lyx
    alpha.bst
    chapter-1.lyx
    chapter-2.lyx
    Summary.lyx
    thesis.lyx
    thesisExample.bib
''')


lib_ui_files = Split('''
    default.ui
    stdcontext.inc
    stdmenus.inc
    stdtoolbars.inc
''')


lib_fonts_files = Split('''
    BaKoMaFontLicense.txt
    cmex10.ttf
    cmmi10.ttf
    cmr10.ttf
    cmsy10.ttf
    esint10.ttf
    eufm10.ttf
    msam10.ttf
    msbm10.ttf
    ReadmeBaKoMa4LyX.txt
    rsfs10.ttf
    wasy10.ttf
''')


lib_images_files = Split('''
    all-changes-accept.png
    all-changes-reject.png
    amssymb.png
    banner.png
    bookmark-goto.png
    bookmark-goto_0.png
    bookmark-save.png
    box-insert.png
    break-line.png
    buffer-close.png
    buffer-export.png
    buffer-export_dvi.png
    buffer-export_dvi3.png
    buffer-export_latex.png
    buffer-export_pdf.png
    buffer-export_pdf2.png
    buffer-export_pdf3.png
    buffer-export_pdf4.png
    buffer-export_pdf5.png
    buffer-export_ps.png
    buffer-export_text.png
    buffer-new.png
    buffer-reload.png
    buffer-toggle-output-sync.png
    buffer-update.png
    buffer-update_dvi.png
    buffer-update_dvi3.png
    buffer-update_pdf.png
    buffer-update_pdf2.png
    buffer-update_pdf3.png
    buffer-update_pdf4.png
    buffer-update_pdf5.png
    buffer-update_ps.png
    buffer-view.png
    buffer-view_dvi.png
    buffer-view_dvi3.png
    buffer-view_pdf.png
    buffer-view_pdf2.png
    buffer-view_pdf3.png
    buffer-view_pdf4.png
    buffer-view_pdf5.png
    buffer-view_ps.png
    buffer-write.png
    buffer-write-as.png
    build-program.png
    busy.gif
    change-accept.png
    change-next.png
    change-reject.png
    changes-merge.png
    changes-output.png
    changes-track.png
    closetab.png
    close-tab-group.png
    copy.png
    cut.png
    demote.png
    depth-decrement.png
    depth-increment.png
    dialog-preferences.png
    dialog-show-new-inset_citation.png
    dialog-show-new-inset_graphics.png
    dialog-show-new-inset_include.png
    dialog-show-new-inset_ref.png
    dialog-show_character.png
    dialog-show_findreplace.png
    dialog-show_mathdelimiter.png
    dialog-show_mathmatrix.png
    dialog-show_print.png
    dialog-show_spellchecker.png
    dialog-show_vclog.png
    dialog-toggle_findreplaceadv.png
    dialog-toggle_toc.png
    down.png
    editclear.png
    ert-insert.png
    file-open.png
    float-insert_figure.png
    float-insert_table.png
    font-bold.png
    font-emph.png
    font-noun.png
    font-sans.png
    footnote-insert.png
    href-insert.png
    hidetab.png
    index-insert.png
    info-insert_buffer_vcs-revision.png
    label-insert.png
    layout-document.png
    layout-paragraph.png
    layout.png
    layout_Description.png
    layout_Enumerate.png
    layout_Itemize.png
    layout_List.png
    layout_LyX-Code.png
    layout_Scrap.png
    layout_Section.png
    lyx-quit.png
    lyx.png
    marginalnote-insert.png
    master-buffer-update.png
    master-buffer-view.png
    math-display.png
    math-macro-add-greedy-optional-param.png
    math-macro-add-optional-param.png
    math-macro-add-param.png
    math-macro-append-greedy-param.png
    math-macro-make-nonoptional.png
    math-macro-make-optional.png
    math-macro-remove-greedy-param.png
    math-macro-remove-optional-param.png
    math-macro-remove-param.png
    math-macro_newmacroname_newcommand.png
    math-matrix.png
    math-mode.png
    math-subscript.png
    math-superscript.png
    nomencl-insert.png
    note-insert.png
    note-next.png
    paste.png
    promote.png
    psnfss1.png
    psnfss2.png
    psnfss3.png
    psnfss4.png
    redo.png
    reload.png
    script-insert_subscript.png
    script-insert_superscript.png
    split-view_horizontal.png
    split-view_vertical.png
    standard.png
    tabular-feature_align-decimal.png
    tabular-feature_append-column.png
    tabular-feature_append-row.png
    tabular-feature_delete-column.png
    tabular-feature_delete-row.png
    tabular-feature_m-align-center.png
    tabular-feature_m-align-left.png
    tabular-feature_m-align-right.png
    tabular-feature_m-valign-bottom.png
    tabular-feature_m-valign-middle.png
    tabular-feature_m-valign-top.png
    tabular-feature_multicolumn.png
    tabular-feature_multirow.png
    tabular-feature_set-all-lines.png
    tabular-feature_set-border-lines.png
    tabular-feature_set-longtabular.png
    tabular-feature_set-rotate-cell.png
    tabular-feature_set-rotate-tabular.png
    tabular-feature_toggle-line-bottom.png
    tabular-feature_toggle-line-left.png
    tabular-feature_toggle-line-right.png
    tabular-feature_toggle-line-top.png
    tabular-feature_toggle-rotate-cell.png
    tabular-feature_toggle-rotate-tabular.png
    tabular-feature_unset-all-lines.png
    tabular-insert.png
    textstyle-apply.png
    thesaurus-entry.png
    toolbar-toggle_math.png
    toolbar-toggle_math_panels.png
    toolbar-toggle_table.png
    undo.png
    unknown.png
    up.png
    update-others.png
    url-insert.png
    vc-compare.png
    vc-compare_0.png
    vc-check-in.png
    vc-check-out.png
    vc-locking-toggle.png
    vc-register.png
    vc-repo-update.png
    vc-revert.png
    view-others.png
''')


lib_images_extra_files = Split('''
    font-smallcaps.png
    README
''')


lib_images_math_files = Split('''
    acute.png
    adots.png
    aleph.png
    alpha.png
    amalg.png
    angle.png
    approx.png
    approxeq.png
    asymp.png
    backepsilon.png
    backprime.png
    backsim.png
    backsimeq.png
    backslash.png
    bar.png
    bars.png
    barwedge.png
    Bbbk.png
    because.png
    beta.png
    beth.png
    between.png
    bigcap.png
    bigcirc.png
    bigcup.png
    bigodot.png
    bigoplus.png
    bigotimes.png
    bigsqcup.png
    bigstar.png
    bigtriangledown.png
    bigtriangleup.png
    biguplus.png
    bigvee.png
    bigwedge.png
    blacklozenge.png
    blacksquare.png
    blacktriangle.png
    blacktriangledown.png
    blacktriangleleft.png
    blacktriangleright.png
    bot.png
    bowtie.png
    boxdot.png
    boxminus.png
    boxplus.png
    boxtimes.png
    breve.png
    bullet.png
    bumpeq.png
    bumpeq2.png
    cap.png
    cap2.png
    cases.png
    cdot.png
    cdots.png
    centerdot.png
    check.png
    chi.png
    circ.png
    circeq.png
    circlearrowleft.png
    circlearrowright.png
    circledS.png
    circledast.png
    circledcirc.png
    circleddash.png
    clubsuit.png
    complement.png
    cong.png
    coprod.png
    cup.png
    cup2.png
    curlyeqprec.png
    curlyeqsucc.png
    curlyvee.png
    curlywedge.png
    curvearrowleft.png
    curvearrowright.png
    dagger.png
    daleth.png
    dashleftarrow.png
    dashrightarrow.png
    dashv.png
    ddagger.png
    ddddot.png
    dddot.png
    ddot.png
    ddots.png
    delim.png
    delta.png
    delta2.png
    diagdown.png
    diagup.png
    diamond.png
    diamond2.png
    diamondsuit.png
    digamma.png
    div.png
    divideontimes.png
    dot.png
    doteq.png
    doteqdot.png
    dotplus.png
    dotsint.png
    dotsintop.png
    doublebarwedge.png
    downarrow.png
    downarrow2.png
    downdownarrows.png
    downharpoonleft.png
    downharpoonright.png
    ell.png
    empty.png
    emptyset.png
    epsilon.png
    eqcirc.png
    eqslantgtr.png
    eqslantless.png
    equation.png
    equiv.png
    eta.png
    eth.png
    exists.png
    export-others.png
    fallingdotseq.png
    fint.png
    fintop.png
    Finv.png
    flat.png
    font.png
    forall.png
    frac-square.png
    frac.png
    frown.png
    functions.png
    Game.png
    gamma.png
    gamma2.png
    geq.png
    geqq.png
    geqslant.png
    gg.png
    ggg.png
    gimel.png
    gnapprox.png
    gneq.png
    gneqq.png
    gnsim.png
    grave.png
    gtrapprox.png
    gtrdot.png
    gtreqless.png
    gtreqqless.png
    gtrless.png
    gtrsim.png
    gvertneqq.png
    hat.png
    hbar.png
    heartsuit.png
    hookleftarrow.png
    hookrightarrow.png
    hphantom.png
    hslash.png
    iddots.png
    iiiint.png
    iiiintop.png
    iiint.png
    iiintop.png
    iint.png
    iintop.png
    Im.png
    imath.png
    in.png
    infty.png
    int.png
    intercal.png
    intop.png
    iota.png
    jmath.png
    kappa.png
    lambda.png
    lambda2.png
    landdownint.png
    landdownintop.png
    landupint.png
    landupintop.png
    langle.png
    lbrace.png
    lbrace_rbrace.png
    lbracket.png
    lbracket_rbracket.png
    lceil.png
    lceil_rceil.png
    ldots.png
    leftarrow.png
    leftarrow2.png
    leftarrowtail.png
    leftharpoondown.png
    leftharpoonup.png
    leftleftarrows.png
    leftrightarrow.png
    leftrightarrow2.png
    leftrightarrows.png
    leftrightharpoons.png
    leftrightsquigarrow.png
    leftthreetimes.png
    leq.png
    leqq.png
    leqslant.png
    lessapprox.png
    lessdot.png
    lesseqgtr.png
    lesseqqgtr.png
    lessgtr.png
    lesssim.png
    lfloor.png
    lfloor_rfloor.png
    ll.png
    llcorner.png
    Lleftarrow.png
    lll.png
    lnapprox.png
    lneq.png
    lneqq.png
    lnsim.png
    longleftarrow.png
    longleftarrow2.png
    longleftrightarrow.png
    longleftrightarrow2.png
    longmapsto.png
    longrightarrow.png
    longrightarrow2.png
    looparrowleft.png
    looparrowright.png
    lozenge.png
    lparen.png
    lparen_rparen.png
    lrcorner.png
    Lsh.png
    ltimes.png
    lvertneqq.png
    mapsto.png
    mathbb_C.png
    mathbb_H.png
    mathbb_N.png
    mathbb_Q.png
    mathbb_R.png
    mathbb_Z.png
    mathcal_F.png
    mathcal_H.png
    mathcal_L.png
    mathcal_O.png
    mathcircumflex.png
    mathrm_T.png
    matrix.png
    measuredangle.png
    mho.png
    mid.png
    models.png
    mp.png
    mu.png
    multimap.png
    nabla.png
    natural.png
    ncong.png
    nearrow.png
    neg.png
    neq.png
    nexists.png
    ngeq.png
    ngeqq.png
    ngeqslant.png
    ngtr.png
    ni.png
    nleftarrow.png
    nleftarrow2.png
    nleftrightarrow.png
    nleftrightarrow2.png
    nleq.png
    nleqq.png
    nleqslant.png
    nless.png
    nmid.png
    notin.png
    nparallel.png
    nprec.png
    npreceq.png
    nrightarrow.png
    nrightarrow2.png
    nshortmid.png
    nshortparallel.png
    nsim.png
    nsubseteq.png
    nsucc.png
    nsucceq.png
    nsupseteq.png
    nsupseteqq.png
    ntriangleleft.png
    ntrianglelefteq.png
    ntriangleright.png
    ntrianglerighteq.png
    nu.png
    nvdash.png
    nvdash2.png
    nvdash3.png
    nwarrow.png
    odot.png
    oiint.png
    oiintop.png
    oint.png
    ointclockwise.png
    ointclockwiseop.png
    ointctrclockwise.png
    ointctrclockwiseop.png
    ointop.png
    omega.png
    omega2.png
    ominus.png
    oplus.png
    oslash.png
    otimes.png
    overbrace.png
    overleftarrow.png
    overleftrightarrow.png
    overline.png
    overrightarrow.png
    overset.png
    parallel.png
    partial.png
    perp.png
    phantom.png
    phi.png
    phi2.png
    pi.png
    pi2.png
    pitchfork.png
    pm.png
    prec.png
    precapprox.png
    preccurlyeq.png
    preceq.png
    precnapprox.png
    precnsim.png
    precsim.png
    prime.png
    prod.png
    propto.png
    psi.png
    psi2.png
    rangle.png
    rbrace.png
    rbracket.png
    rceil.png
    Re.png
    rfloor.png
    rho.png
    rightarrow.png
    rightarrow2.png
    rightarrowtail.png
    rightharpoondown.png
    rightharpoonup.png
    rightleftarrows.png
    rightleftharpoons.png
    rightrightarrows.png
    rightsquigarrow.png
    rightthreetimes.png
    risingdotseq.png
    root.png
    rparen.png
    Rrightarrow.png
    Rsh.png
    rtimes.png
    searrow.png
    setminus.png
    sharp.png
    shortmid.png
    shortparallel.png
    sigma.png
    sigma2.png
    sim.png
    simeq.png
    slash.png
    smallfrown.png
    smallsetminus.png
    smallsmile.png
    smile.png
    space.png
    spadesuit.png
    sphericalangle.png
    sqcap.png
    sqcup.png
    sqiint.png
    sqiintop.png
    sqint.png
    sqintop.png
    sqrt-square.png
    sqrt.png
    sqsubset.png
    sqsubseteq.png
    sqsupset.png
    sqsupseteq.png
    square.png
    star.png
    style.png
    sub.png
    subset.png
    subset2.png
    subseteq.png
    subseteqq.png
    subsetneq.png
    subsetneqq.png
    succ.png
    succapprox.png
    succcurlyeq.png
    succeq.png
    succnapprox.png
    succnsim.png
    succsim.png
    sum.png
    super.png
    supset.png
    supset2.png
    supseteq.png
    supseteqq.png
    supsetneq.png
    supsetneqq.png
    surd.png
    swarrow.png
    tau.png
    textrm_AA.png
    textrm_O.png
    therefore.png
    theta.png
    theta2.png
    thickapprox.png
    thicksim.png
    tilde.png
    times.png
    top.png
    triangle.png
    triangledown.png
    triangleleft.png
    trianglelefteq.png
    triangleq.png
    triangleright.png
    trianglerighteq.png
    twoheadleftarrow.png
    twoheadrightarrow.png
    ulcorner.png
    underbrace.png
    underleftarrow.png
    underleftrightarrow.png
    underline.png
    underrightarrow.png
    underscore.png
    underset.png
    undertilde.png
    uparrow.png
    uparrow2.png
    updownarrow.png
    updownarrow2.png
    upharpoonleft.png
    upharpoonright.png
    uplus.png
    upsilon.png
    upsilon2.png
    upuparrows.png
    urcorner.png
    utilde.png
    varepsilon.png
    varkappa.png
    varnothing.png
    varphi.png
    varpi.png
    varpropto.png
    varrho.png
    varsigma.png
    varsubsetneq.png
    varsubsetneqq.png
    varsupsetneq.png
    varsupsetneqq.png
    vartheta.png
    vartriangle.png
    vartriangleleft.png
    vartriangleright.png
    vdash.png
    vdash2.png
    vdash3.png
    vdots.png
    vec.png
    vee.png
    veebar.png
    vert.png
    vert2.png
    vphantom.png
    Vvdash.png
    wedge.png
    widehat.png
    widetilde.png
    wp.png
    wr.png
    xi.png
    xi2.png
    zeta.png
''')


lib_images_math_extra_files = Split('''
    ams_arrows.png
    ams_misc.png
    ams_nrel.png
    ams_ops.png
    ams_rel.png
    arrows.png
    bop.png
    brel.png
    deco.png
    deco.png
    delim.png
    delim0.png
    delim1.png
    dots.png
    font.png
    functions.png
    greek.png
    misc.png
    varsz.png
''')


lib_images_commands_files = Split('''
''')


lib_images_classic_files = Split('''
    all-changes-accept.png
    all-changes-reject.png
    bookmark-goto.png
    bookmark-goto_0.png
    bookmark-save.png
    box-insert.png
    break-line.png
    buffer-close.png
    buffer-export.png
    buffer-export_dvi.png
    buffer-export_dvi3.png
    buffer-export_latex.png
    buffer-export_pdf.png
    buffer-export_pdf2.png
    buffer-export_pdf3.png
    buffer-export_pdf4.png
    buffer-export_pdf5.png
    buffer-export_ps.png
    buffer-export_text.png
    buffer-new.png
    buffer-reload.png
    buffer-toggle-output-sync.png
    buffer-update.png
    buffer-update_dvi.png
    buffer-update_dvi3.png
    buffer-update_pdf.png
    buffer-update_pdf2.png
    buffer-update_pdf3.png
    buffer-update_pdf4.png
    buffer-update_pdf5.png
    buffer-update_ps.png
    buffer-view.png
    buffer-view_dvi.png
    buffer-view_dvi3.png
    buffer-view_pdf.png
    buffer-view_pdf2.png
    buffer-view_pdf3.png
    buffer-view_pdf4.png
    buffer-view_pdf5.png
    buffer-view_ps.png
    buffer-write.png
    buffer-write-as.png
    build-program.png
    change-accept.png
    change-next.png
    change-reject.png
    changes-merge.png
    changes-output.png
    changes-track.png
    close-tab-group.png
    copy.png
    cut.png
    demote.png
    depth-decrement.png
    depth-increment.png
    dialog-preferences.png
    dialog-show_character.png
    dialog-show_findreplace.png
    dialog-show_mathdelimiter.png
    dialog-show_mathmatrix.png
    dialog-show_print.png
    dialog-show_spellchecker.png
    dialog-show_vclog.png
    dialog-show-new-inset_citation.png
    dialog-show-new-inset_graphics.png
    dialog-show-new-inset_include.png
    dialog-show-new-inset_ref.png
    dialog-toggle_findreplaceadv.png
    dialog-toggle_toc.png
    down.png
    ert-insert.png
    file-open.png
    float-insert_figure.png
    float-insert_table.png
    font-bold.png
    font-emph.png
    font-noun.png
    font-sans.png
    footnote-insert.png
    href-insert.png
    iconsize.png
    index-insert.png
    info-insert_buffer_vcs-revision.png
    label-insert.png
    layout-document.png
    layout-paragraph.png
    layout.png
    layout_Description.png
    layout_Enumerate.png
    layout_Itemize.png
    layout_List.png
    layout_LyX-Code.png
    layout_Scrap.png
    layout_Section.png
    marginalnote-insert.png
    master-buffer-update.png
    master-buffer-view.png
    math-display.png
    math-macro-add-greedy-optional-param.png
    math-macro-add-optional-param.png
    math-macro-add-param.png
    math-macro-append-greedy-param.png
    math-macro-make-nonoptional.png
    math-macro-make-optional.png
    math-macro-remove-greedy-param.png
    math-macro-remove-optional-param.png
    math-macro-remove-param.png
    math-macro_newmacroname_newcommand.png
    math-matrix.png
    math-mode.png
    math-subscript.png
    math-superscript.png
    nomencl-insert.png
    note-insert.png
    note-next.png
    paste.png
    promote.png
    redo.png
    reload.png
    script-insert_subscript.png
    script-insert_superscript.png
    split-view_horizontal.png
    split-view_vertical.png
    tabular-feature_align-decimal.png
    tabular-feature_append-column.png
    tabular-feature_append-row.png
    tabular-feature_delete-column.png
    tabular-feature_delete-row.png
    tabular-feature_m-align-center.png
    tabular-feature_m-align-left.png
    tabular-feature_m-align-right.png
    tabular-feature_m-valign-bottom.png
    tabular-feature_m-valign-middle.png
    tabular-feature_m-valign-top.png
    tabular-feature_multicolumn.png
    tabular-feature_multirow.png
    tabular-feature_set-all-lines.png
    tabular-feature_set-border-lines.png
    tabular-feature_set-longtabular.png
    tabular-feature_set-rotate-cell.png
    tabular-feature_set-rotate-tabular.png
    tabular-feature_toggle-line-bottom.png
    tabular-feature_toggle-line-left.png
    tabular-feature_toggle-line-right.png
    tabular-feature_toggle-line-top.png
    tabular-feature_toggle-rotate-cell.png
    tabular-feature_toggle-rotate-tabular.png
    tabular-feature_unset-all-lines.png
    tabular-insert.png
    textstyle-apply.png
    thesaurus-entry.png
    toolbar-toggle_math.png
    toolbar-toggle_math_panels.png
    toolbar-toggle_table.png
    undo.png
    unknown.png
    up.png
    update-others.png
    url-insert.png
    vc-compare.png
    vc-compare_0.png
    vc-check-in.png
    vc-check-out.png
    vc-locking-toggle.png
    vc-register.png
    vc-repo-update.png
    vc-revert.png
    view-others.png
''')


lib_images_oxygen_files = Split('''
    all-changes-accept.png
    all-changes-reject.png
    bookmark-goto.png
    bookmark-goto_0.png
    bookmark-save.png
    box-insert.png
    buffer-new.png
    buffer-reload.png
    buffer-toggle-output-sync.png
    buffer-update.png
    buffer-update_dvi.png
    buffer-update_dvi3.png
    buffer-update_pdf.png
    buffer-update_pdf2.png
    buffer-update_pdf3.png
    buffer-update_pdf4.png
    buffer-update_pdf5.png
    buffer-update_ps.png
    buffer-view.png
    buffer-view_dvi.png
    buffer-view_dvi3.png
    buffer-view_pdf.png
    buffer-view_pdf2.png
    buffer-view_pdf3.png
    buffer-view_pdf4.png
    buffer-view_pdf5.png
    buffer-view_ps.png
    buffer-write.png
    buffer-write-as.png
    change-accept.png
    change-next.png
    change-reject.png
    changes-output.png
    changes-track.png
    closetab.png
    close-tab-group.png
    copy.png
    cut.png
    depth-decrement.png
    depth-increment.png
    dialog-show_character.png
    dialog-show_findreplace.png
    dialog-show_print.png
    dialog-show_spellchecker.png
    dialog-show-new-inset_citation.png
    dialog-show-new-inset_graphics.png
    dialog-show-new-inset_include.png
    dialog-show-new-inset_ref.png
    dialog-toggle_findreplaceadv.png
    dialog-toggle_toc.png
    ert-insert.png
    file-open.png
    float-insert_figure.png
    float-insert_table.png
    font-bold.png
    font-emph.png
    font-noun.png
    footnote-insert.png
    href-insert.png
    iconsize.png
    index-insert.png
    label-insert.png
    layout.png
    layout-document.png
    layout-paragraph.png
    layout_Description.png
    layout_Enumerate.png
    layout_Itemize.png
    layout_List.png
    lyx-quit.png
    marginalnote-insert.png
    master-buffer-update.png
    master-buffer-view.png
    math-macro_newmacroname_newcommand.png
    math-mode.png
    nomencl-insert.png
    note-insert.png
    note-next.png
    paste.png
    redo.png
    reload.png
    split-view_horizontal.png
    split-view_vertical.png
    tabular-insert.png
    textstyle-apply.png
    thesaurus-entry.png
    toolbar-toggle_math.png
    toolbar-toggle_table.png
    undo.png
    update-others.png
    url-insert.png
    vc-revert.png
    view-others.png
''')


lib_images_attic_extra_files = Split('''
    dialog-show_mathpanel.png
''')


# Do not add subscript.sty to lib_tex_files since we don't want to install it
# (it is part of TeXLive). We include it in our source package only so that
# packagers may decide to install it (e.g. for MikTeX).
lib_tex_files = Split('''
    broadway.cls
    hollywood.cls
    lyxchess.sty
    lyxskak.sty
    revtex.cls
''')


lib_doc_files = Split('''
    Customization.lyx
    DummyDocument1.lyx
    DummyDocument2.lyx
    DummyTextDocument.txt
    EmbeddedObjects.lyx
    Additional.lyx
    Formula-numbering.lyx
    Intro.lyx
    LaTeXConfig.lyx
    LFUNs.lyx
    Math.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_biblio_files = Split('''
    alphadin.bst
    LyXDocs.bib
''')


lib_doc_clipart_files = Split('''
    Abstract.pdf
    BoxInsetDefaultQt4.png
    ChangesToolbar.png
    ChildDocumentQt4.png
    CommentNoteImageQt4.png
    endnotes.pdf
    ERT.png
    escher-lsd.eps
    ExternalMaterialQt4.png
    ExtraToolbar.png
    floatQt4.png
    footnoteQt4.png
    GreyedOutNoteImageQt4.png
    labelQt4.png
    LaTeX.png
    LyXNoteImageQt4.png
    macrobox.png
    MacroToolbar.png
    macrouse.png
    mobius.eps
    platypus.eps
    referenceQt4.png
    SpaceMarker.png
    StandardToolbar.png
    ToolbarEnvBox.png
    ViewToolbar.png
    with_fntright.pdf
    without_fntright.pdf
''')


lib_doc_extra_files = Split('''
    Makefile.am
    README.Documentation
''')


lib_doc_ca_files = Split('''
    Intro.lyx
''')


lib_doc_cs_files = Split('''
    Tutorial.lyx
''')


lib_doc_de_files = Split('''
    Customization.lyx
    DummyDocument1.lyx
    DummyDocument2.lyx
    DummyTextDocument.txt
    EmbeddedObjects.lyx
    Additional.lyx
    Formelnummerierung.lyx
    Intro.lyx
    Math.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_de_clipart_files = Split('''
    ERT.png
    ExternesMaterialQt4.png
    FussnoteQt4.png
    GleitobjektQt4.png
    GrauschriftNotizQt4.png
    KommentarQt4.png
    LyXNotizQt4.png
    Marke.png
    Querverweis.png
    StandardBoxQt4.png
    UnterdokumentQt4.png
''')


lib_doc_da_files = Split('''
    Intro.lyx
''')


lib_doc_el_files = Split('''
    Intro.lyx
''')


lib_doc_es_files = Split('''
    Customization.lyx
    DocumentoPostizo1.lyx
    DocumentoPostizo2.lyx
    DocumentoTextoPostizo.txt
    EmbeddedObjects.lyx
    Additional.lyx
    Formula-numbering.lyx
    Intro.lyx
    Math.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_es_clipart_files = Split('''
    ComentNotaImagenQt4.png
    CuadroMinipagQt4.png
    DocumentoHijoQt4.png
    es_ERT.png
    es_ToolbarEnvBox.png
    etiquetaQt4.png
    flotanteQt4.png
    GrisNotaImagenQt4.png
    MaterialExternoQt4.png
    NotaLyXImagenQt4.png
    notapieQt4.png
    referenciaQt4.png
    Resumen.pdf
''')


lib_doc_eu_files = Split('''
    Customization.lyx
    Additional.lyx
    Intro.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_fr_files = Split('''
    Customization.lyx
    DocumentBidon1.lyx
    DocumentBidon2.lyx
    DocumentTexteBidon.txt
    EmbeddedObjects.lyx
    Additional.lyx
    Formula-numbering.lyx
    Intro.lyx
    Math.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_fr_clipart_files = Split('''
    BoxInsetDefaultQt4.png
    ChildDocumentQt4.png
    CommentNoteImageQt4.png
    ERT.png
    floatQt4.png
    footnoteQt4.png
    GreyedOutNoteImageQt4.png
    labelQt4.png
    LyXNoteImageQt4.png    
''')


lib_doc_gl_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_he_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_hu_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_id_files = Split('''
    Intro.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_id_clipart_files = Split('''
    id-contrib.png
    id-footnote.png
    id-lingkungan.png
''')


lib_doc_it_files = Split('''
    Customization.lyx
    Intro.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_it_clipart_files = Split('''
    it_footnoteQt4.png
    it_referenceQt4.png
''')


lib_doc_ja_files = Split('''
    Additional.lyx
    Customization.lyx
    DummyDocument1.lyx
    DummyDocument2.lyx
    EmbeddedObjects.lyx
    Formula-numbering.lyx
    Intro.lyx
    LaTeXConfig.lyx
    Math.lyx
    Shortcuts.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_ja_clipart_files = Split('''
    ChildDocumentQt4.png
    ERT.png
    ExternalMaterialQt4.png
    floatQt4.png
    footnoteQt4.png
    referenceQt4.png
    ToolbarEnvBox.png
''')


lib_doc_nb_files = Split('''
    Intro.lyx
''')


lib_doc_nl_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_pl_files = Split('''
    Additional.lyx
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_pt_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_ro_files = Split('''
    Intro.lyx
''')


lib_doc_ru_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_sl_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_sk_files = Split('''
    Intro.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_sv_files = Split('''
    Intro.lyx
    Shortcuts.lyx
    Tutorial.lyx
''')


lib_doc_uk_files = Split('''
    Intro.lyx
''')


lib_doc_uk_clipart_files = Split('''
    FootnoteQT4.png
''')


lib_doc_zhCN_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_examples_files = Split('''
    aa_sample.lyx
    aas_sample.lyx
    achemso.lyx
    addressExample.adr
    amsart-test.lyx
    amsbook-test.lyx
    armenian-article.lyx
    beamer-g4-mask.jpg
    beamer-g4.jpg
    beamer-icsi-logo.pdf
    beamer-knight1-mask.png
    beamer-knight1.png
    beamer-knight2-mask.png
    beamer-knight2.png
    beamer-knight3-mask.png
    beamer-knight3.png
    beamer-knight4-mask.png
    beamer-knight4.png
    beamerlyxexample1.lyx
    biblioExample.bib
    Braille.lyx
    chess-article.lyx
    chessgame.lyx
    CV-image.eps
    CV-image.png
    docbook_article.lyx
    europeCV.lyx
    example_lyxified.lyx
    example_raw.lyx
    FeynmanDiagrams.lyx
    Foils.lyx
    iecc05.fen
    iecc07.fen
    iecc12.fen
    ItemizeBullets.lyx
    landslide.lyx
    lilypond.lyx
    linguistics.lyx
    listerrors.lyx
    Literate.lyx
    localization_test.lyx
    longsheet.gnumeric
    modernCV.lyx
    MultilingualCaptions.lyx
    noweb2lyx.lyx
    powerdot-example.lyx
    recipebook.lyx
    R-S-statements.lyx
    script_form.lyx
    seminar.lyx
    serial_letter1.lyx
    serial_letter2.lyx
    serial_letter3.lyx
    sheet1.gnumeric
    sheet2.ods
    sheet3.xls
    simplecv.lyx
    splash.lyx
    spreadsheet.lyx
    sweave.lyx
    tufte-book.lyx
    tufte-handout.lyx
    xyfigure.png
    xypic.lyx
''')


lib_examples_ca_files = Split('''
    ItemizeBullets.lyx
    mathed.lyx
    splash.lyx
''')


lib_examples_cs_files = Split('''
    splash.lyx
    priklad_zLyXovany.lyx
    priklad_syrovy.lyx
''')


lib_examples_da_files = Split('''
    splash.lyx
''')


lib_examples_de_files = Split('''
    beispiel_gelyxt.lyx
    beispiel_roh.lyx
    Braille.lyx
    Dezimal.lyx
    ItemizeBullets.lyx
    Lebenslauf.lyx
    linguistics.lyx
    serienbrief1.lyx
    serienbrief2.lyx
    serienbrief3.lyx
    splash.lyx
    tufte-handout.lyx
''')


lib_examples_el_files = Split('''
    splash.lyx
''')


lib_examples_es_files = Split('''
    Braille.lyx
    ejemplo_con_lyx.lyx
    ejemplo_sin_lyx.lyx
    ItemizeBullets.lyx
    linguistics.lyx
    splash.lyx
    tufte-book.lyx
    tufte-handout.lyx
''')


lib_examples_fa_files = Split('''
    splash.lyx
''')


lib_examples_fr_files = Split('''
    AlignementDecimal.lyx
    Braille.lyx
    exemple_brut.lyx
    exemple_lyxifie.lyx
    exemple-powerdot.lyx
    FeynmanDiagrams.lyx
    Foils.lyx
    linguistics.lyx
    ListesPuces.lyx
    seminar.lyx
    simplecv.lyx
    splash.lyx
    xyfigure.png
    xypic.lyx
''')


lib_examples_eu_files = Split('''
    adibide_gordina.lyx
    adibide_lyx-atua.lyx
    splash.lyx
''')


lib_examples_gl_files = Split('''
    exemplo_bruto.lyx
    exemplo_lyxificado.lyx
    splash.lyx
''')


lib_examples_he_files = Split('''
    example_lyxified.lyx
    example_raw.lyx
    splash.lyx
''')


lib_examples_hu_files = Split('''
    example_lyxified.lyx
    example_raw.lyx
    splash.lyx
''')


lib_examples_id_files = Split('''
    splash.lyx
''')


lib_examples_it_files = Split('''
    ItemizeBullets.lyx
    splash.lyx
''')


lib_examples_ja_files = Split('''
    Braille.lyx
    FeynmanDiagrams.lyx
    linguistics.lyx
    splash.lyx
    xyfigure.png
    xypic.lyx
''')


lib_examples_nl_files = Split('''
    opsommingstekens.lyx
    splash.lyx
    voorbeeld_ruw.lyx
    voorbeeld_verlyxt.lyx
''')


lib_examples_pl_files = Split('''
    splash.lyx
''')


lib_examples_pt_files = Split('''
    splash.lyx
''')


lib_examples_ro_files = Split('''
    splash.lyx
''')


lib_examples_ru_files = Split('''
    splash.lyx
''')


lib_examples_sk_files = Split('''
    splash.lyx
''')


lib_examples_sl_files = Split('''
    primer_lyxan.lyx
    primer_surov.lyx
    splash.lyx
''')


lib_examples_sr_files = Split('''
    Braille.lyx
    splash.lyx
''')


lib_examples_sv_files = Split('''
    splash.lyx
''')


lib_examples_uk_files = Split('''
    splash.lyx
''')


lib_lyx2lyx_files = Split('''
    generate_encoding_info.py
    LyX.py
    lyx2lyx
    lyx2lyx_lang.py
    lyx_0_06.py
    lyx_0_08.py
    lyx_0_10.py
    lyx_0_12.py
    lyx_1_0.py
    lyx_1_1.py
    lyx_1_1_5.py
    lyx_1_1_6_0.py
    lyx_1_1_6_3.py
    lyx_1_2.py
    lyx_1_3.py
    lyx_1_4.py
    lyx_1_5.py
    lyx_1_6.py
    lyx_2_0.py
    lyx_2_1.py
    lyx2lyx_tools.py
    parser_tools.py
    profiling.py
    test_parser_tools.py
    unicode_symbols.py
''')


lib_lyx2lyx_extra_files = Split('''
    lyx2lyx_version.py.in
    Makefile.am
''')


lib_layouts_files = Split('''
    aa.layout
    aapaper.layout
    aastex.layout
    achemso.layout
    acmsiggraph.layout
    AEA.layout
    agutex.layout
    agu-dtd.layout
    agums.layout
    amsart.layout
    amsbook.layout
    apa.layout
    arab-article.layout
    armenian-article.layout
    article.layout
    article-beamer.layout
    beamer.layout
    bicaption.module
    book.layout
    broadway.layout
    chess.layout
    cl2emult.layout
    ctex-article.layout
    ctex-book.layout
    ctex-report.layout
    customHeadersFooters.module
    dinbrief.layout
    docbook-book.layout
    docbook-chapter.layout
    docbook-section.layout
    docbook.layout
    doublecol-new.layout
    dtk.layout
    ectaart.layout
    egs.layout
    elsart.layout
    elsarticle.layout
    entcs.layout
    enumitem.module
    europecv.layout
    extarticle.layout
    extbook.layout
    extletter.layout
    extreport.layout
    foils.layout
    frletter.layout
    g-brief.layout
    g-brief2.layout
    heb-article.layout
    heb-letter.layout
    hollywood.layout
    IEEEtran.layout
    ijmpc.layout
    ijmpd.layout
    iopart.layout
    isprs.layout
    jarticle.layout
    jasatex.layout
    jbook.layout
    jgrga.layout
    jreport.layout
    jsarticle.layout
    jsbook.layout
    jss.layout
    kluwer.layout
    latex8.layout
    letter.layout
    lettre.layout
    literate-article.layout
    literate-book.layout
    literate-report.layout
    llncs.layout
    ltugboat.layout
    memoir.layout
    moderncv.layout
    mwart.layout
    mwbk.layout
    mwrep.layout
    paper.layout
    powerdot.layout
    recipebook.layout
    report.layout
    revtex.layout
    revtex4.layout
    revtex4-1.layout
    scrartcl.layout
    scrarticle-beamer.layout
    scrbook.layout
    scrlettr.layout
    scrlttr2.layout
    scrreprt.layout
    seminar.layout
    siamltex.layout
    sigplanconf.layout
    simplecv.layout
    singlecol.layout
    singlecol-new.layout
    slides.layout
    spie.layout
    svglobal.layout
    svglobal3.layout
    svjog.layout
    svmono.layout
    svmult.layout
    svprobth.layout
    tarticle.layout
    tbook.layout
    treport.layout
    tufte-book.layout
    tufte-handout.layout
''')


lib_layouts_inc_files = Split('''
    aapaper.inc
    agu_stdclass.inc
    agu_stdcounters.inc
    agu_stdlists.inc
    agu_stdsections.inc
    agu_stdtitle.inc
    aguplus.inc
    amsdefs.inc
    db_lyxmacros.inc
    db_stdcharstyles.inc
    db_stdclass.inc
    db_stdcounters.inc
    db_stdlayouts.inc
    db_stdlists.inc
    db_stdsections.inc
    db_stdstarsections.inc
    db_stdstruct.inc
    db_stdtitle.inc
    literate-scrap.inc
    lyxmacros.inc
    numarticle.inc
    numreport.inc
    numrevtex.inc
    scrclass.inc
    stdcharstyles.inc
    stdciteformats.inc
    stdclass.inc
    stdcounters.inc
    stdcustom.inc
    stdfloats.inc
    stdinsets.inc
    stdlayouts.inc
    stdletter.inc
    stdlists.inc
    stdlyxlist.inc 
    stdrefprefix.inc
    stdsections.inc
    stdstarsections.inc
    stdstruct.inc
    stdtitle.inc
    svcommon.inc
    theorems.inc
    theorems-ams.inc
    theorems-ams-bytype.inc
    theorems-bytype.inc
    theorems-case.inc
    theorems-order.inc
    theorems-proof.inc
    theorems-proof-std.inc
    theorems-refprefix.inc
    theorems-starred.inc
    theorems-starred-equivalents.inc
    theorems-without-preamble.inc
''')


lib_layouts_module_files = Split('''
    braille.module
    endnotes.module
    eqs-within-sections.module
    figs-within-sections.module
    fixltx2e.module
    fix-cm.module
    foottoend.module
    hanging.module
    initials.module
    lilypond.module
    linguistics.module
    logicalmkup.module
    minimalistic.module
    multicol.module
    noweb.module
    rsphrase.module
    sweave.module
    tabs-within-sections.module
    theorems-ams-bytype.module
    theorems-ams-extended.module
    theorems-ams-extended-bytype.module
    theorems-ams.module
    theorems-bytype.module
    theorems-chap-bytype.module
    theorems-chap.module
    theorems-named.module
    theorems-sec-bytype.module
    theorems-sec.module
    theorems-starred.module
    theorems-std.module
''')

lib_scripts_files = Split('''
    bash_completion
    clean_dvi.py
    convertDefault.py
    csv2lyx.py
    date.py
    ext_copy.py
    fen2ascii.py
    fig2pdftex.py
    fig2pstex.py
    fig_copy.py
    include_bib.py
    layout2layout.py
    legacy_lyxpreview2ppm.py
    listerrors
    lyxpak.py
    lyxpreview2bitmap.py
    lyxpreview_tools.py
    lyxstangle.R
    lyxsweave.R
    prefs2prefs.py
    prefs2prefs_lfuns.py
    prefs2prefs_prefs.py
    TeXFiles.py
    tex_copy.py
''')


lib_bind_files = Split('''
    aqua.bind
    broadway.bind
    cua.bind
    cyrkeys.bind
    emacs.bind
    greekkeys.bind
    hollywood.bind
    latinkeys.bind
    mac.bind
    math.bind
    menus.bind
    sciword.bind
    site.bind
    xemacs.bind
''')


lib_bind_de_files = Split('''
    menus.bind
''')


lib_commands_files = Split('''
    default.def
''')


boost_extra_files = Split('''
    LICENSE_1_0.txt
    Makefile.am
''')


boost_libs_extra_files = Split('''
    Makefile.am
    README
''')


boost_libs_signals_extra_files = Split('''
    Makefile.am
    signals.vcproj
''')


boost_libs_signals_src_files = Split('''
    connection.cpp
    named_slot_map.cpp
    signal_base.cpp
    slot.cpp
    trackable.cpp
''')


boost_libs_signals_src_extra_files = Split('''
    Makefile.am
    pch.h
''')


boost_libs_regex_extra_files = Split('''
    Makefile.am
    regex.vcproj
''')


boost_libs_regex_src_files = Split('''
    c_regex_traits.cpp
    cpp_regex_traits.cpp
    cregex.cpp
    fileiter.cpp
    instances.cpp
    posix_api.cpp
    regex.cpp
    regex_debug.cpp
    regex_raw_buffer.cpp
    regex_traits_defaults.cpp
    w32_regex_traits.cpp
    wide_posix_api.cpp
    winstances.cpp
''')


boost_libs_regex_src_extra_files = Split('''
    Makefile.am
    pch.h
''')



development_Win32_packaging_installer = Split('''
    license.rtf
    lyx.nsi
    settings.nsh
''')


development_Win32_packaging_installer_graphics = Split('''
    header.bmp
    wizard.bmp
''')


development_Win32_packaging_installer_gui = Split('''
    external.nsh
    langselect.nsh
    reinstall.nsh
''')


development_Win32_packaging_installer_include = Split('''
    declarations.nsh
    detection.nsh
    dictlist.nsh    
    filelist.nsh
    gui.nsh
    init.nsh
    langlist.nsh
    nsis.nsh
    variables.nsh
''')


development_Win32_packaging_installer_lang = Split('''
    english.nsh
    french.nsh
    german.nsh
    italian.nsh
''')


development_Win32_packaging_installer_setup = Split('''
    configure.nsh
    install.nsh
    reinstall.nsh
''')
