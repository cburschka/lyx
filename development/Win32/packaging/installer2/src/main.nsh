#-------------------------------------------------------------
# - - - - - - - - - - - - Preamble - - - - - - - - - - - - - -
#-------------------------------------------------------------

  # Do a Cyclic Redundancy Check to make sure the installer
  # was not corrupted by the download.
  CRCCheck force
  # Make the installer as small as possible
  SetCompressor /SOLID lzma
  # Make it a Unicode installer
  Unicode true
  # Enable support for high DPI resolution
  ManifestDPIAware true

  Name "LyX ${APP_VERSION}" # Name of the Program to be displayed during installation
  BrandingText " " # Remove branding text ('Nullsoft Install System vX.XX')

  # ===== Variables =====

    Var LatexPath # Used to store path to "latex.exe"
    Var StartMenuFolder # Used to store the start menu folder
    Var pathPrefix

    # FIXME Variables needed due to "old" set of language strings, delete
    Var LaTeXInstalled
    Var LaTeXName
    Var OldVersionNumber
    Var AppPre
    Var AppSuff
    # R3

    # FIXME Compile time constants needed due to "old" set of language strings, delete
    !define APP_SERIES_KEY2 ${APP_VERSION}
    !define APP_DIR_USERDATA "LyX${VERSION_MAJOR}.${VERSION_MINOR}"

  # ========= Multiuser settings =========

    !define MULTIUSER_MUI # Multiuser uses modern UI 2
    !define MULTIUSER_EXECUTIONLEVEL Highest # Can install with any privileges (admin, user)
    !if ${APP_ARCHITECTURE} = 64
      !define MULTIUSER_USE_PROGRAMFILES64 # Install 64 bit program in "Program files" instead of "Program files (x86)"
    !endif

    !define MULTIUSER_INSTALLMODE_COMMANDLINE # Allow setting installmode in commandline (/AllUsers or /CurrentUser)

    # Set default install mode based on a non empty registry entry (if exists in HKLM, then all users, if HKCU then current user)
    !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "SOFTWARE\LyX"
    !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "InstDir${VERSION_MAJOR}${VERSION_MINOR}"

    !define MULTIUSER_INSTALLMODE_INSTDIR "LyX ${VERSION_MAJOR}.${VERSION_MINOR}" # Default install dir if not over-installing

    # Retrieve default install location from following registry entry (HKCU/HKLM  is chosen automatically)
    !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "SOFTWARE\LyX"
    !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstDir${VERSION_MAJOR}${VERSION_MINOR}"

    !include MultiUser.nsh # Header file: Installer configuration for multi-user Windows environments
    # When MULTIUSER_INSTALLMODE_COMMANDLINE is defined, MultiUser.nsh defines ${StrStr} and ${UnStrStr} functions

  # ============= Modern UI 2 general settings ===============

    #!include MUI2.nsh # included in MultiUser # Header file for creating modern looking installer
    !define MUI_ABORTWARNING # Show a message box with a warning when the user wants to close the installer.
    !define MUI_ICON "..\installer\icons\lyx.ico" # FIXME path # The icon for the installer.
    !define MUI_UNICON "..\installer\icons\lyx.ico" # FIXME path # The icon for the uninstaller
    !define MUI_HEADERIMAGE # Display an image on the header of the page.
    !define MUI_HEADERIMAGE_BITMAP "..\installer\graphics\header.bmp" # FIXME path # Header image source
    !define MUI_HEADERIMAGE_RIGHT # Display header image on the right and not left
    !define MUI_WELCOMEFINISHPAGE_BITMAP "..\installer\graphics\wizard.bmp" # FIXME path # Source for image on welcome and finish page of the installer
    !define MUI_UNWELCOMEFINISHPAGE_BITMAP "..\installer\graphics\wizard.bmp" # FIXME path # Source for image on welcome and finish page of the uninstaller

  # ======== Pages definitions ==========

    # Installer

    !define MUI_WELCOMEPAGE_TEXT $(TEXT_WELCOME) # Use custom welcome text
    !insertmacro MUI_PAGE_WELCOME # Welcome page

    !insertmacro MUI_PAGE_LICENSE "..\installer\license.rtf" # FIXME path # License page

    !define MUI_PAGE_CUSTOMFUNCTION_PRE PrepareShellCTX # Prepare shell context before the multiuser page is created, so that the installer searches for default install folder in the right context (x64/x32)
    !insertmacro MULTIUSER_PAGE_INSTALLMODE # Multiuser page

    !define MUI_PAGE_CUSTOMFUNCTION_LEAVE VerifyInstDir # Custom leave-function for directory page to verify installdir
    !insertmacro MUI_PAGE_DIRECTORY # Directory page (where to install)

    !define MUI_PAGE_CUSTOMFUNCTION_PRE DefaultDesktopFileAssoc # Custom pre-function for components page to search for already present dictionaries (in dictionaries.nsh)
    !insertmacro MUI_PAGE_COMPONENTS # Components page (what to install)

    # Default startmenu folder if registry contains no information from a previous installment, same as the default install dir
    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${MULTIUSER_INSTALLMODE_INSTDIR}"
    # Define registry entry to store the startmenu folder, also reads this entry in as default startmenu folder in case of over-installing
    !define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX" # Context dependent on what the user chooses on multiuser page
    !define MUI_STARTMENUPAGE_REGISTRY_KEY "SOFTWARE\LyX"
    !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "SMDir${VERSION_MAJOR}${VERSION_MINOR}"
    !define MUI_PAGE_CUSTOMFUNCTION_PRE PrepareShellCTX # Prepare shell context before the startmenu page is created, so that the installer searches for default startmenu folder in the right context
    !define MUI_PAGE_CUSTOMFUNCTION_SHOW RetrieveSMState # Custom show-function for startmenu page to disable startmenu dir creation in case of over-installment where previously disabled
    !insertmacro MUI_PAGE_STARTMENU startmenuPage $StartMenuFolder # Page to choose start menu folder entry, the folder is stored in VARIABLE

    # Latexfolder page
    !include "src\CustomPages\LaTeXFolder.nsdinc"
    Page custom LatexFolder_Show LatexFolder_Leave

    !insertmacro MUI_PAGE_INSTFILES # Progress page

    !define MUI_FINISHPAGE_SHOWREADME # Showreadme-checkbox appears
    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED # Showreadme-checkbox disabled by default
    !define MUI_FINISHPAGE_SHOWREADME_FUNCTION StartLyX # Custom function for the showreadme-checkbox (start latex instead of show readme)
    !define MUI_FINISHPAGE_SHOWREADME_TEXT $(FinishPageRun) # Custom text for the label on the showreadme-checkbox
    !define MUI_FINISHPAGE_LINK $(TEXT_FINISH_WEBSITE) # Text for the link on the bottom of finish page
    !define MUI_FINISHPAGE_LINK_LOCATION "https://www.lyx.org/" # Website opens by clicking on the link on finish page
    !insertmacro MUI_PAGE_FINISH # Finish page

    # Uninstaller

    !insertmacro MUI_UNPAGE_WELCOME # Welcome page
    !insertmacro MUI_UNPAGE_CONFIRM # Confirmation page

    !define MUI_PAGE_CUSTOMFUNCTION_PRE un.ComponentsPre # Custom function for components page to prepare strings for the description of a section
    !insertmacro MUI_UNPAGE_COMPONENTS # Components page

    !insertmacro MUI_UNPAGE_INSTFILES # Progress page
    !insertmacro MUI_UNPAGE_FINISH # Finish page

  # ======= Other includes =======

    !include "src\lang\TranslatedLanguages.nsh" # languages strings

    #!include LogicLib.nsh # included in MUI2 # Allows using logic commands (such as ${If}..${Else}..${EndIf})
    #!include LangFile.nsh # included in MUI2 # Header file to create language files that can be included with a single command.
    !include x64.nsh # Header file to check if target system is 64 bit or not with ${RunningX64}, also defines ${DisableX64FSRedirection} and ${EnableX64FSRedirection}
    !include NSISList.nsh # Header file to create and work with lists in NSIS (plugin)

    #!include StrFunc.nsh # included in MultiUser # Provides some additional string functions
    ${StrRep} # Define this function from StrFunc.nsh for further use
    ${StrStrAdv} # Define this function from StrFunc.nsh for further use
    ${UnStrStrAdv} # Define this function from StrFunc.nsh for further use
    ${UnStrRep} # Define this function from StrFunc.nsh for further use

  # ========= Installer Attributes ===========

    VIProductVersion "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}.${VERSION_BUILD}" # Must be X.X.X.X format
    VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "LyX"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "LyX ${APP_VERSION} installer"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APP_VERSION}"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "LyX - The Document Processor"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "LyX is Copyright © 1995 by Matthias Ettrich, 1995-${COPYRIGHT_YEAR} by the LyX Team"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "LyX Team"
    VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""

