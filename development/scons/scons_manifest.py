from SCons.Util import Split

TOP_extra_files = Split('''
    ABOUT-NLS
    ANNOUNCE
    COPYING
    ChangeLog
    ChangeLog.1
    INSTALL
    INSTALL.MacOSX
    INSTALL.Win32
    INSTALL.autoconf
    INSTALL.cmake
    INSTALL.scons
    Makefile.am
    NEWS
    OLD-CHANGES
    README
    README.Cygwin
    README.Win32
    README.localization
    RELEASE-NOTES
    UPGRADING
    autogen.sh
    config.log
    configure.ac
    lyx.man
    rename.sh
    scons_lyx.log
''')


src_header_files = Split('''
    ASpell_local.h
    Author.h
    Bidi.h
    Box.h
    BranchList.h
    Buffer.h
    BufferList.h
    BufferParams.h
    BufferView.h
    Bullet.h
    Changes.h
    Chktex.h
    Color.h
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
    Encoding.h
    ErrorList.h
    Exporter.h
    FloatList.h
    Floating.h
    Font.h
    FontIterator.h
    Format.h
    FuncRequest.h
    FuncStatus.h
    Graph.h
    ISpell.h
    Importer.h
    InsetIterator.h
    InsetList.h
    Intl.h
    KeyMap.h
    KeySequence.h
    LaTeX.h
    LaTeXFeatures.h
    Language.h
    Layout.h
    Length.h
    Lexer.h
    LyX.h
    LyXAction.h
    LyXFunc.h
    LyXRC.h
    LyXVC.h
    MenuBackend.h
    Messages.h
    MetricsInfo.h
    Mover.h
    OutputParams.h
    PSpell.h
    ParIterator.h
    Paragraph.h
    ParagraphList.h
    ParagraphMetrics.h
    ParagraphParameters.h
    PrinterParams.h
    Row.h
    Section.h
    Server.h
    ServerSocket.h
    Session.h
    Spacing.h
    SpellBase.h
    TexRow.h
    Text.h
    TextClass.h
    TextClassList.h
    TextMetrics.h
    Thesaurus.h
    TocBackend.h
    ToolbarBackend.h
    Trans.h
    Undo.h
    VCBackend.h
    VSpace.h
    Variables.h
    WordLangTuple.h
    buffer_funcs.h
    bufferview_funcs.h
    callback.h
    debug.h
    factory.h
    gettext.h
    lengthcommon.h
    lfuns.h
    lyxfind.h
    lyxlayout_ptr_fwd.h
    output.h
    output_docbook.h
    output_latex.h
    output_plaintext.h
    paper.h
    paragraph_funcs.h
    rowpainter.h
    sgml.h
    toc.h
    update_flags.h
    version.h
''')


src_pre_files = Split('''
    Author.cpp
    Bidi.cpp
    BranchList.cpp
    Buffer.cpp
    BufferList.cpp
    BufferParams.cpp
    BufferView.cpp
    Bullet.cpp
    Changes.cpp
    Chktex.cpp
    Color.cpp
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
    FloatList.cpp
    Floating.cpp
    Font.cpp
    FontIterator.cpp
    Format.cpp
    FuncRequest.cpp
    FuncStatus.cpp
    Graph.cpp
    Importer.cpp
    InsetIterator.cpp
    InsetList.cpp
    Intl.cpp
    KeyMap.cpp
    KeySequence.cpp
    LaTeX.cpp
    LaTeXFeatures.cpp
    Language.cpp
    Layout.cpp
    Length.cpp
    Lexer.cpp
    LyX.cpp
    LyXAction.cpp
    LyXFunc.cpp
    LyXRC.cpp
    LyXVC.cpp
    MenuBackend.cpp
    Messages.cpp
    MetricsInfo.cpp
    Mover.cpp
    OutputParams.cpp
    ParIterator.cpp
    Paragraph.cpp
    ParagraphMetrics.cpp
    ParagraphParameters.cpp
    Row.cpp
    Server.cpp
    ServerSocket.cpp
    Session.cpp
    Spacing.cpp
    TexRow.cpp
    Text.cpp
    Text2.cpp
    Text3.cpp
    TextClass.cpp
    TextClassList.cpp
    TextMetrics.cpp
    TocBackend.cpp
    ToolbarBackend.cpp
    Trans.cpp
    Undo.cpp
    VCBackend.cpp
    VSpace.cpp
    boost.cpp
    buffer_funcs.cpp
    bufferview_funcs.cpp
    callback.cpp
    debug.cpp
    factory.cpp
    gettext.cpp
    lengthcommon.cpp
    lyxfind.cpp
    output.cpp
    output_docbook.cpp
    output_latex.cpp
    output_plaintext.cpp
    paragraph_funcs.cpp
    rowpainter.cpp
    sgml.cpp
    toc.cpp
''')


src_post_files = Split('''
    Box.cpp
    Dimension.cpp
    PrinterParams.cpp
    SpellBase.cpp
    Thesaurus.cpp
''')


src_extra_src_files = Split('''
    ASpell.cpp
    ISpell.cpp
    PSpell.cpp
    Section.cpp
    Variables.cpp
    main.cpp
    stamp-h.in
    version.cpp.in
''')


src_extra_files = Split('''
    ChangeLog
    Makefile.am
    pch.h
''')


src_client_header_files = Split('''
    Messages.h
    debug.h
''')


src_client_files = Split('''
    Messages.cpp
    boost.cpp
    client.cpp
    debug.cpp
    gettext.cpp
''')


src_client_extra_files = Split('''
    ChangeLog
    Makefile.am
    lyxclient.man
    pch.h
''')


src_support_header_files = Split('''
    ExceptionMessage.h
    FileFilterList.h
    FileMonitor.h
    FileName.h
    ForkedCallQueue.h
    Forkedcall.h
    ForkedcallsController.h
    Package.h
    Path.h
    RandomAccessList.h
    Systemcall.h
    Translator.h
    convert.h
    copied_ptr.h
    cow_ptr.h
    debugstream.h
    docstream.h
    docstring.h
    environment.h
    filetools.h
    fs_extras.h
    gzstream.h
    limited_stack.h
    lstrings.h
    lyxalgo.h
    lyxlib.h
    lyxmanip.h
    lyxtime.h
    os.h
    os_win32.h
    qstring_helpers.h
    socktools.h
    std_istream.h
    std_ostream.h
    textutils.h
    types.h
    unicode.h
    userinfo.h
''')


src_support_files = Split('''
    FileFilterList.cpp
    FileMonitor.cpp
    FileName.cpp
    ForkedCallQueue.cpp
    Forkedcall.cpp
    ForkedcallsController.cpp
    Path.cpp
    Systemcall.cpp
    abort.cpp
    chdir.cpp
    convert.cpp
    copy.cpp
    docstream.cpp
    docstring.cpp
    environment.cpp
    filetools.cpp
    fs_extras.cpp
    gzstream.cpp
    getcwd.cpp
    kill.cpp
    lstrings.cpp
    lyxsum.cpp
    lyxtime.cpp
    mkdir.cpp
    os.cpp
    qstring_helpers.cpp
    rename.cpp
    socktools.cpp
    tempname.cpp
    textutils.cpp
    unicode.cpp
    unlink.cpp
    userinfo.cpp
''')