#-------------------------------------------------------------
# - - - - - - - - - - Helper Functions - - - - - - - - - - - -
#-------------------------------------------------------------

  # Forward declarations
  
  !macro SearchAllRegistry resultVar resultStateIdx resultRegIdx subKey searchFor startStateIdx startRegIdx
    Push ${subKey}
    Push ${searchFor}
    Push ${startStateIdx}
    Push ${startRegIdx}
    Call SearchAllRegistry
    Pop ${resultStateIdx}
    Pop ${resultRegIdx}
    Pop ${resultVar}
  !macroend
  
  !define SearchAllRegistry "!insertmacro SearchAllRegistry"
  
  !macro SearchRegistry resultVar resultRegIdx subKey searchFor startRegIdx
    Push ${subKey}
    Push ${searchFor}
    Push ${startRegIdx}
    Call SearchRegistry
    Pop ${resultRegIdx}
    Pop ${resultVar}
  !macroend
  
  !define SearchRegistry "!insertmacro SearchRegistry"
  
  # Functions
  
  Function SearchAllRegistry
    Exch $9 # startRegIdx
    Exch
    Exch $8 # startStateIdx
    Exch 2
    Exch $7 # searchFor
    Exch 3
    Exch $6 # subKey
    Push $5
  
    ${Do}
      ${Select} $8
        ${Case} 0
          SetShellVarContext all
          ${If} ${RunningX64}
            SetRegView 64
          ${Else}
            IntOp $8 $8 + 1
          ${EndIf}
        ${Case} 1
          SetShellVarContext all
          ${If} ${RunningX64}
            SetRegView 32
          ${EndIf}
        ${Case} 2
          SetShellVarContext current
          ${If} ${RunningX64}
            SetRegView 64
          ${Else}
            IntOp $8 $8 + 1
          ${EndIf}
        ${Case} 3
          SetShellVarContext current
          ${If} ${RunningX64}
            SetRegView 32
          ${EndIf}
      ${EndSelect}
      ${SearchRegistry} $5 $9 $6 $7 $9
      ${If} $5 != ""
        ${Break}
      ${EndIf}
      IntOp $8 $8 + 1
      StrCpy $9 0
    ${LoopUntil} $8 >= 4
  
    Exch $5 # resultVar
    Exch 4
    Pop $7
    Pop $6
    Exch $9 # resultRegIdx
    Exch
    Exch $8 # resultStateIdx
  FunctionEnd
  
  Function SearchRegistry
    Exch $9 # startRegIdx
    Exch
    Exch $8 # searchFor
    Exch 2
    Exch $7 #subKey
    Push $6
    Push $5
  
    ${Do}
      EnumRegKey $6 SHCTX $7 $9
      ${StrStr} $5 $6 $8
      IntOp $9 $9 + 1
      ${If} $5 != ""
        ${Break}
      ${EndIf}
    ${LoopUntil} $6 == ""
  
    Exch $5 # resultVar
    Exch 4
    Pop $8
    Pop $6
    Pop $7
    Exch $9 # resultRegIdx
  FunctionEnd
  
  !macro PrepareShellCTX
    ${if} $MultiUser.InstallMode == "CurrentUser"
      SetShellVarContext current
    ${Else}
      SetShellVarContext all
    ${EndIf}
    ${If} ${RunningX64}
      SetRegView ${APP_ARCHITECTURE}
    ${EndIf}
  !macroend

  Function PrepareShellCTX
    !insertmacro PrepareShellCTX
  FunctionEnd

  Function un.PrepareShellCTX
    !insertmacro PrepareShellCTX
  FunctionEnd

#-------------------------------------------------------------
# - - - - - - - - - - - - Sections - - - - - - - - - - - - - -
#-------------------------------------------------------------

# ============ Installer Page Functions ============

Function .onInit # Callback function, called at the very beginning, when user double clicks on the installer

  !if ${APP_ARCHITECTURE} = 64 # If 64 bit installer
    # Abort if not 64 bit Windows
    ${IfNot} ${RunningX64}
      MessageBox MB_OK|MB_ICONSTOP "Cannot install a 64 bit application on 32 bit Windows, please use the 32 bit installer" /SD IDOK
      Quit
    ${EndIf}
  !EndIf

  # Check Windows version
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
  ${if} $0 S<= "6.0" # Atleast Win7 is needed
    MessageBox MB_OK|MB_ICONSTOP "LyX ${APP_VERSION} requires Windows 7 or newer." /SD IDOK
    Quit
  ${endif}

  Call PrepareShellCTX # MULTIUSER_INIT should search in the right registry view
  !insertmacro MULTIUSER_INIT # Verify multiuser privileges

  # Check if the same version of LyX is installed anywhere (admin/user, x64/x32) on the computer
  StrCpy $1 0
  StrCpy $2 0
  ${DoUntil} $1 >= 4
    ${SearchAllRegistry} $0 $1 $2 "SOFTWARE\LyX" ${APP_VERSION_DOTLESS} $1 $2 # Helper Function from above
    ${If} $0 == ${APP_VERSION_DOTLESS}
      ReadRegStr $0 SHCTX "SOFTWARE\LYX\$0" ""
      ${If} ${FileExists} "$0\Uninstall-LyX.exe"
        ${IfCmd} MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION "$(AlreadyInstalled)" /SD IDNO IDNO ${||} Quit ${|}
      ${Else}
        DeleteRegKey SHCTX "SOFTWARE\LYX\$0"
      ${EndIf}
    ${EndIf}
  ${Loop} 

  # Check if a newer version of the same LyX series is installed anywhere (admin/user, x64/x32) on the computer
  StrCpy $1 0
  StrCpy $2 0
  ${DoUntil} $1 >= 4
    ${SearchAllRegistry} $R9 $1 $2 "SOFTWARE\LyX" "${VERSION_MAJOR}${VERSION_MINOR}" $1 $2 # Helper Function from above
    StrCpy $OldVersionNumber $R9 # FIXME remove
    ${If} $R9 S> ${APP_VERSION_DOTLESS} # $R9 is used in $(NewerInstalled)
      ReadRegStr $0 SHCTX "SOFTWARE\LYX\$0" ""
      ${If} ${FileExists} "$0\Uninstall-LyX.exe"
        MessageBox MB_OK|MB_ICONSTOP "$(NewerInstalled)" /SD IDOK
        Quit
      ${Else}
        DeleteRegKey SHCTX "SOFTWARE\LYX\$0"
      ${EndIf}
    ${EndIf}
  ${Loop}