src_support_extra_header_files = Split('''
    
''')


src_support_extra_src_files = Split('''
    Package.cpp.in
    atexit.c
    os_cygwin.cpp
    os_unix.cpp
    os_win32.cpp
    strerror.c
''')


src_support_extra_files = Split('''
    ChangeLog
    Makefile.am
    pch.h
''')


src_support_tests_extra_files = Split('''
    ChangeLog
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
    LoaderQueue.h
    PreviewImage.h
    PreviewLoader.h
    Previews.h
''')


src_graphics_files = Split('''
    GraphicsCache.cpp
    GraphicsCacheItem.cpp
    GraphicsConverter.cpp
    GraphicsImage.cpp
    GraphicsLoader.cpp
    GraphicsParams.cpp
    GraphicsTypes.cpp
    LoaderQueue.cpp
    PreviewImage.cpp
    PreviewLoader.cpp
    Previews.cpp
''')


src_graphics_extra_files = Split('''
    ChangeLog
    Makefile.am
    pch.h
''')


src_mathed_header_files = Split('''
    CommandInset.h
    InsetMath.h
    InsetMathAMSArray.h
    InsetMathArray.h
    InsetMathBig.h
    InsetMathBinom.h
    InsetMathBoldSymbol.h
    InsetMathBox.h
    InsetMathBoxed.h
    InsetMathBrace.h
    InsetMathCases.h
    InsetMathChar.h
    InsetMathColor.h
    InsetMathComment.h
    InsetMathDFrac.h
    InsetMathDecoration.h
    InsetMathDelim.h
    InsetMathDiff.h
    InsetMathDots.h
    InsetMathEnv.h
    InsetMathExFunc.h
    InsetMathExInt.h
    InsetMathFBox.h
    InsetMathFont.h
    InsetMathFontOld.h
    InsetMathFrac.h
    InsetMathFracBase.h
    InsetMathFrameBox.h
    InsetMathGrid.h
    InsetMathHull.h
    InsetMathKern.h
    InsetMathLefteqn.h
    InsetMathLim.h
    InsetMathMakebox.h
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
    InsetMathSplit.h
    InsetMathSqrt.h
    InsetMathStackrel.h
    InsetMathString.h
    InsetMathSubstack.h
    InsetMathSymbol.h
    InsetMathTFrac.h
    InsetMathTabular.h
    InsetMathUnderset.h
    InsetMathUnknown.h
    InsetMathXArrow.h
    InsetMathXYMatrix.h
    MacroTable.h
    MathAtom.h
    MathAutoCorrect.h
    MathData.h
    MathExtern.h
    MathFactory.h
    MathGridInfo.h
    MathMacro.h
    MathMacroArgument.h
    MathMacroTemplate.h
    MathParser.h
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
    InsetMathBinom.cpp
    InsetMathBoldSymbol.cpp
    InsetMathBox.cpp
    InsetMathBoxed.cpp
    InsetMathBrace.cpp
    InsetMathCases.cpp
    InsetMathChar.cpp
    InsetMathColor.cpp
    InsetMathComment.cpp
    InsetMathDFrac.cpp
    InsetMathDecoration.cpp
    InsetMathDelim.cpp
    InsetMathDiff.cpp
    InsetMathDots.cpp
    InsetMathEnv.cpp
    InsetMathExFunc.cpp
    InsetMathExInt.cpp
    InsetMathFBox.cpp
    InsetMathFont.cpp
    InsetMathFontOld.cpp
    InsetMathFrac.cpp
    InsetMathFracBase.cpp
    InsetMathFrameBox.cpp
    InsetMathGrid.cpp
    InsetMathHull.cpp
    InsetMathKern.cpp
    InsetMathLefteqn.cpp
    InsetMathLim.cpp
    InsetMathMakebox.cpp
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
    InsetMathSplit.cpp
    InsetMathSqrt.cpp
    InsetMathStackrel.cpp
    InsetMathString.cpp
    InsetMathSubstack.cpp
    InsetMathSymbol.cpp
    InsetMathTFrac.cpp
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
    ChangeLog
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
    Font.h
    Parser.h
    Spacing.h
    tex2lyx.h
''')


src_tex2lyx_files = Split('''
    Context.cpp
    Font.cpp
    Parser.cpp
    boost.cpp
    gettext.cpp
    lengthcommon.cpp
    math.cpp
    preamble.cpp
    table.cpp
    tex2lyx.cpp
    text.cpp
''')


src_tex2lyx_copied_files = Split('''
    Counters.cpp
    FloatList.cpp
    Floating.cpp
    Layout.cpp
    Lexer.cpp
    TextClass.cpp
''')


src_tex2lyx_copied_header_files = Split('''
    Layout.h
    Lexer.h
    TextClass.h
''')


src_tex2lyx_extra_files = Split('''
    ChangeLog
    Makefile.am
    pch.h
    test-insets.tex
    test-structure.tex
    test.ltx
    tex2lyx.man
''')


src_frontends_header_files = Split('''
    Alert_pimpl.h
    Application.h
    Clipboard.h
    Dialogs.h
    FileDialog.h
    FontLoader.h
    FontMetrics.h
    Gui.h
    KeySymbol.h
    LyXView.h
    Menubar.h
    NoGuiFontLoader.h
    NoGuiFontMetrics.h
    Painter.h
    Selection.h
    Timeout.h
    Toolbars.h
    WorkArea.h
    alert.h
    key_state.h
    mouse_state.h
''')


src_frontends_files = Split('''
    Application.cpp
    Dialogs.cpp
    LyXView.cpp
    Painter.cpp
    Timeout.cpp
    Toolbars.cpp
    WorkArea.cpp
    alert.cpp
''')


src_frontends_extra_files = Split('''
    ChangeLog
    Makefile.am
    pch.h
''')


src_frontends_controllers_header_files = Split('''
    BCView.h
    ButtonController.h
    ButtonPolicy.h
    ControlAboutlyx.h
    ControlBibtex.h
    ControlBox.h
    ControlBranch.h
    ControlChanges.h
    ControlCharacter.h
    ControlCitation.h
    ControlCommand.h
    ControlCommandBuffer.h
    ControlDocument.h
    ControlERT.h
    ControlErrorList.h
    ControlExternal.h
    ControlFloat.h
    ControlGraphics.h
    ControlInclude.h
    ControlListings.h
    ControlLog.h
    ControlMath.h
    ControlNote.h
    ControlParagraph.h
    ControlPrefs.h
    ControlPrint.h
    ControlRef.h
    ControlSearch.h
    ControlSendto.h
    ControlShowFile.h
    ControlSpellchecker.h
    ControlTabular.h
    ControlTabularCreate.h
    ControlTexinfo.h
    ControlThesaurus.h
    ControlToc.h
    ControlVSpace.h
    ControlViewSource.h
    ControlWrap.h
    Dialog.h
    Kernel.h
    frontend_helpers.h
''')