FunctionEnd

Function CheckIfRunning # Check that LyX in $INSTDIR is not currently running, called from Function VerifyInstDir and Section -CheckSilent (if silentinstall)
  ${If} ${RunningX64}
    ${DisableX64FSRedirection} # We need the following process to be 64 bit on 64 bit system
  ${EndIf}
  nsExec::ExecToStack "powershell (Get-Process LyX).Path"
  Pop $0 # Exit code
  Pop $0 # Result string
  ${If} ${RunningX64}
    ${EnableX64FSRedirection} # Need to be anabled asap or installer might crash
  ${EndIf}
  ${StrStr} $0 $0 "$INSTDIR\bin\LyX.exe"
  ${If} $0 != ""
    MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)" /SD IDOK
    Abort # Abort leaving the page (when called from the page callback) / Abort install (when called from the section)
  ${EndIf}
FunctionEnd

Function VerifyInstDir # Custom Function, called when leaving directory page
  # if the $INSTDIR does not contain "LyX" we must add a subfolder to avoid that LyX will e.g.
  # be installed directly to "C:\Program Files" - the uninstaller will then delete the whole
  # "C:\Program Files" directory
  ${StrStr} $0 $INSTDIR LyX
  ${If} $0 == ""
    StrCpy $INSTDIR "$INSTDIR\${MULTIUSER_INSTALLMODE_INSTDIR}" # Append e.g. "LyX 2.3" to $INSTDIR
    ${NSD_SetText} $mui.DirectoryPage.Directory $INSTDIR # Refresh Textbox
    Abort # Abort leaving the page
  ${EndIf}

  Call CheckIfRunning
FunctionEnd

Function RetrieveSMState # Custom function, called after the Startmenu page has been created
  ${IfNot} ${FileExists} "$INSTDIR\Uninstall-LyX.exe"
    Return # Not over-installing
  ${EndIf}

  Call PrepareShellCTX # Helper function from above
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "SMDir${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == ""
    ${If} ${Silent}
      StrCpy $StartMenuFolder ">$StartMenuFolder"
    ${Else}
      GetDlgItem $0 $mui.StartMenuPage 1005 # Get the HWND of the checkbox, MUI2 does not provide variable for that
      SendMessage $0 ${BM_CLICK} 0 0 # NSIS delivers no other possibility here other than to send a click event to the checkbox
    ${EndIf}
  ${EndIf}
FunctionEnd

Function LatexFolder_Show # Custom Function, called before showing the LaTeXFolder page to create it
  Call FindLatex # detect latex  
  Pop $0 # Result string from FindLatex
  # $0 should contain the path to latex.exe, it is used in the fnc_LaTeXFolder_Create
  ${StrStr} $1 $0 "\latex.exe"
  ${If} $1 != "" # If latex was actually found
    ${StrRep} $0 $0 $1 ""
    # $R9 is used in $(EnterLaTeXFolder) on the page
    ${StrStr} $1 $0 "\miktex\bin"
    ${If} $1 != ""
      StrCpy $R9 "MiKTeX"
    ${Else}
      StrCpy $R9 "TeXLive"
    ${EndIf}
    StrCpy $LaTeXName $R9 # FIXME remove
    StrCpy $1 $(EnterLaTeXFolder) # $1 is used on the page
  ${Else}
    StrCpy $1 $(EnterLaTeXFolderNone)
  ${EndIf}
  ${StrRep} $1 $1 "\r\n" "$\r$\n" # FIXME remove

  Call fnc_LaTeXFolder_Show # In LaTeXFolder.nsdinc, defined by NSIS Dialog Designer
FunctionEnd

Function FindLatex # Searches latex.exe, called from pre and leave functions of latexfolder page
  # Find "LaTeX Path" value in registry written by a previous installment of the same LyX series, prioritized, because could be user preference
  Call PrepareShellCTX
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  StrCpy $1 0
  StrCpy $2 0
  ${DoUntil} $2 > 1
    ${SearchRegistry} $0 $1 "SOFTWARE\LyX" "${VERSION_MAJOR}${VERSION_MINOR}" $1
    ${If} $0 != ""
      ReadRegStr $0 SHCTX "SOFTWARE\LyX\$0" "LaTeX Path"
      StrCpy $0 "$0\latex.exe"
      ${If} ${FileExists} $0
        Push $0
        Return
      ${EndIf}
    ${ElseIf} ${RunningX64}
      IntOp $2 $2 + 1
      SetRegView 32
    ${Else}
      ${Break}
    ${EndIf}
  ${Loop}

  nsExec::ExecToStack "where latex.exe" # Ask CMD to find "latex.exe" for us, works if latexpath is added to PATH
  Pop $0 # Exit code: 0 = success, 1 = fail
  ${If} $0 = 0
    Return
  ${EndIf}
  Pop $0 # If search failed, this is garbage, otherwise path to latex

  # Search the registry for TexLive
  StrCpy $1 0
  StrCpy $2 0
  ${DoUntil} $1 >= 4
    ${SearchAllRegistry} $0 $1 $2 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" "TeXLive" $1 $2 # Helper Function from above
    ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$0" "UninstallString"
    ${StrRep} $0 $0 '"' ""
    ${StrRep} $0 $0 "tlpkg\installer\uninst.bat" "bin\win32\latex.exe" # TeXLive is not 64 bit yet
    ${If} ${FileExists} $0
      Push $0
      Return
    ${EndIf}
  ${Loop}

  # Search the registry for MiKTex  
  StrCpy $1 0
  StrCpy $2 0
  ${DoUntil} $1 >= 4
    ${SearchAllRegistry} $0 $1 $2 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" "MiKTeX" $1 $2 # Helper Function from above
    ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$0" "InstallLocation"
    StrCpy $0 "$0\miktex\bin\latex.exe"
    ${If} ${FileExists} $0
      Push $0
      Return
    ${EndIf}
    ${If} ${RunningX64} # MiKTeX can also be x64
      ${StrRep} $0 $0 "latex.exe" "x64\latex.exe"
      ${If} ${FileExists} $0
        Push $0
        Return
      ${EndIf}
    ${EndIf}
  ${Loop}

  # No latex was found
  Push "C:\"

FunctionEnd

Function LatexFolder_Leave # Custom function, called when trying to leave LatexFolder page to verify that "latex.exe" really exists in the given path
  ${NSD_GetText} $hCtl_LaTeXFolder_DirRequest1_Txt $LatexPath
  ${NSD_GetState} $hCtl_LaTeXFolder_CheckBox1 $0

  ${If} $0 = ${BST_CHECKED}
    StrCpy $LatexPath ""
  ${ElseIfNot} ${FileExists} "$LatexPath\latex.exe"
    MessageBox MB_OK $(InvalidLaTeXFolder)
    Call FindLatex # Search for latex again
    Pop $0 # Result string from FindLatex
    ${StrStrAdv} $0 $0 "\latex.exe" ">" "<" "0" "0" "0" # Get everything before "\latex.exe"
    ${NSD_SetText} $hCtl_LaTeXFolder_DirRequest1_Txt $0 # Refresh textbox
    Abort # Abort leaving the page
  ${EndIf}
FunctionEnd

# ========= Installer Sections =========

Section -CheckSilent # This section checks if it's a silent install and calls needed callback- and custom-pagefunctions, which are not called in this case
  ${IfNot} ${Silent}
    Return
  ${EndIf}

  # .onInit is called

  Call CheckIfRunning

  Call RetrieveSMState

  Call FindLatex # Search for latex
  Pop $0 # Result string from FindLatex
  ${StrStrAdv} $0 $0 "\latex.exe" ">" "<" "0" "0" "0" # Get everything before "\latex.exe"
  ${If} ${FileExists} "$0\latex.exe"
    StrCpy $LatexPath $0
  ${Else}
    MessageBox MB_OK $(InvalidLaTeXFolder)
    Quit
  ${EndIf}

  Call DefaultDesktopFileAssoc
SectionEnd

Section -OverInstallReg # If over-installing, we need to delete the registry keys written by the previous installment
  Call PrepareShellCTX # Helper function from above
  StrCpy $1 0
  ${Do}
    EnumRegKey $2 SHCTX "SOFTWARE\LyX" $1
    ${StrStr} $3 $2 "${VERSION_MAJOR}${VERSION_MINOR}"
    ReadRegStr $0 SHCTX "SOFTWARE\LyX\$3" ""
    ${If} $0 == $INSTDIR
      DeleteRegKey SHCTX "SOFTWARE\LyX\$3"
      DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX$3"
    ${EndIf}
    IntOp $1 $1 + 1
  ${LoopUntil} $2 == ""
SectionEnd

Section -ProgramFiles

  # Install the core LyX files
  
  # Initializes the plug-ins dir ($PLUGINSDIR) if not already initialized.
  # $PLUGINSDIR is automatically deleted when the installer exits.
  InitPluginsDir
  
  # Binaries
  SetOutPath "$INSTDIR\bin"
  File "${FILES_LYX}\bin\LyX.exe"
  File "${FILES_LYX}\bin\tex2lyx.exe"
  # Visual C++ redistributable (if native build)
  File "${FILES_DEPS}\bin\concrt140.dll"
  File "${FILES_DEPS}\bin\msvcp140.dll"
  File "${FILES_DEPS}\bin\vcamp140.dll"
  File "${FILES_DEPS}\bin\vccorlib140.dll"
  File "${FILES_DEPS}\bin\vcomp140.dll"
  File "${FILES_DEPS}\bin\vcruntime140.dll"
  !if ${APP_ARCHITECTURE} = 64
    File "${FILES_DEPS}\bin\vcruntime140_1.dll"
  !endif
  # MinGW redistributable (if cross platform with MinGW)
  #File "${FILES_DEPS}\bin\iconv.dll"
  #File "${FILES_DEPS}\bin\libbz2-1.dll"
  #File "${FILES_DEPS}\bin\libfreetype-6.dll"
  #File "${FILES_DEPS}\bin\libgcc_s_sjlj-1.dll"
  #File "${FILES_DEPS}\bin\libglib-2.0-0.dll"
  #File "${FILES_DEPS}\bin\libharfbuzz-0.dll"
  #File "${FILES_DEPS}\bin\libintl-8.dll"
  #File "${FILES_DEPS}\bin\libjpeg-62.dll"
  #File "${FILES_DEPS}\bin\libpcre-1.dll"
  #File "${FILES_DEPS}\bin\libpcre2-16-0.dll"
  #File "${FILES_DEPS}\bin\libpng16-16.dll"
  #File "${FILES_DEPS}\bin\libstdc++-6.dll"
  #File "${FILES_DEPS}\bin\libtiff-5.dll"
  #File "${FILES_DEPS}\bin\libwinpthread-1.dll"
  #File "${FILES_DEPS}\bin\libwebp-7.dll"
  #File "${FILES_DEPS}\bin\libwebpdemux-2.dll"
  #File "${FILES_DEPS}\bin\zlib1.dll"
  # Netpbm
  File "${FILES_DEPS}\bin\libnetpbm10.dll"
  File "${FILES_DEPS}\bin\pnmcrop.exe"
  # Rsvg
  File "${FILES_DEPS}\bin\rsvg-convert.exe"
  # Pdfview
  File "${FILES_DEPS}\bin\pdfview.exe"
  # Qt libraries
  File "${FILES_QT}\bin\Qt5Concurrent.dll"
  File "${FILES_QT}\bin\Qt5Core.dll"
  File "${FILES_QT}\bin\Qt5Gui.dll"
  File "${FILES_QT}\bin\Qt5Network.dll"
  File "${FILES_QT}\bin\Qt5OpenGL.dll"
  File "${FILES_QT}\bin\Qt5PrintSupport.dll"
  File "${FILES_QT}\bin\Qt5Svg.dll"
  File "${FILES_QT}\bin\Qt5Widgets.dll"
  File "${FILES_QT}\bin\Qt5WinExtras.dll"

  # Qt plugin DLLs
  SetOutPath "$INSTDIR\bin\imageformats"
  File "${FILES_QT}\bin\imageformats\qgif.dll"
  File "${FILES_QT}\bin\imageformats\qico.dll"
  File "${FILES_QT}\bin\imageformats\qjpeg.dll"
  File "${FILES_QT}\bin\imageformats\qsvg.dll"
  File "${FILES_QT}\bin\imageformats\qicns.dll"
  File "${FILES_QT}\bin\imageformats\qtiff.dll"
  File "${FILES_QT}\bin\imageformats\qwbmp.dll"
  File "${FILES_QT}\bin\imageformats\qwebp.dll"

  SetOutPath "$INSTDIR\bin\iconengines"
  File "${FILES_QT}\bin\iconengines\qsvgicon.dll"

  SetOutPath "$INSTDIR\bin\platforms"
  File "${FILES_QT}\bin\platforms\qminimal.dll"
  File "${FILES_QT}\bin\platforms\qwindows.dll"

  SetOutPath "$INSTDIR\bin\styles"
  File "${FILES_QT}\bin\styles\qwindowsvistastyle.dll"

  # Resources
  SetOutPath "$INSTDIR\Resources"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources\"
  File /r "${FILES_DEPS}\Resources\" # Already present files will be skiped

  # Python
  SetOutPath "$INSTDIR\Python"
  # recursively copy all files under Python
  File /r "${FILES_DEPS}\Python\"

  # Components of ImageMagick
  SetOutPath "$INSTDIR\imagemagick"
  File /r "${FILES_DEPS}\imagemagick\"

  # Components of Ghostscript
  SetOutPath "$INSTDIR\ghostscript"
  File /r "${FILES_DEPS}\ghostscript\"

  # Install unoconv
  SetOutPath "$INSTDIR\Python\Lib"
  File "${FILES_DEPS}\bin\unoconv.py"
  
  WriteUninstaller "$INSTDIR\Uninstall-LyX.exe"
SectionEnd