src_frontends_controllers_files = Split('''
    BCView.cpp
    ButtonController.cpp
    ButtonPolicy.cpp
    ControlAboutlyx.cpp
    ControlBibtex.cpp
    ControlBox.cpp
    ControlBranch.cpp
    ControlChanges.cpp
    ControlCharacter.cpp
    ControlCitation.cpp
    ControlCommand.cpp
    ControlCommandBuffer.cpp
    ControlDocument.cpp
    ControlERT.cpp
    ControlErrorList.cpp
    ControlExternal.cpp
    ControlFloat.cpp
    ControlGraphics.cpp
    ControlInclude.cpp
    ControlListings.cpp
    ControlLog.cpp
    ControlMath.cpp
    ControlNote.cpp
    ControlParagraph.cpp
    ControlPrefs.cpp
    ControlPrint.cpp
    ControlRef.cpp
    ControlSearch.cpp
    ControlSendto.cpp
    ControlShowFile.cpp
    ControlSpellchecker.cpp
    ControlTabular.cpp
    ControlTabularCreate.cpp
    ControlTexinfo.cpp
    ControlThesaurus.cpp
    ControlToc.cpp
    ControlVSpace.cpp
    ControlViewSource.cpp
    ControlWrap.cpp
    Dialog.cpp
    Kernel.cpp
    frontend_helpers.cpp
''')


src_frontends_controllers_extra_files = Split('''
    BCView.tmpl
    ChangeLog
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
    Action.h
    BulletsModule.h
    CheckedLineEdit.h
    ColorCache.h
    DockView.h
    EmptyTable.h
    FloatPlacement.h
    GuiApplication.h
    GuiClipboard.h
    GuiFontLoader.h
    GuiImplementation.h
    GuiSelection.h
    GuiView.h
    GuiWorkArea.h
    IconPalette.h
    InsertTableWidget.h
    LengthCombo.h
    LyXFileDialog.h
    PanelStack.h
    QAbout.h
    QBibitem.h
    QBibtex.h
    QBox.h
    QBranch.h
    QBranches.h
    QChanges.h
    QCharacter.h
    QCharacter.h
    QCitation.h
    QCitationDialog.h
    QCommandBuffer.h
    QCommandEdit.h
    QDelimiterDialog.h
    QDialogView.h
    QDocument.h
    QERT.h
    QErrorList.h
    QExternal.h
    QFloat.h
    QFloatDialog.h
    QFontExample.h
    QGraphics.h
    QGraphicsDialog.h
    QGraphicsUi.h
    QInclude.h
    QIndex.h
    QKeySymbol.h
    QListings.h
    QLImage.h
    QLMenubar.h
    QLPainter.h
    QLPopupMenu.h
    QLPrintDialog.h
    QLToolbar.h
    QLog.h
    QMathMatrixDialog.h
    QNomencl.h
    QNote.h
    QParagraph.h
    QPrefs.h
    QPrint.h
    QRef.h
    QSearch.h
    QSendto.h
    QSetBorder.h
    QShowFile.h
    QSpellchecker.h
    QTabular.h
    QTabularCreate.h
    QTexinfo.h
    QThesaurus.h
    QToc.h
    QURLDialog.h
    QVSpace.h
    QViewSource.h
    QWrap.h
    Qt2BC.h
    TocModel.h
    TocWidget.h
    UrlView.h
    Validator.h
    qlkey.h
    qtTimeout.h
    qt_helpers.h
    socket_callback.h
''')


src_frontends_qt4_files = Split('''
    Action.cpp
    BulletsModule.cpp
    CheckedLineEdit.cpp
    ColorCache.cpp
    Dialogs.cpp
    EmptyTable.cpp
    FileDialog.cpp
    FloatPlacement.cpp
    GuiApplication.cpp
    GuiClipboard.cpp
    GuiFontLoader.cpp
    GuiFontMetrics.cpp
    GuiImplementation.cpp
    GuiSelection.cpp
    GuiView.cpp
    GuiWorkArea.cpp
    IconPalette.cpp
    InsertTableWidget.cpp
    KeySymbol.cpp
    LengthCombo.cpp
    LyXFileDialog.cpp
    PanelStack.cpp
    QAbout.cpp
    QBibitem.cpp
    QBibtex.cpp
    QBox.cpp
    QBranch.cpp
    QBranches.cpp
    QChanges.cpp
    QCharacter.cpp
    QCitation.cpp
    QCitationDialog.cpp
    QCommandBuffer.cpp
    QCommandEdit.cpp
    QDelimiterDialog.cpp
    QDialogView.cpp
    QDocument.cpp
    QERT.cpp
    QErrorList.cpp
    QExternal.cpp
    QFloat.cpp
    QFloatDialog.cpp
    QFontExample.cpp
    QGraphics.cpp
    QGraphicsDialog.cpp
    QInclude.cpp
    QIndex.cpp
    QKeySymbol.cpp
    QListings.cpp
    QLImage.cpp
    QLMenubar.cpp
    QLPainter.cpp
    QLPopupMenu.cpp
    QLPrintDialog.cpp
    QLToolbar.cpp
    QLog.cpp
    QMathMatrixDialog.cpp
    QNomencl.cpp
    QNote.cpp
    QParagraph.cpp
    QPrefs.cpp
    QPrint.cpp
    QRef.cpp
    QSearch.cpp
    QSendto.cpp
    QSetBorder.cpp
    QShowFile.cpp
    QSpellchecker.cpp
    QTabular.cpp
    QTabularCreate.cpp
    QTexinfo.cpp
    QThesaurus.cpp
    QToc.cpp
    QURLDialog.cpp
    QVSpace.cpp
    QViewSource.cpp
    QWrap.cpp
    Qt2BC.cpp
    TocModel.cpp
    TocWidget.cpp
    UrlView.cpp
    Validator.cpp
    alert_pimpl.cpp
    qtTimeout.cpp
    qt_helpers.cpp
    socket_callback.cpp
''')


src_frontends_qt4_moc_files = Split('''
    Action.cpp
    BulletsModule.cpp
    EmptyTable.cpp
    FloatPlacement.cpp
    GuiApplication.cpp
    GuiClipboard.cpp
    GuiImplementation.cpp
    GuiSelection.cpp
    GuiView.cpp
    GuiWorkArea.cpp
    IconPalette.cpp
    InsertTableWidget.cpp
    LengthCombo.cpp
    LyXFileDialog.cpp
    PanelStack.cpp
    QAbout.cpp
    QBibitem.cpp
    QBibtex.cpp
    QBox.cpp
    QBranch.cpp
    QBranches.cpp
    QChanges.cpp
    QCharacter.cpp
    QCitationDialog.cpp
    QCommandBuffer.cpp
    QCommandEdit.cpp
    QDelimiterDialog.cpp
    QDialogView.cpp
    QDocument.cpp
    QERT.cpp
    QErrorList.cpp
    QExternal.cpp
    QFloatDialog.cpp
    QGraphicsDialog.cpp
    QInclude.cpp
    QIndex.cpp
    QListings.cpp
    QLMenubar.cpp
    QLPopupMenu.cpp
    QLPrintDialog.cpp
    QLToolbar.cpp
    QLog.cpp
    QLog.cpp
    QMathMatrixDialog.cpp
    QNomencl.cpp
    QNomencl.cpp
    QNote.cpp
    QParagraph.cpp
    QPrefs.cpp
    QRef.cpp
    QSearch.cpp
    QSendto.cpp
    QSetBorder.cpp
    QShowFile.cpp
    QSpellchecker.cpp
    QTabular.cpp
    QTabularCreate.cpp
    QTexinfo.cpp
    QThesaurus.cpp
    QToc.cpp
    QURLDialog.cpp
    QVSpace.cpp
    QViewSource.cpp
    QWrap.cpp
    TocModel.cpp
    TocWidget.cpp
    Validator.cpp
    socket_callback.cpp
''')