Section -TexRessources # Section installs cls files from Ressources\tex and updates latex filebase
  # First make sure that latex was found in LatexFolder_Leave function
  ${If} $LatexPath == ""
    Return
  ${EndIf}

  # Miktex or Texlive?
  ${StrStr} $0 $LatexPath "\miktex\bin"
  ${If} $0 == "" # TexLive
    ${StrRep} $0 $LatexPath "\bin\win32" "\texmf-dist\tex\latex\lyx"
    ${If} ${FileExists} "$0\broadway.cls"
      Return
    ${EndIf}
    CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" $0
    ExecWait '$LatexPath\texhash' # Update package file list

  ${Else} # Miktex
    ${StrRep} $0 $LatexPath $0 "\tex\latex\lyx"
    ${If} ${FileExists} "$0\broadway.cls"
      Return
    ${EndIf}
    CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" $0 # First try to put into Miktex install dir

    ${IfNot} ${FileExists} "$0\broadway.cls" # Check if copying succeeded
      # Didn't suceed to put into install dir, put into users dir
      nsExec::ExecToStack "$LatexPath\mpm.exe --version" # get the Miktex version
      Pop $0 # Exit code
      Pop $0 # Result
      ${StrStrAdv} $0 $0 "MiKTeX Package Manager " ">" ">" "0" "0" "0" # $0 contains "2.9.7420 ......" now (everything after "MiKTeX Package Manager ")
      ${StrStrAdv} $0 $0 "." ">" "<" "0" "1" "0" # $0 contains "2.9" now (everything before the second dot)
      SetShellVarContext current
      ${If} ${FileExists} "$APPDATA\MiKTeX\$0\tex\latex\lyx\broadway.cls" # UserInstall-dir can not be changed in Miktex currently, so this should work
        Return
      ${EndIf}
      CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" "$APPDATA\MiKTeX\$0\tex\latex\lyx" # Put into user install dir
    ${EndIf}

    # Update package file list
    nsExec::ExecToLog "$LatexPath\initexmf.exe --admin --update-fndb" # Try as admin
    Pop $0 # Exit code
    ${If} $0 <> 0 # Not successful
      nsExec::ExecToLog "$LatexPath\initexmf.exe --update-fndb" # Do as user
      Pop $0
    ${EndIf}
  ${EndIf}
SectionEnd

Section -CompilePython
  # Compile all Python files to byte-code
  # The user using the scripts may not have write access
  FileOpen $R0 "$INSTDIR\compilepy.py" w
  FileWrite $R0 "import compileall$\r$\n"
  FileWrite $R0 "compileall.compile_dir('$INSTDIR\python\Lib')$\r$\n"
  FileWrite $R0 "compileall.compile_dir('$INSTDIR\Resources')$\r$\n"
  FileClose $R0
  DetailPrint $(TEXT_CONFIGURE_PYTHON)
  nsExec::ExecToLog '"$INSTDIR\python\python.exe" "$INSTDIR\compilepy.py"'
  Pop $0 # Exit code
  Delete "$INSTDIR\compilepy.py"
SectionEnd

Section -LyxrcDist
  StrCpy $pathPrefix "$LatexPath;$$LyXDir\bin;$$LyXDir\Python;$$LyXDir\Python\Lib;$$LyXDir\imagemagick;$$LyXDir\ghostscript\bin"

  # Find additional software and add their install locations to the path_prefix
  # This list contains all software, which write the string value "InstallLocation" in their corresponding keys in "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
  # To add additional software matching this requirement, just append it to the string after "${List.Append} editors "
  ${List.Create} editors
  ${List.Append} editors "jEdit,PSPad,WinShell,TeXnicCenter,WinEdt"
  ${List.Count} $R1 editors
  IntOp $R1 $R1 - 1
  ${For} $R2 0 $R1
    ${List.Get} $0 editors $R2
    ${SearchAllRegistry} $0 $1 $1 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" $0 0 0 # Helper Function from above
    ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$0" "InstallLocation"
    ${If} $0 != ""
      StrCpy $1 $0 "" -1 # Get the last char
      ${If} $1 == "\" # If ends with "\"
        StrCpy $0 $0 -1 # Remove "\"
      ${EndIf}
      StrCpy $pathPrefix "$pathPrefix;$0"
    ${EndIf}
  ${Next}
  ${List.Destroy} editors

  # Following Software does not create "InstallLocation"
  # Vim  
  ${SearchAllRegistry} $0 $1 $1 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" "Vim" 0 0 # Helper Function from above
  ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$0" "DisplayIcon"
  ${StrStrAdv} $0 $0 "\gvim.exe" ">" "<" "0" "0" "0" # get everything before "\gvim.exe"
  ${If} $0 != ""
    StrCpy $pathPrefix "$pathPrefix;$0"
  ${EndIf}

  # JabRef, Hard to locate, JabRef uses MSI installer, lets try to find it through the .bib extension
  SetShellVarContext all
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  ${For} $R1 0 1 # Try as admin and then as user
    ReadRegStr $0 SHCTX "SOFTWARE\Classes\.bib" ""
    ReadRegStr $0 SHCTX "Software\Classes\$0\shell\open\command" ""
    ${StrRep} $0 $0 '"' "" # Remove quotes
    ${StrStrAdv} $0 $0 "\JabRef.exe" ">" "<" "0" "0" "0" # Get everything before "\JabRef.exe"
    ${If} ${FileExists} "$0\JabRef.exe"
      StrCpy $pathPrefix "$pathPrefix;$0"
      ${Break}
    ${EndIf}
    SetShellVarContext current # Retry as user
  ${Next}

  # Gnumeric
  SetShellVarContext all
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  ${For} $R1 0 1 # Try as admin and then as user
    ReadRegStr $0 SHCTX "Software\Classes\Applications\gnumeric.exe\shell\Open\command" ""
    ${StrRep} $0 $0 '"' "" # Remove quotes
    ${StrStrAdv} $0 $0 "\gnumeric.exe" ">" "<" "0" "0" "0" # Get everything before "\gnumeric.exe"
    ${If} $0 != ""
      StrCpy $pathPrefix "$pathPrefix;$0"
      ${Break}
    ${EndIf}
    SetShellVarContext current # Retry as user
  ${Next}

  # Pandoc, same as JabRef, can only attempt to ask cmd if added to PATH, the user can't disable that while installing pandoc though
  nsExec::ExecToStack "where pandoc.exe"
  Pop $1 # Exit code
  Pop $0 # Return sring
  ${If} $1 = 0
    ${StrStrAdv} $0 $0 "\pandoc.exe" ">" "<" "0" "0" "0" # Get everything before "\pandoc.exe"
    ${If} $0 != ""
      StrCpy $pathPrefix "$pathPrefix;$0"
    ${EndIf}
  ${EndIf}

  ${Map.Create} softwarePathsMap # Create a map (NSISList plugin) to temp store paths of 3rd party software

  # LilyPond
  ${SearchAllRegistry} $0 $1 $1 "SOFTWARE" "LilyPond" 0 0 # Helper Function from above
  ReadRegStr $0 SHCTX "SOFTWARE\$0" "Install_Dir"
  ${If} $0 != ""
    StrCpy $pathPrefix "$pathPrefix;$0\usr\bin"
    ${Map.Set} softwarePathsMap "LilyPond" "$0\usr\bin"
  ${EndIf}

  # Gimp
  ${SearchAllRegistry} $2 $1 $1 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" "GIMP" 0 0 # Helper Function from above
  ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$2" "InstallLocation"
  ${If} $0 != ""
    StrCpy $pathPrefix "$pathPrefix;$0bin" # Install Location ends with '\' for Gimp in Registry
    ReadRegStr $0 SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$2" "DisplayIcon"
    ${StrStrAdv} $0 $0 ".exe" ">" "<" "0" "0" "0" # Get everything before the ".exe"
    ${StrStrAdv} $0 $0 "\" "<" ">" "0" "0" "0" # Get everything after the last "\", $0 contains "gimp-10" now
    ${Map.Set} softwarePathsMap "ImageEditor" $0
  ${EndIf}

  # Krita
  ${SearchAllRegistry} $0 $1 $1 "SOFTWARE" "Krita" 0 0 # Helper Function from above
  ReadRegStr $0 SHCTX "SOFTWARE\$0" "InstallLocation"
  ${If} $0 != ""
    StrCpy $pathPrefix "$pathPrefix;$0\bin"
    ${Map.Set} softwarePathsMap "ImageEditor" "krita"
  ${EndIf}

  # Photoshop
  SetShellVarContext all
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
  ${For} $R1 0 1 # Try as admin and then as user
    ReadRegStr $0 SHCTX "Software\Classes\Applications\Photoshop.exe\shell\Open\command" ""
    ${StrRep} $0 $0 '"' "" # Remove quotes
    ${StrStrAdv} $0 $0 "\photoshop.exe" ">" "<" "0" "0" "0" # Get everything before "\photoshop.exe"
    ${If} $0 != ""
      StrCpy $pathPrefix "$pathPrefix;$0"
      ${Break}
    ${EndIf}
    SetShellVarContext current # Retry as user
  ${Next}

  ClearErrors

  FileOpen $R0 "$INSTDIR\Resources\lyxrc.dist" a
  FileSeek $R0 0 END
  # set some general things
  FileWrite $R0 '\screen_zoom 120$\r$\n'

  FileWrite $R0 '\path_prefix "$pathPrefix"$\r$\n'
  
  # use pdfview for all types of PDF files
  FileWrite $R0 '\format "pdf5" "pdf" "PDF (LuaTeX)" "u" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		\format "pdf4" "pdf" "PDF (XeTeX)" "X" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		\format "pdf3" "pdf" "PDF (dvipdfm)" "m" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		\format "pdf2" "pdf" "PDF (pdflatex)" "F" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		\format "pdf" "pdf" "PDF (ps2pdf)" "P" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n'

  # If krita or gimp was found previously
  ${Map.Get} $0 softwarePathsMap "ImageEditor"
  ${If} $0 != "__NULL" # If "ImageEditor" is set in map    
    FileWrite $R0 '\format "gif" "gif" "GIF" "" "auto" "$0" "" "image/gif"$\r$\n\
		  \format "jpg" "jpg, jpeg" "JPEG" "" "auto" "$0" "" "image/jpeg"$\r$\n\
		  \format "png" "png" "PNG" "" "auto" "$0" "" "image/x-png"$\r$\n'
  ${EndIf}
  
  FileWrite $R0 '\converter "wmf" "eps" "magick -density 300 $$$$i $$$$o" ""$\r$\n\
    \converter "emf" "eps" "magick -density 300 $$$$i $$$$o" ""$\r$\n'

  # If LilyPond was found previously
  ${Map.Get} $0 softwarePathsMap "LilyPond"
  ${If} $0 != "__NULL" # If "LilyPond" is set in map
    FileWrite $R0 '\format "lilypond-book" "lytex" "LilyPond book (LaTeX)" "" "" "auto" "document,menu=export" ""$\r$\n\
		  \converter "lilypond-book" "pdflatex" "python \"$0\\lilypond-book.py\" --safe --pdf --latex-program=pdflatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "xetex" "python \"$0\\lilypond-book.py\" --safe --pdf --latex-program=xelatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "luatex" "python \"$0\\lilypond-book.py\" --safe --pdf --latex-program=lualatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "latex" "python \"$0\\lilypond-book.py\" --safe --lily-output-dir=ly-eps $$$$i" ""$\r$\n'
  ${EndIf}

  ${Map.Destroy} softwarePathsMap

  FileClose $R0
  
  ${If} ${Errors}
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(ModifyingConfigureFailed)" /SD IDOK
    ClearErrors
  ${EndIf}