src_frontends_qt4_extra_files = Split('''
    ChangeLog
    GuiFontMetrics.h
    Makefile.am
    Makefile.dialogs
    README
    pch.h
''')


src_frontends_qt4_ui_files = Split('''
    AboutUi.ui
    AskForTextUi.ui
    BibitemUi.ui
    BiblioUi.ui
    BibtexAddUi.ui
    BibtexUi.ui
    BoxUi.ui
    BranchUi.ui
    BranchesUi.ui
    BulletsUi.ui
    ChangesUi.ui
    CharacterUi.ui
    CitationUi.ui
    DelimiterUi.ui
    DocumentUi.ui
    ERTUi.ui
    ErrorListUi.ui
    ExternalUi.ui
    FloatPlacementUi.ui
    FloatUi.ui
    FontUi.ui
    GraphicsUi.ui
    IncludeUi.ui
    IndexUi.ui
    LaTeXUi.ui
    LanguageUi.ui
    ListingsUi.ui
    LogUi.ui
    MarginsUi.ui
    MathMatrixUi.ui
    MathsUi.ui
    NomenclUi.ui
    NoteUi.ui
    NumberingUi.ui
    PageLayoutUi.ui
    ParagraphUi.ui
    PreambleUi.ui
    PrefColorsUi.ui
    PrefConvertersUi.ui
    PrefCopiersUi.ui
    PrefCygwinPathUi.ui
    PrefDateUi.ui
    PrefDisplayUi.ui
    PrefFileformatsUi.ui
    PrefIdentityUi.ui
    PrefKeyboardUi.ui
    PrefLanguageUi.ui
    PrefLatexUi.ui
    PrefPathsUi.ui
    PrefPlaintextUi.ui
    PrefPrinterUi.ui
    PrefScreenFontsUi.ui
    PrefSpellcheckerUi.ui
    PrefUi.ui
    PrefsUi.ui
    PrintUi.ui
    RefUi.ui
    SearchUi.ui
    SendtoUi.ui
    ShowFileUi.ui
    SpellcheckerUi.ui
    TabularCreateUi.ui
    TabularUi.ui
    TexinfoUi.ui
    TextLayoutUi.ui
    ThesaurusUi.ui
    TocUi.ui
    URLUi.ui
    VSpaceUi.ui
    ViewSourceUi.ui
    WrapUi.ui
''')


src_frontends_qt4_ui_extra_files = Split('''
    Makefile.am
    compile_uic.sh
''')


src_insets_header_files = Split('''
    ExternalSupport.h
    ExternalTemplate.h
    ExternalTransforms.h
    Inset.h
    InsetBibitem.h
    InsetBibtex.h
    InsetBox.h
    InsetBranch.h
    InsetCaption.h
    InsetCharStyle.h
    InsetCitation.h
    InsetCollapsable.h
    InsetCommand.h
    InsetCommandParams.h
    InsetERT.h
    InsetEnvironment.h
    InsetExternal.h
    InsetFloat.h
    InsetFloatList.h
    InsetFoot.h
    InsetFootlike.h
    InsetGraphics.h
    InsetGraphicsParams.h
    InsetHFill.h
    InsetInclude.h
    InsetIndex.h
    InsetLabel.h
    InsetLine.h
    InsetListings.h
    InsetListingsParams.h
    InsetMarginal.h
    InsetNewline.h
    InsetNomencl.h
    InsetNote.h
    InsetOptArg.h
    InsetPagebreak.h
    InsetQuotes.h
    InsetRef.h
    InsetSpace.h
    InsetSpecialChar.h
    InsetTOC.h
    InsetTabular.h
    InsetText.h
    InsetUrl.h
    InsetVSpace.h
    InsetWrap.h
    MailInset.h
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
    InsetBibitem.cpp
    InsetBibtex.cpp
    InsetBox.cpp
    InsetBranch.cpp
    InsetCaption.cpp
    InsetCharStyle.cpp
    InsetCitation.cpp
    InsetCollapsable.cpp
    InsetCommand.cpp
    InsetCommandParams.cpp
    InsetERT.cpp
    InsetEnvironment.cpp
    InsetExternal.cpp
    InsetFloat.cpp
    InsetFloatList.cpp
    InsetFoot.cpp
    InsetFootlike.cpp
    InsetGraphics.cpp
    InsetGraphicsParams.cpp
    InsetHFill.cpp
    InsetInclude.cpp
    InsetIndex.cpp
    InsetLabel.cpp
    InsetLine.cpp
    InsetListings.cpp
    InsetListingsParams.cpp
    InsetMarginal.cpp
    InsetNewline.cpp
    InsetNomencl.cpp
    InsetNote.cpp
    InsetOptArg.cpp
    InsetPagebreak.cpp
    InsetQuotes.cpp
    InsetRef.cpp
    InsetSpace.cpp
    InsetSpecialChar.cpp
    InsetTOC.cpp
    InsetTabular.cpp
    InsetText.cpp
    InsetUrl.cpp
    InsetVSpace.cpp
    InsetWrap.cpp
    MailInset.cpp
    RenderButton.cpp
    RenderGraphic.cpp
    RenderPreview.cpp
''')


src_insets_extra_files = Split('''
    ChangeLog
    InsetTheorem.cpp
    InsetTheorem.h
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
    ChangeLog
    VERSION
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
''')


config_extra_files = Split('''
    ChangeLog
    Makefile.am
    common.am
    config.guess
    config.rpath
    config.sub
    depcomp
    install-sh
    libtool.m4
    ltmain.sh
    lyxinclude.m4
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
    ChangeLog
    LINGUAS
    Makefile.in.in
    Makevars
    POTFILES.in
    README
    Rules-quot
    bg.po
    boldquot.sed
    ca.po
    cs.po
    da.po
    de.po
    en@boldquot.header
    en@quot.header
    es.po
    eu.po
    fi.po
    fr.po
    gl.po
    he.po
    hu.po
    insert-header.sin
    it.po
    ja.po
    ko.po
    lyx_pot.py
    nb.po
    nl.po
    nn.po
    pl.po
    pocheck.pl
    postats.sh
    pt.po
    quot.sed
    remove-potcdate.sin
    ro.po
    ru.po
    sk.po
    sl.po
    sv.po
    tr.po
    wa.po
    uk.po
    zh_CN.po
    zh_TW.po
''')


lib_files = Split('''
    CREDITS
    chkconfig.ltx
    configure.py
    encodings
    external_templates
    languages
    symbols
    syntax.default
    unicodesymbols
''')


lib_extra_files = Split('''
    ChangeLog
    Makefile.am
    autocorrect
    build-listerrors
    generate_contributions.py
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
    ACM-sigplan.lyx
    agu_article.lyx
    apa.lyx
    beamer-conference-ornate-20min.lyx
    de_beamer-conference-ornate-20min.lyx
    dinbrief.lyx
    docbook_article.lyx
    elsart.lyx
    fr_beamer-conference-ornate-20min.lyx
    g-brief-de.lyx
    g-brief-en.lyx
    g-brief2.lyx
    hollywood.lyx
    IEEEtran.lyx
    ijmpc.lyx
    ijmpd.lyx
    iop-article.lyx
    JSS-article.lyx
    kluwer.lyx
    koma-letter2.lyx
    latex8.lyx
    letter.lyx
    README.new_templates
    revtex.lyx
    revtex4.lyx
    slides.lyx
''')


lib_ui_files = Split('''
    classic.ui
    default.ui
    stdmenus.inc
    stdtoolbars.inc
''')


lib_fonts_files = Split('''
    BaKoMaFontLicense.txt
    ReadmeBaKoMa4LyX.txt
    cmex10.ttf
    cmmi10.ttf
    cmr10.ttf
    cmsy10.ttf
    esint10.ttf
    eufm10.ttf
    msam10.ttf
    msbm10.ttf
    wasy10.ttf
''')


lib_images_files = Split('''
    all-changes-accept.xpm
    all-changes-reject.xpm
    amssymb.xpm
    banner.png
    bookmark-goto.xpm
    bookmark-save.xpm
    box-insert.xpm
    break-line.xpm
    buffer-close.xpm
    buffer-export_dvi.xpm
    buffer-export_latex.xpm
    buffer-export_pdf2.xpm
    buffer-export_ps.xpm
    buffer-export_text.xpm
    buffer-new.xpm
    buffer-reload.xpm
    buffer-update_dvi.xpm
    buffer-update_pdf2.xpm
    buffer-update_ps.xpm
    buffer-view_dvi.xpm
    buffer-view_pdf2.xpm
    buffer-view_ps.xpm
    buffer-write-as.xpm
    buffer-write.xpm
    build-program.xpm
    change-accept.xpm
    change-next.xpm
    change-reject.xpm
    changes-merge.xpm
    changes-output.xpm
    changes-track.xpm
    closetab.xpm
    copy.xpm
    cut.xpm
    demote.xpm
    depth-decrement.xpm
    depth-increment.xpm
    dialog-preferences.xpm
    dialog-show-new-inset_citation.xpm
    dialog-show-new-inset_graphics.xpm
    dialog-show-new-inset_include.xpm
    dialog-show-new-inset_ref.xpm
    dialog-show_character.xpm
    dialog-show_findreplace.xpm
    dialog-show_mathdelimiter.xpm
    dialog-show_mathmatrix.xpm
    dialog-show_print.xpm
    dialog-show_spellchecker.xpm
    dialog-toggle_toc.xpm
    down.xpm
    ert-insert.xpm
    file-open.xpm
    float-insert_figure.xpm
    float-insert_table.xpm
    font-bold.xpm
    font-emph.xpm
    font-free-apply.xpm
    font-noun.xpm
    font-sans.xpm
    footnote-insert.xpm
    index-insert.xpm
    label-insert.xpm
    layout-document.xpm
    layout-paragraph.xpm
    layout.xpm
    layout_Description.xpm
    layout_Enumerate.xpm
    layout_Itemize.xpm
    layout_List.xpm
    layout_LyX-Code.xpm
    layout_Scrap.xpm
    layout_Section.xpm
    lyx-quit.xpm
    lyx.xpm
    marginalnote-insert.xpm
    math-display.xpm
    math-matrix.xpm
    math-mode.xpm
    math-subscript.xpm
    math-superscript.xpm
    nomencl-insert.xpm
    note-insert.xpm
    note-next.xpm
    paste.xpm
    promote.xpm
    psnfss1.xpm
    psnfss2.xpm
    psnfss3.xpm
    psnfss4.xpm
    redo.xpm
    reload.xpm
    standard.xpm
    tabular-feature_align-center.xpm
    tabular-feature_align-left.xpm
    tabular-feature_align-right.xpm
    tabular-feature_append-column.xpm
    tabular-feature_append-row.xpm
    tabular-feature_delete-column.xpm
    tabular-feature_delete-row.xpm
    tabular-feature_multicolumn.xpm
    tabular-feature_set-all-lines.xpm
    tabular-feature_set-longtabular.xpm
    tabular-feature_set-rotate-cell.xpm
    tabular-feature_toggle-rotate-cell.xpm
    tabular-feature_set-rotate-tabular.xpm
    tabular-feature_toggle-rotate-tabular.xpm
    tabular-feature_toggle-line-bottom.xpm
    tabular-feature_toggle-line-left.xpm
    tabular-feature_toggle-line-right.xpm
    tabular-feature_toggle-line-top.xpm
    tabular-feature_unset-all-lines.xpm
    tabular-feature_valign-bottom.xpm
    tabular-feature_valign-middle.xpm
    tabular-feature_valign-top.xpm
    tabular-insert.xpm
    thesaurus-entry.xpm
    toolbar-toggle_math.xpm
    toolbar-toggle_math_panels.xpm
    toolbar-toggle_table.xpm
    undo.xpm
    unknown.xpm
    up.xpm
    url-insert.xpm
''')


lib_images_extra_files = Split('''
    README
    font-smallcaps.xpm
''')