SectionEnd

Section "$(SecFileAssocTitle)" SecFileAssoc # Registry information to assiciate ".lyx" extension with this LyX installation
  Call PrepareShellCTX # Helper function from above

  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\LyX.exe" "" "$INSTDIR\bin\LyX.exe" # Writes in both 64 and 32 bit registry
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\LyX.exe" "Version" ${APP_VERSION} # Writes in both 64 and 32 bit registry
  
  WriteRegStr SHCTX "Software\Classes\.lyx" "" "LyX.Document"
  WriteRegStr SHCTX "Software\Classes\.lyx" "Content Type" "application/lyx"

  WriteRegStr SHCTX "Software\Classes\LyX.Document" "" "LyX Document"
  WriteRegStr SHCTX "Software\Classes\LyX.Document\DefaultIcon" "" "$INSTDIR\bin\LyX.exe,1"
  WriteRegStr SHCTX "Software\Classes\LyX.Document\Shell\open\command" "" '"$INSTDIR\bin\LyX.exe" "%1"'

  # we need to update also the automatically created entry about the lyx.exe
  # otherwise .lyx-files will could be opened with an older LyX version
  ReadRegStr $0 SHCTX "Software\Classes\Applications\LyX.exe\shell\open\command" ""
  ${if} $0 != "" # if something was found
    WriteRegStr SHCTX "Software\Classes\Applications\LyX.exe\shell\open\command" "" '"$INSTDIR\bin\LyX.exe" "%1"'
  ${endif}

  WriteRegStr SHCTX "SOFTWARE\LyX" "FileAssoc${VERSION_MAJOR}${VERSION_MINOR}" $INSTDIR # Save user preference for over-installing
SectionEnd

Section "$(SecDesktopTitle)" SecDesktop
  SetOutPath "$INSTDIR\bin" # Need to set this so that the shortcut to lyx.exe is executed in the bin folder
  CreateShortCut "$DESKTOP\LyX ${VERSION_MAJOR}.${VERSION_MINOR}.lnk" "$INSTDIR\bin\LyX.exe" "" "" "" "" "" "LyX - The Document Processor" # Last parameter is the comment of the shortcut
  Call PrepareShellCTX
  WriteRegStr SHCTX "SOFTWARE\LyX" "Desktop${VERSION_MAJOR}${VERSION_MINOR}" $INSTDIR # Save user preference for over-installing
SectionEnd

Section -StartMenuFolder # Creates starm menu shortcut and website links to lyx website and lyx wiki
  Call PrepareShellCTX # Helper function from above, need to call be before the following macro, because the macro writes registry string (storing the default startmenu folder)
  !insertmacro MUI_STARTMENU_WRITE_BEGIN startmenuPage # This macro also assures that the folder is not empty for us
    SetOutPath "$INSTDIR\bin" # Need to set this so that the shortcut to lyx.exe is executed in the bin folder
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\LyX.lnk" "$INSTDIR\bin\LyX.exe" "" "" "" "" "" "LyX - The Document Processor" # Last parameter is the comment of the shortcut
    SetOutPath $INSTDIR
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall LyX.lnk" "$INSTDIR\Uninstall-LyX.exe"
    WriteINIStr "$SMPROGRAMS\$StartMenuFolder\LyX Website.url" "InternetShortcut" "URL" "https://www.lyx.org/"
    WriteINIStr "$SMPROGRAMS\$StartMenuFolder\LyX Wiki.url" "InternetShortcut" "URL" "https://wiki.lyx.org"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