lib_images_math_files = Split('''
    Bbbk.xpm
    Finv.xpm
    Game.xpm
    Im.xpm
    Lleftarrow.xpm
    Lsh.xpm
    Re.xpm
    Rrightarrow.xpm
    Rsh.xpm
    Vvdash.xpm
    acute.xpm
    aleph.xpm
    alpha.xpm
    amalg.xpm
    angle.xpm
    approx.xpm
    approxeq.xpm
    asymp.xpm
    backepsilon.xpm
    backprime.xpm
    backsim.xpm
    backsimeq.xpm
    backslash.xpm
    bar.xpm
    bars.xpm
    barwedge.xpm
    because.xpm
    beta.xpm
    beth.xpm
    between.xpm
    bigcap.xpm
    bigcirc.xpm
    bigcup.xpm
    bigodot.xpm
    bigoplus.xpm
    bigotimes.xpm
    bigsqcup.xpm
    bigstar.xpm
    bigtriangledown.xpm
    bigtriangleup.xpm
    biguplus.xpm
    bigvee.xpm
    bigwedge.xpm
    blacklozenge.xpm
    blacksquare.xpm
    blacktriangle.xpm
    blacktriangledown.xpm
    blacktriangleleft.xpm
    blacktriangleright.xpm
    bot.xpm
    bowtie.xpm
    boxdot.xpm
    boxminus.xpm
    boxplus.xpm
    boxtimes.xpm
    breve.xpm
    bullet.xpm
    bumpeq.xpm
    bumpeq2.xpm
    cap.xpm
    cap2.xpm
    cases.xpm
    cdot.xpm
    cdots.xpm
    centerdot.xpm
    check.xpm
    chi.xpm
    circ.xpm
    circeq.xpm
    circlearrowleft.xpm
    circlearrowright.xpm
    circledS.xpm
    circledast.xpm
    circledcirc.xpm
    circleddash.xpm
    clubsuit.xpm
    complement.xpm
    cong.xpm
    coprod.xpm
    cup.xpm
    cup2.xpm
    curlyeqprec.xpm
    curlyeqsucc.xpm
    curlyvee.xpm
    curlywedge.xpm
    curvearrowleft.xpm
    curvearrowright.xpm
    dagger.xpm
    daleth.xpm
    dashleftarrow.xpm
    dashrightarrow.xpm
    dashv.xpm
    ddagger.xpm
    ddot.xpm
    ddots.xpm
    delim.xpm
    delta.xpm
    delta2.xpm
    diagdown.xpm
    diagup.xpm
    diamond.xpm
    diamondsuit.xpm
    digamma.xpm
    div.xpm
    divideontimes.xpm
    dot.xpm
    doteq.xpm
    doteqdot.xpm
    dotplus.xpm
    dotsint.xpm
    dotsintop.xpm
    doublebarwedge.xpm
    downarrow.xpm
    downarrow2.xpm
    downdownarrows.xpm
    downharpoonleft.xpm
    downharpoonright.xpm
    ell.xpm
    empty.xpm
    emptyset.xpm
    epsilon.xpm
    eqcirc.xpm
    eqslantgtr.xpm
    eqslantless.xpm
    equation.xpm
    equiv.xpm
    eta.xpm
    eth.xpm
    exists.xpm
    fallingdotseq.xpm
    flat.xpm
    font.xpm
    forall.xpm
    frac-square.xpm
    frac.xpm
    frown.xpm
    functions.xpm
    gamma.xpm
    gamma2.xpm
    geq.xpm
    geqq.xpm
    geqslant.xpm
    gg.xpm
    ggg.xpm
    gimel.xpm
    gnapprox.xpm
    gneq.xpm
    gneqq.xpm
    gnsim.xpm
    grave.xpm
    gtrapprox.xpm
    gtrdot.xpm
    gtreqless.xpm
    gtreqqless.xpm
    gtrless.xpm
    gtrsim.xpm
    gvertneqq.xpm
    hat.xpm
    hbar.xpm
    heartsuit.xpm
    hookleftarrow.xpm
    hookrightarrow.xpm
    hphantom.xpm
    hslash.xpm
    iiiint.xpm
    iiiintop.xpm
    iiint.xpm
    iiintop.xpm
    iint.xpm
    iintop.xpm
    imath.xpm
    in.xpm
    infty.xpm
    int.xpm
    intercal.xpm
    intop.xpm
    iota.xpm
    jmath.xpm
    kappa.xpm
    lambda.xpm
    lambda2.xpm
    langle.xpm
    lbrace.xpm
    lbrace_rbrace.xpm
    lbracket.xpm
    lbracket_rbracket.xpm
    lceil.xpm
    lceil_rceil.xpm
    ldots.xpm
    leftarrow.xpm
    leftarrow2.xpm
    leftarrowtail.xpm
    leftharpoondown.xpm
    leftharpoonup.xpm
    leftleftarrows.xpm
    leftrightarrow.xpm
    leftrightarrow2.xpm
    leftrightarrows.xpm
    leftrightharpoons.xpm
    leftrightsquigarrow.xpm
    leftthreetimes.xpm
    leq.xpm
    leqq.xpm
    leqslant.xpm
    lessapprox.xpm
    lessdot.xpm
    lesseqgtr.xpm
    lesseqqgtr.xpm
    lessgtr.xpm
    lesssim.xpm
    lfloor.xpm
    lfloor_rfloor.xpm
    ll.xpm
    llcorner.xpm
    lll.xpm
    lnapprox.xpm
    lneq.xpm
    lneqq.xpm
    lnsim.xpm
    longleftarrow.xpm
    longleftarrow2.xpm
    longleftrightarrow.xpm
    longleftrightarrow2.xpm
    longmapsto.xpm
    longrightarrow.xpm
    longrightarrow2.xpm
    looparrowleft.xpm
    looparrowright.xpm
    lozenge.xpm
    lparen.xpm
    lparen_rparen.xpm
    lrcorner.xpm
    ltimes.xpm
    lvertneqq.xpm
    mapsto.xpm
    mathbb_C.xpm
    mathbb_H.xpm
    mathbb_N.xpm
    mathbb_Q.xpm
    mathbb_R.xpm
    mathbb_Z.xpm
    mathcal_F.xpm
    mathcal_H.xpm
    mathcal_L.xpm
    mathcal_O.xpm
    mathcircumflex.xpm
    mathrm_T.xpm
    matrix.xpm
    measuredangle.xpm
    mho.xpm
    mid.xpm
    models.xpm
    mp.xpm
    mu.xpm
    multimap.xpm
    nabla.xpm
    natural.xpm
    ncong.xpm
    nearrow.xpm
    neg.xpm
    neq.xpm
    nexists.xpm
    ngeq.xpm
    ngeqq.xpm
    ngeqslant.xpm
    ngtr.xpm
    ni.xpm
    nleftarrow.xpm
    nleftarrow2.xpm
    nleftrightarrow.xpm
    nleftrightarrow2.xpm
    nleq.xpm
    nleqq.xpm
    nleqslant.xpm
    nless.xpm
    nmid.xpm
    notin.xpm
    nparallel.xpm
    nprec.xpm
    npreceq.xpm
    nrightarrow.xpm
    nrightarrow2.xpm
    nshortmid.xpm
    nshortparallel.xpm
    nsim.xpm
    nsubseteq.xpm
    nsucc.xpm
    nsucceq.xpm
    nsupseteq.xpm
    nsupseteqq.xpm
    ntriangleleft.xpm
    ntrianglelefteq.xpm
    ntriangleright.xpm
    ntrianglerighteq.xpm
    nu.xpm
    nvdash.xpm
    nvdash2.xpm
    nvdash3.xpm
    nwarrow.xpm
    odot.xpm
    oiint.xpm
    oiintop.xpm
    oint.xpm
    ointclockwise.xpm
    ointclockwiseop.xpm
    ointctrclockwise.xpm
    ointctrclockwiseop.xpm
    ointop.xpm
    omega.xpm
    omega2.xpm
    ominus.xpm
    oplus.xpm
    oslash.xpm
    otimes.xpm
    overbrace.xpm
    overleftarrow.xpm
    overleftrightarrow.xpm
    overline.xpm
    overrightarrow.xpm
    overset.xpm
    parallel.xpm
    partial.xpm
    perp.xpm
    phantom.xpm
    phi.xpm
    phi2.xpm
    pi.xpm
    pi2.xpm
    pitchfork.xpm
    pm.xpm
    prec.xpm
    precapprox.xpm
    preccurlyeq.xpm
    preceq.xpm
    precnapprox.xpm
    precnsim.xpm
    precsim.xpm
    prime.xpm
    prod.xpm
    propto.xpm
    psi.xpm
    psi2.xpm
    rangle.xpm
    rbrace.xpm
    rbracket.xpm
    rceil.xpm
    rfloor.xpm
    rho.xpm
    rightarrow.xpm
    rightarrow2.xpm
    rightarrowtail.xpm
    rightharpoondown.xpm
    rightharpoonup.xpm
    rightleftarrows.xpm
    rightleftharpoons.xpm
    rightrightarrows.xpm
    rightsquigarrow.xpm
    rightthreetimes.xpm
    risingdotseq.xpm
    root.xpm
    rparen.xpm
    rtimes.xpm
    searrow.xpm
    setminus.xpm
    sharp.xpm
    shortmid.xpm
    shortparallel.xpm
    sigma.xpm
    sigma2.xpm
    sim.xpm
    simeq.xpm
    slash.xpm
    smallfrown.xpm
    smallsetminus.xpm
    smallsmile.xpm
    smile.xpm
    space.xpm
    spadesuit.xpm
    sphericalangle.xpm
    sqcap.xpm
    sqcup.xpm
    sqiint.xpm
    sqiintop.xpm
    sqint.xpm
    sqintop.xpm
    sqrt-square.xpm
    sqrt.xpm
    sqsubset.xpm
    sqsubseteq.xpm
    sqsupset.xpm
    sqsupseteq.xpm
    square.xpm
    star.xpm
    style.xbm
    style.xpm
    sub.xpm
    subset.xpm
    subset2.xpm
    subseteq.xpm
    subseteqq.xpm
    subsetneq.xpm
    subsetneqq.xpm
    succ.xpm
    succapprox.xpm
    succcurlyeq.xpm
    succeq.xpm
    succnapprox.xpm
    succnsim.xpm
    succsim.xpm
    sum.xpm
    super.xpm
    supset.xpm
    supset2.xpm
    supseteq.xpm
    supseteqq.xpm
    supsetneq.xpm
    supsetneqq.xpm
    surd.xpm
    swarrow.xpm
    tau.xpm
    textrm_AA.xpm
    textrm_Oe.xpm
    therefore.xpm
    theta.xpm
    theta2.xpm
    thickapprox.xpm
    thicksim.xpm
    tilde.xpm
    times.xpm
    top.xpm
    triangle.xpm
    triangledown.xpm
    triangleleft.xpm
    trianglelefteq.xpm
    triangleq.xpm
    triangleright.xpm
    trianglerighteq.xpm
    twoheadleftarrow.xpm
    twoheadrightarrow.xpm
    ulcorner.xpm
    underbrace.xpm
    underleftarrow.xpm
    underleftrightarrow.xpm
    underline.xpm
    underrightarrow.xpm
    underscore.xpm
    underset.xpm
    uparrow.xpm
    uparrow2.xpm
    updownarrow.xpm
    updownarrow2.xpm
    upharpoonleft.xpm
    upharpoonright.xpm
    uplus.xpm
    upsilon.xpm
    upsilon2.xpm
    upuparrows.xpm
    urcorner.xpm
    varepsilon.xpm
    varkappa.xpm
    varnothing.xpm
    varphi.xpm
    varpi.xpm
    varpropto.xpm
    varrho.xpm
    varsigma.xpm
    varsubsetneq.xpm
    varsubsetneqq.xpm
    varsupsetneq.xpm
    varsupsetneqq.xpm
    vartheta.xpm
    vartriangle.xpm
    vartriangleleft.xpm
    vartriangleright.xpm
    vdash.xpm
    vdash2.xpm
    vdash3.xpm
    vdots.xpm
    vec.xpm
    vee.xpm
    veebar.xpm
    vert.xpm
    vert2.xpm
    vphantom.xpm
    wedge.xpm
    widehat.xpm
    widetilde.xpm
    wp.xpm
    wr.xpm
    xi.xpm
    xi2.xpm
    zeta.xpm
''')