!include "src\dictionaries.nsh"

Section -SoftwareRegistry # Registry information in "SOFTWARE" subkey
  Call PrepareShellCTX # Helper function from above

  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "latestVersion"
  ${If} $0 S< ${APP_VERSION_DOTLESS}
    WriteRegStr SHCTX "SOFTWARE\LyX" "latestVersion" ${APP_VERSION_DOTLESS}
  ${EndIf}
  WriteRegStr SHCTX "SOFTWARE\LyX\${APP_VERSION_DOTLESS}" "" $INSTDIR
  WriteRegStr SHCTX "SOFTWARE\LyX\${APP_VERSION_DOTLESS}" "LaTeX Path" $LatexPath
  WriteRegStr SHCTX "SOFTWARE\LyX" "InstDir${VERSION_MAJOR}${VERSION_MINOR}" $INSTDIR # Remember install dir for the next over-installment of the same series
SectionEnd

Section -UninstallInfoRegistry # Registry information in "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall" subkey
  Call PrepareShellCTX # Helper function from above

  ${If} $MultiUser.InstallMode == "AllUsers"
    WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "DisplayName" "LyX ${APP_VERSION}"
  ${Else}
    WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "DisplayName" "LyX ${APP_VERSION} $(TEXT_INSTALL_CURRENTUSER)"
  ${EndIf}

  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "UninstallString" '"$INSTDIR\Uninstall-LyX.exe"'
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "DisplayVersion" "${APP_VERSION}"
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "VersionMajor" "${VERSION_MAJOR}"
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "VersionMinor" "${VERSION_MINOR}"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "DisplayIcon" "$INSTDIR\bin\LyX.exe"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "URLUpdateInfo" "https://www.lyx.org/"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "URLInfoAbout" "https://www.lyx.org/AboutLyX"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "Publisher" "LyX Team"
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "HelpLink" "https://www.lyx.org/MailingLists"
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "NoModify" 0x00000001
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "NoRepair" 0x00000001
  WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "InstallLocation" "$INSTDIR"

  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}" "EstimatedSize" $0
SectionEnd

!if ${VERSION_MINOR} > 0 # Only valid for minor releases of the same major release (e.g. not valid for 3.0)
  Section -CopyOldPreferences # Searches user's preferences (userdir) from previous minor release and copies it if necessary
    Call PrepareShellCTX
    SetShellVarContext current # Otherwise $APPDATA would return C:\ProgrammData instead of C:\Users\username\AppData\Roaming
    
    IntOp $0 ${VERSION_MINOR} - 1 # only check the direct previous minor release
    ${If} ${FileExists} "$APPDATA\LyX${VERSION_MAJOR}.$0\lyxrc.defaults"
    ${AndIfNot} ${FileExists} "$APPDATA\LyX${VERSION_MAJOR}.${VERSION_MINOR}\lyxrc.defaults"
      CopyFiles /SILENT "$APPDATA\LyX${VERSION_MAJOR}.$0\*.*" "$APPDATA\LyX${VERSION_MAJOR}.${VERSION_MINOR}"
    ${EndIf}
  SectionEnd
!endif

Section -ConfigureScript # Runs the configure.py script

  # Manipulate PATH environment of the running installer process, so that configure.py can find all the stuff needed
  ReadEnvStr $0 "PATH"
  ${StrRep} $pathPrefix $pathPrefix "$$LyXDir" "$INSTDIR"
  System::Call 'Kernel32::SetEnvironmentVariable(t "PATH", t "$pathPrefix;$0")'
  
  Call PrepareShellCTX
  SetShellVarContext current # Otherwise $APPDATA would return C:\ProgrammData instead of C:\Users\username\AppData\Roaming
  SetOutPath "$APPDATA\LyX${VERSION_MAJOR}.${VERSION_MINOR}" # Need to run configure from the user dir, because it creates .lst files and some folders.

  ${StrStr} $0 $LatexPath "\miktex\bin"
  # R9 is used in $(TEXT_CONFIGURE_LYX)
  ${If} $0 == "" # TexLive
    StrCpy $R9 "TeXLive"
  ${Else}
    StrCpy $R9 "MiKTeX"
  ${EndIf}
  StrCpy $LaTeXInstalled $R9 # FIXME remove
  DetailPrint $(TEXT_CONFIGURE_LYX) # Uses R9 to display the name of the installed latex distribution
  
  nsExec::ExecToLog '"$INSTDIR\Python\python.exe" "$INSTDIR\Resources\configure.py"'
  Pop $0 # Return value
SectionEnd

Function DefaultDesktopFileAssoc # Custom function, called before the components page, reads the states of SecDesktop and SecFileAssoc from registry (if overinstalling) and sets them

  Call CheckDictionaries # Search for already installed dictionaries, in dictionaries.nsh

  ${IfNot} ${FileExists} "$INSTDIR\Uninstall-LyX.exe"
    Return # Not over-installing
  ${EndIf}

  Call PrepareShellCTX
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "FileAssoc${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == ""
    SectionSetFlags ${SecFileAssoc} 0 # 0 means unselected
  ${EndIf}
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "Desktop${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == ""
    SectionSetFlags ${SecDesktop} 0 # 0 means unselected
  ${EndIf}
FunctionEnd

Function StartLyX
  ShellExecAsUser::ShellExecAsUser "" "$INSTDIR\bin\LyX.exe"
FunctionEnd

# Installer Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "$(SecFileAssocDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDictionaries} "$(SecDictionariesDescription)" # In dictionaries.nsh
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThesaurus} "$(SecThesaurusDescription)" # In dictionaries.nsh
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# ===== Uninstaller ======

Function un.onInit # Callback function, called when the uninstaller initializes
  # Check that LyX is not currently running
  ${If} ${RunningX64}
    ${DisableX64FSRedirection} # We need the following process to be 64 bit on 64 bit system
  ${EndIf}
  nsExec::ExecToStack "powershell (Get-Process LyX).Path"
  Pop $0 # Exit code
  Pop $0 # Result string
  ${If} ${RunningX64}
    ${EnableX64FSRedirection} # Need to be enabled asap or installer might crash
  ${EndIf}
  ${UnStrStr} $0 $0 "$INSTDIR\bin\LyX.exe"
  ${If} $0 != ""
    MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)" /SD IDOK
    Quit # Quit uninstaller
  ${EndIf}

  Call un.PrepareShellCTX
  !insertmacro MULTIUSER_UNINIT

  # Ascertain whether the user has sufficient privileges to uninstall.
  # abort when LyX was installed with admin permissions but the user doesn't have administrator privileges
  Call un.PrepareShellCTX # Helperfunction from above
  ReadRegStr $0 HKLM "SOFTWARE\LyX\${APP_VERSION_DOTLESS}" ""
  ${If} $0 == $INSTDIR
    ${If} $MultiUser.Privileges != "Admin"
    ${AndIf} $MultiUser.Privileges != "Power"
      MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)" /SD IDOK
      Abort
    ${Else}
      StrCpy $MultiUser.InstallMode "AllUsers"
    ${EndIf}
  ${Else}
    ReadRegStr $0 HKCU "SOFTWARE\LyX\${APP_VERSION_DOTLESS}" ""
    ${If} $0 == ""
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)" /SD IDOK
    ${Else}
      StrCpy $MultiUser.InstallMode "CurrentUser"
    ${EndIf}
  ${EndIf}

  # question message if the user really wants to uninstall LyX
  ${IfCmd} MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" /SD IDYES IDNO ${||} Quit ${|}