lib_images_math_extra_files = Split('''
    ams_arrows.xbm
    ams_misc.xbm
    ams_nrel.xbm
    ams_ops.xbm
    ams_rel.xbm
    arrows.xbm
    bop.xbm
    brel.xbm
    deco.xbm
    deco.xpm
    delim.xbm
    delim0.xpm
    delim1.xpm
    dots.xbm
    font.xbm
    functions.xpm
    greek.xbm
    misc.xbm
    varsz.xbm
''')


lib_images_attic_extra_files = Split('''
    dialog-show_mathpanel.xpm
''')


lib_tex_files = Split('''
    broadway.cls
    hollywood.cls
    lyxchess.sty
    lyxskak.sty
    revtex.cls
''')


lib_doc_files = Split('''
    Customization.lyx
    DocStyle.lyx
    DummyDocument1.lyx
    DummyDocument2.lyx
    DummyTextDocument.txt
    EmbeddedObjects.lyx
    Extended.lyx
    FAQ.lyx
    Formula-numbering.lyx
    Intro.lyx
    Math.lyx
    LaTeXConfig.lyx.in
    Reference.lyx
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
    ERT.png
    ExternalMaterialQt4.png
    ExtraToolbar.png
    FramedNoteImageQt4.png
    GreyedOutNoteImageQt4.png
    LaTeX.png
    LyXNoteImageQt4.png
    ShadedNoteImageQt4.png
    SpaceMarker.png
    StandardToolbar.png
    ToolbarEnvBox.png
    endnotes.pdf
    escher-lsd.eps
    floatQt4.png
    footnoteQt4.png
    labelQt4.png
    macrobox.png
    macrouse.png
    mbox.png
    mobius.eps
    platypus.eps
    referenceQt4.png
    ViewToolbar.png
    with_fntright.pdf
    without_fntright.pdf
''')


lib_doc_extra_files = Split('''
    ChangeLog
    Makefile.am
    Makefile.depend
    README.Documentation
    depend.py
    doc_toc.py
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
    Extended.lyx
    FAQ.lyx
    Formelnummerierung.lyx
    Intro.lyx
    Math.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_de_clipart_files = Split('''
    ERT.png
    ExternesMaterialQt4.png
    FussnoteQt4.png
    GerahmteNotizQt4.png
    GleitobjektQt4.png
    GrauschriftNotizQt4.png
    KommentarQt4.png
    LyXNotizQt4.png
    Marke.png
    Querverweis.png
    SchattierteNotizQt4.png
    StandardBoxQt4.png
    UnterdokumentQt4.png
''')


lib_doc_da_files = Split('''
    Intro.lyx
''')


lib_doc_es_files = Split('''
    DocumentoPostizo1.lyx
    DocumentoPostizo2.lyx
    DocumentoTextoPostizo.txt
    EmbeddedObjects.lyx
    Extended.lyx
    FAQ.lyx
    Formula-numbering.lyx
    Intro.lyx
    Math.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_es_clipart_files = Split('''
    ComentNotaImagenQt4.png
    CuadroMinipagQt4.png
    DocumentoHijoQt4.png
    GrisNotaImagenQt4.png
    MaterialExternoQt4.png
    NotaEnmarcadaImg.png
    NotaLyXImagenQt4.png
    NotaSombreadaImg.png
    Resumen.pdf
    es_ERT.png
    etiquetaQt4.png
    flotanteQt4.png
    notapieQt4.png
    referenciaQt4.png
''')


lib_doc_eu_files = Split('''
    Customization.lyx
    Extended.lyx
    FAQ.lyx
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
    Extended.lyx
    FAQ.lyx
    Formula-numbering.lyx
    Intro.lyx
    Math.lyx
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_fr_clipart_files = Split('''
    BoxInsetDefaultQt4.png
    ChildDocumentQt4.png
    CommentNoteImageQt4.png
    FramedNoteImageQt4.png
    GreyedOutNoteImageQt4.png
    LyXNoteImageQt4.png
    ShadedNoteImageQt4.png
    floatQt4.png
    footnoteQt4.png
    labelQt4.png
''')


lib_doc_gl_extra_files = Split('''
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


lib_doc_nb_files = Split('''
    Intro.lyx
''')


lib_doc_nl_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_pl_files = Split('''
    Extended.lyx
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
    FAQ.lyx
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_sl_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_sk_files = Split('''
    Tutorial.lyx
    UserGuide.lyx
''')


lib_doc_sv_files = Split('''
    Intro.lyx
    Tutorial.lyx
''')


lib_doc_uk_files = Split('''
    Intro.lyx
''')


lib_doc_uk_clipart_files = Split('''
    FootnoteQT4.png
''')


lib_examples_files = Split('''
    CV-image.eps
    CV-image.png
    Foils.lyx
    ItemizeBullets.lyx
    Literate.lyx
    aa_sample.lyx
    aas_sample.lyx
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
    chess-article.lyx
    chessgame.lyx
    docbook_article.lyx
    europeCV.lyx
    example_lyxified.lyx
    example_raw.lyx
    iecc05.fen
    iecc07.fen
    iecc12.fen
    landslide.lyx
    listerrors.lyx
    modernCV.lyx
    multicol.lyx
    noweb2lyx.lyx
    powerdot-example.lyx
    script_form.lyx
    serial_letter1.lyx
    serial_letter2.lyx
    serial_letter3.lyx
    simplecv.lyx
    splash.lyx
''')


lib_examples_ca_files = Split('''
    ItemizeBullets.lyx
    mathed.lyx
    splash.lyx
''')


lib_examples_cs_files = Split('''
    splash.lyx
''')


lib_examples_da_files = Split('''
    splash.lyx
''')


lib_examples_de_files = Split('''
    Dezimal.lyx
    ItemizeBullets.lyx
    Lebenslauf.lyx
    beispiel_gelyxt.lyx
    beispiel_roh.lyx
    multicol.lyx
    serienbrief1.lyx
    serienbrief2.lyx
    serienbrief3.lyx
    splash.lyx
''')


lib_examples_fa_files = Split('''
    splash.lyx
''')


lib_examples_fr_files = Split('''
    AlignementDecimal.lyx
    Foils.lyx
    ListesPuces.lyx
    exemple_brut.lyx
    exemple_lyxifie.lyx
    multicol.lyx
    simplecv.lyx
    splash.lyx
''')


lib_examples_es_files = Split('''
    ItemizeBullets.lyx
    ejemplo_con_lyx.lyx
    ejemplo_sin_lyx.lyx
    splash.lyx
''')


lib_examples_eu_files = Split('''
    adibide_gordina.lyx
    adibide_lyx-atua.lyx
    splash.lyx
''')


lib_examples_gl_extra_files = Split('''
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


lib_examples_it_files = Split('''
    ItemizeBullets.lyx
    splash.lyx
''')


lib_examples_ja_files = Split('''
    splash.lyx
''')


lib_examples_nl_files = Split('''
    multicol.lyx
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


lib_examples_sl_files = Split('''
    primer_lyxan.lyx
    primer_surov.lyx
    splash.lyx
''')


lib_lyx2lyx_files = Split('''
    LyX.py
    generate_encoding_info.py
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
    parser_tools.py
    profiling.py
    test_parser_tools.py
''')


lib_lyx2lyx_extra_files = Split('''
    ChangeLog
    Makefile.am
    lyx2lyx_version.py.in
''')


lib_layouts_files = Split('''
    aa.layout
    aapaper.layout
    aastex.layout
    agu-dtd.layout
    agums.layout
    amsart-plain.layout
    amsart-seq.layout
    amsart.layout
    amsbook.layout
    apa.layout
    arab-article.layout
    armenian-article.layout
    article.layout
    article-beamer.layout
    beamer.layout
    book.layout
    broadway.layout
    chess.layout
    cl2emult.layout
    dinbrief.layout
    docbook-book.layout
    docbook-chapter.layout
    docbook-section.layout
    docbook.layout
    dtk.layout
    egs.layout
    elsart.layout
    entcs.layout
    europecv.layout
    extarticle.layout
    extbook.layout
    extletter.layout
    extreport.layout
    foils.layout
    g-brief-de.layout
    g-brief-en.layout
    g-brief2.layout
    heb-article.layout
    heb-letter.layout
    hollywood.layout
    IEEEtran.layout
    ijmpc.layout
    ijmpd.layout
    iopart.layout
    isprs.layout
    jgrga.layout
    jss.layout
    kluwer.layout
    latex8.layout
    letter.layout
    literate-article.layout
    literate-book.layout
    literate-report.layout
    llncs.layout
    ltugboat.layout
    manpage.layout
    memoir.layout
    moderncv.layout
    mwart.layout
    mwbk.layout
    mwrep.layout
    paper.layout
    powerdot.layout
    report.layout
    revtex.layout
    revtex4.layout
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
    slides.layout
    spie.layout
    svglobal.layout
    svglobal3.layout
    svjog.layout
    svprobth.layout
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
    amsmaths-plain.inc
    amsmaths-seq.inc
    amsmaths.inc
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
    stdclass.inc
    stdcounters.inc
    stdfloats.inc
    stdlayouts.inc
    stdletter.inc
    stdlists.inc
    stdsections.inc
    stdstarsections.inc
    stdstruct.inc
    stdtitle.inc
    svjour.inc
''')


lib_scripts_files = Split('''
    TeXFiles.py
    clean_dvi.py
    convertDefault.py
    csv2lyx.py
    date.py
    ext_copy.py
    fen2ascii.py
    fig2pdftex.py
    fig2pstex.py
    fig_copy.py
    layout2layout.py
    legacy_lyxpreview2ppm.py
    listerrors
    lyxpreview2bitmap.py
    lyxpreview_tools.py
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
    xemacs.bind
''')


lib_bind_fi_files = Split('''
    menus.bind
''')


lib_bind_sv_files = Split('''
    menus.bind
''')


lib_bind_pt_files = Split('''
    menus.bind
''')


lib_bind_de_files = Split('''
    menus.bind
''')


boost_extra_files = Split('''
    ChangeLog
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


boost_libs_filesystem_extra_files = Split('''
    Makefile.am
    filesystem.vcproj
''')


boost_libs_filesystem_src_files = Split('''
    exception.cpp
    operations.cpp
    path.cpp
    portability.cpp
''')


boost_libs_filesystem_src_extra_files = Split('''
    Makefile.am
    pch.h
''')


boost_libs_iostreams_extra_files = Split('''
    Makefile.am
''')


boost_libs_iostreams_src_files = Split('''
    file_descriptor.cpp
    mapped_file.cpp
    zlib.cpp
''')


boost_libs_iostreams_src_extra_files = Split('''
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