FunctionEnd

Function un.ComponentsPre # Callback Function, called before the components page is loaded
  # R9 and R8 are used in $(SecUnPreferencesDescription)
  SetShellVarContext current
  ${UnStrStrAdv} $R9 $PROFILE "\" "<" "<" "0" "0" "0" # Get everything before the last "\" of $PROFILE ("C:\Users\username") -> $R9: "C:\Users"
  StrCpy $AppPre $R9 # FIXME remove
  ${UnStrRep} $R8 $APPDATA "$PROFILE\" "" # "C:\Users\username\" gets replaced with "" in "C:\Users\username\AppData\Roaming" -> $R8: "Appdata\Roaming"
  StrCpy $AppSuff $R8 # FIXME remove
FunctionEnd

Section "un.LyX" un.SecUnProgramFiles # Deletes files in $INSTDIR and registry entries
  SectionIn RO
  # LaTeX class files that were installed together with LyX
  # will not be uninstalled because other LyX versions will
  # need them and these few files don't harm to stay in LaTeX 

  Call un.PrepareShellCTX # Helperfunction from above

  # Subfolders and unistaller
  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\Resources"
  RMDir /r "$INSTDIR\Python"
  RMDir /r "$INSTDIR\imagemagick"
  RMDir /r "$INSTDIR\ghostscript"
  Delete "$INSTDIR\Uninstall-LyX.exe"
  # Install directory
  SetOutPath $PLUGINSDIR
  RMDir $INSTDIR

  # Startmenu
  !insertmacro MUI_STARTMENU_GETFOLDER startmenuPage $StartMenuFolder # Reads registry value written with macro MUI_STARTMENU_WRITE_BEGIN
  ShellLink::GetShortCutTarget "$SMPROGRAMS\$StartMenuFolder\LyX.lnk" # Check if the shortcut really is pointing to the current installation
  Pop $0
  ${IfNot} ${FileExists} $0 # If still exists, then belongs to another lyx installment, so don't delete
    RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    DeleteRegValue SHCTX "SOFTWARE\LyX" "SMDir${VERSION_MAJOR}${VERSION_MINOR}"
  ${EndIf}

  # Desktop icon
  ShellLink::GetShortCutTarget "$DESKTOP\LyX ${VERSION_MAJOR}.${VERSION_MINOR}.lnk"
  Pop $0
  ${UnStrStr} $0 $0 $INSTDIR
  ${If} $0 != ""
    Delete "$DESKTOP\LyX ${VERSION_MAJOR}.${VERSION_MINOR}.lnk"
  ${EndIf}
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "Desktop${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == $INSTDIR
    DeleteRegValue SHCTX "SOFTWARE\LyX" "Desktop${VERSION_MAJOR}${VERSION_MINOR}"
  ${EndIf}

  # File association
  ReadRegStr $0 SHCTX "Software\Classes\LyX.Document\DefaultIcon" ""
  ${UnStrStr} $0 $0 $INSTDIR
  ${If} $0 != ""
    DeleteRegKey SHCTX "Software\Classes\LyX.Document"
    DeleteRegKey SHCTX "Software\Classes\.lyx"
  ${EndIf}
  ReadRegStr $0 SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\LyX.exe" ""
  ${UnStrStr} $0 $0 $INSTDIR
  ${If} $0 != ""
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\LyX.exe" # Deletes key in both 64 and 32 bit registry
  ${EndIf}
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "FileAssoc${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == $INSTDIR
    DeleteRegValue SHCTX "SOFTWARE\LyX" "FileAssoc${VERSION_MAJOR}${VERSION_MINOR}"
  ${EndIf}
  DeleteRegKey SHCTX "Software\Classes\Applications\LyX.exe"

  # Uninstaller info registry
  DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${APP_VERSION_DOTLESS}"

  # Software registry
  DeleteRegKey SHCTX "SOFTWARE\LyX\${APP_VERSION_DOTLESS}"
  DeleteRegValue SHCTX "SOFTWARE\LyX" "latestVersion"
  ReadRegStr $0 SHCTX "SOFTWARE\LyX" "InstDir${VERSION_MAJOR}${VERSION_MINOR}"
  ${If} $0 == $INSTDIR
    DeleteRegValue SHCTX "SOFTWARE\LyX" "InstDir${VERSION_MAJOR}${VERSION_MINOR}"
  ${EndIf}
  DeleteRegValue SHCTX "SOFTWARE\LyX" "latestVersion"
  # Now that "latestVersion" is gone we need to find if there is another (newest) LyX version installed and write it's verion to "latestVersion"
  # The keys in the registry are sorted alphabetically, so we dont need to compare, the latest found will be the newest version
  StrCpy $0 0
  StrCpy $1 ""
  ${Do}
    StrCpy $2 $1
    EnumRegKey $2 SHCTX "SOFTWARE\LyX" $0
    IntOp $0 $0 + 1
  ${LoopUntil} $1 == ""
  ${If} $2 != ""
    WriteRegStr SHCTX "SOFTWARE\LyX" "latestVersion" $2
  ${EndIf}
  DeleteRegKey /ifempty SHCTX "SOFTWARE\LyX"
SectionEnd

Section /o "un.$(UnLyXPreferencesTitle)" un.SecUnPreferences # Deletes user preferences folders for all users (if admin) and registry entry made by LyX to HKCU
  Call un.PrepareShellCTX
  SetShellVarContext current
  # If installed as user, only delete folder and registry key of current user
  ${If} $MultiUser.InstallMode == "CurrentUser"
    RMDir /r "$APPDATA\LyX${VERSION_MAJOR}.${VERSION_MINOR}"

  ${Else} # If installed as admin, things are a little more complicated
    ${UnStrStrAdv} $2 $PROFILE "\" "<" ">" "0" "0" "0" # Get everything after the last "\" of $PROFILE ("C:\Users\username") -> $2: "username"
    ${UnStrStrAdv} $3 $PROFILE "\" "<" "<" "0" "0" "0" # Get everything before the last "\" of $PROFILE ("C:\Users\username") -> $3: "C:\Users"
    ClearErrors
    FindFirst $1 $0 "$3\*" # Find any folder/file in "C:\Users"
    ${DoUntil} ${Errors} # Errors are set while FindFirst/FindNext if the search is completed (there are no more files)
      ${UnStrRep} $0 $APPDATA $2 $0 # Replace "username" in $APPDATA ("C:\Users\username\AppData\Roaming") with what was found
      ${If} ${FileExists} "$0\LyX${VERSION_MAJOR}.${VERSION_MINOR}\*" # Check if anything exists in the LyX userdir
        RMDir /r "$0\LyX${VERSION_MAJOR}.${VERSION_MINOR}"
      ${EndIf}
      FindNext $1 $0
    ${Loop}
    FindClose $1
  ${EndIf}

  # Can only delete for current user, because Admin can't access registry of other users
  DeleteRegKey HKCU "Software\LyX\LyX${VERSION_MAJOR}.${VERSION_MINOR}"
  DeleteRegKey /ifempty HKCU "Software\LyX"
SectionEnd

# Uninstaller Section Descriptions

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnProgramFiles} "$(SecUnProgramFilesDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnPreferences} "$(SecUnPreferencesDescription)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END