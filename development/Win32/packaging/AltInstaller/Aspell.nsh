# Aspell dictionaries

!if ${INSTALLER_TYPE} == "NotUpdate" # only for Small and Complete installer

Function InstallAspell
 # install Aspell when it is not already installed

  ${if} $AspellPath == ""
   # extract Aspell's program files
   SetOutPath "$INSTDIR\external"
   File /r "${PRODUCT_SOURCEDIR}\${AspellInstall}"
   # copy the files and register Aspell
   CopyFiles "$INSTDIR\${AspellInstall}" "$APPDATA"
   # finally copy the Aspell personal files to the Application folder of all users
   # this assures that every user can have its own word list
   StrCpy $AppSubfolder "Aspell"
   StrCpy $AppFiles "$APPDATA\Aspell\Personal"
   ${if} $ProductRootKey == "HKLM" # if install for all users
    Call CreateAppPathSub # function from LyXUtils.nsh
   ${else}
    CreateDirectory "$APPDATA\$AppSubfolder"
    CopyFiles "$AppFiles" "$APPDATA\$AppSubfolder"
   ${endif}
   
   WriteRegStr HKLM "SOFTWARE\Aspell" "Base Path" "${AspellDir}"
   WriteRegStr HKLM "SOFTWARE\Aspell" "Dictionary Path" "${AspellDictPath}"
   WriteRegStr HKLM "SOFTWARE\Aspell" "Personal Path" "${AspellPersonalPath}"
   
   WriteRegStr HKLM "Software\Aspell" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
   
   WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "DisplayName" "${AspellDisplay}"
   WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "NoModify" 0x00000001
   WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "NoRepair" 0x00000001
   WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "UninstallString" "${AspellDir}\${AspellUninstall}"
  ${endif}

FunctionEnd

!endif # endif ${INSTALLER_TYPE} == "NotUpdate"

#--------------------------------

!if ${INSTALLER_TYPE} == "NotUpdate" # only for Small and Complete installer

Function DownloadDictionary
 # Downloads Aspell dictionaries from a location that is given in the file
 # $INSTDIR\Resources\AspellDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\AspellDictionaryNames.txt" r
 ${Do}
  FileRead $R5 $String # $String is now the dictionary name
  StrCpy $R3 $String 2 # $R3 is now the dictionary language code
  ${if} $R3 == "tr"
  ${andif} $DictCode != "tr" # if nothing was found (the last line in the file starts with "tr")
   FileClose $R5
   StrCpy $String ""
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
   Goto abortinstall
  ${endif}
 ${LoopUntil} $DictCode == $R3
 FileClose $R5

  StrCpy $String $String -2 # delete the linebreak characters at the end
 
  # Download aspell dictionaries,
  # if first download repository is not available try the other ones listed in "AspellRepositories.txt"
  FileOpen $R5 "$INSTDIR\Resources\AspellRepositories.txt" r
  ${For} $4 1 4
   FileRead $R5 $Search # $Search is now the AspellLocation
   StrCpy $Search $Search -2 # delete the linebreak characters at the end
   Push $R0
   InetLoad::load /TIMEOUT=5000 "$Search/aspell6-$String.exe" "$INSTDIR\aspell6-$String.exe" /END
   Pop $R0
   # test if the downloaded file is really the expected one, because if the file didn't exist on the download server,
   # berlios.de downloads a text file with the name of the non-existing file that contains the line "File doesn't exist" 
   FileOpen $R4 "$INSTDIR\aspell6-$String.exe" r
   FileRead $R4 $Search
   FileClose $R4
   StrCpy $Search $Search -1 # delete the unix linebreak character at the end
   ${if} $Search == "File doesn't exist"
    StrCpy $R0 ""
   ${endif}
   ${if} $R0 == "OK"
    ${ExitFor}
   ${endif}
  ${Next}
  FileClose $R5
  
  # Download failed
  ${if} $R0 != "OK"
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(AspellDownloadFailed) $R0"
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
   Goto abortinstall
  ${endif}
 
  # Download successful
  ExecWait '"$INSTDIR\aspell6-$String.exe" /NoDirChange /AutoClose'
  ${if} $AspellBaseReg == "HKLM"
   ReadRegStr $R2 HKLM "Software\Aspell\Dictionaries" $DictCode
  ${else}
   ReadRegStr $R2 HKCU "Software\Aspell\Dictionaries" $DictCode
  ${endif}
  ${if} $R2 == ""
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(AspellInstallFailed)"
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
  ${endif}

  abortinstall:
  Delete "$INSTDIR\aspell6-$String.exe"

FunctionEnd

!endif # endif ${INSTALLER_TYPE} == "NotUpdate"

#--------------------------------

!if ${INSTALLER_TYPE} == "NotUpdate" # only for Small and Complete installer

Function InstallAspellDictionary
 # install Aspell dictionaries

 StrCpy $AspellInstallYes ""

 # install the english dictionary if not already installed
 StrCpy $DictCode "en"
 StrCpy $RunNumber "1"
 ${if} $AspellBaseReg == "HKLM" # $AspellBaseReg is either "HKLM" or if Aspell is already installed only for the current user "HKCU"
  ReadRegStr $R2 HKLM "Software\Aspell\Dictionaries" $DictCode
 ${else}
  ReadRegStr $R2 HKCU "Software\Aspell\Dictionaries" $DictCode
 ${endif}
 ${if} $R2 == ""
  MessageBox MB_OK|MB_ICONINFORMATION "$(AspellInfo)"
  StrCpy $AspellMessage "Yes"
  Call DownloadDictionary
 ${else}
  StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
 ${endif}
 
 # install the dictionary corresponding to the system and the chosen menu language
 # check if the system language and the chosen menu language are the same, if not install
 # both dictionaries
 StrCpy $DictCode $LangCode 2
 StrCpy $0 $DictCode # $0 is now the language code of the chosen LyX menu language
 StrCpy $RunNumber "2"
 ${if} $AspellInstallYes == "1"
 ${andif} $DictCode == "en"
  StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
 ${else}
  ${if} $AspellBaseReg == "HKLM"
   ReadRegStr $R2 HKLM "Software\Aspell\Dictionaries" $DictCode
  ${else}
   ReadRegStr $R2 HKCU "Software\Aspell\Dictionaries" $DictCode
  ${endif}
  ${if} $R2 == ""
   ${if} $AspellMessage != "Yes"
    MessageBox MB_OK|MB_ICONINFORMATION "$(AspellInfo)"
   ${endif}
   Call DownloadDictionary
  ${else}
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
  ${endif}
 ${endif} # endif $AspellInstallYes == "1"
 ${if} $LangCodeSys != $DictCode
  StrCpy $RunNumber "3"
  ${if} $LangCodeSys == "en"
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
  ${else}
   StrCpy $DictCode $LangCodeSys
   ${if} $AspellBaseReg == "HKLM"
    ReadRegStr $R2 HKLM "Software\Aspell\Dictionaries" $DictCode
   ${else}
    ReadRegStr $R2 HKCU "Software\Aspell\Dictionaries" $DictCode
   ${endif}
   ${if} $R2 == ""
    ${if} $AspellMessage != "Yes"
     MessageBox MB_OK|MB_ICONINFORMATION "$(AspellInfo)"
    ${endif}
    Call DownloadDictionary
   ${else}
    StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
   ${endif}
  ${endif} # endif $LangCodeSys == "en"
 ${else} # else ${if} $LangCodeSys != $DictCode
  StrCpy $AspellInstallYes "4$AspellInstallYes"
 ${endif}

 # check the registry to divide between nothing installed or all already installed
 ${if} $AspellInstallYes == "321"
 ${orif} $AspellInstallYes == "421"
  ${if} $AspellBaseReg == "HKLM"
   ReadRegStr $R2 HKLM "Software\Aspell\Dictionaries" "en"
  ${else}
   ReadRegStr $R2 HKCU "Software\Aspell\Dictionaries" "en"
  ${endif}
  ${if} $R2 != ""
   ${if} $0 == "en"
   ${andif} $LangCodeSys == "en"
    MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellAll) English$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
   ${endif}
   ${if} $0 == "en"
    MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellAll) English$(AspellPartAnd)$LangNameSys$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
   ${endif}
   ${if} $LangCodeSys == "en"
    MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellAll) English$(AspellPartAnd)$LangName$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
   ${endif}
   ${if} $LangCodeSys != $0
    MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellAll) English$(AspellPartSep)$LangName$(AspellPartAnd)$LangNameSys$\r$\n$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
   ${endif}
   ${if} $LangCodeSys == $0
    MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellAll) English$(AspellPartAnd)$LangName$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
   ${endif}
  ${endif}
 ${endif}
 
 Delete "$INSTDIR\Resources\AspellDictionaryNames.txt"
 Delete "$INSTDIR\Resources\AspellRepositories.txt"
 
 # show message about Aspell dictionaries
 # the code rule to display the correct message:
 # - when the englisch dictionary is already installed or couldn't be installed -> set a "1"
 # - when the dictionary of the chosen LyX menu language is already installed or couldn't be installed -> set a "2"
 # - when the dictionary of the Windows system language is already installed or couldn't be installed -> set a "3"
 # - when the dictionary of the chosen LyX menu language is equal to the dictionary of the Windows system language -> set a "4"
 ${if} $AspellInstallYes == "32"
 ${orif} $AspellInstallYes == "42"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart1)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "3"
 ${orif} $AspellInstallYes == "4"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart1)$(AspellPartAnd)$(AspellPart2)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "2"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart1)$(AspellPartAnd)$(AspellPart3)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == ""
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart1)$(AspellPartSep)$(AspellPart2)$(AspellPartAnd)$(AspellPart3)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "321"
 ${orif} $AspellInstallYes == "421"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellNone)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "31"
 ${orif} $AspellInstallYes == "41"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart2)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "21"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart3)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}
 ${if} $AspellInstallYes == "1"
  MessageBox MB_ICONINFORMATION|MB_DEFBUTTON2|MB_YESNO "$(AspellPartStart)$(AspellPart2)$(AspellPartAnd)$(AspellPart3)$(AspellPart4)" IDYES DownloadNow IDNO DownloadLater
 ${endif}

 DownloadNow:
  ExecShell "open" "${AspellLocationExact}"
 DownloadLater:

 # finally copy the Aspell dictionary files to the Application folder of all users
 # this assures that every user can have its own word list
 StrCpy $AppSubfolder "Aspell"
 StrCpy $AppFiles "$APPDATA\Aspell\Dictionaries"
 Call CreateAppPathSub # function from LyXUtils.nsh
 
FunctionEnd

!endif # endif ${INSTALLER_TYPE} == "NotUpdate"
 
#---------------------------


Function un.UninstAspell

    ReadRegStr $1 SHCTX "Software\Aspell" "Base Path"
    # delete Aspells' install folder
    RMDir /r $1
    # remove LyX's config files
    StrCpy $AppSubfolder "Aspell"
    Call un.DelAppPathSub # function from LyXUtils.nsh
    # unregister Aspell and its dictionaries
    DeleteRegKey SHCTX "Software\Aspell"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-af"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-am"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ar"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-az"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-be"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-bg"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-bn"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-br"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ca"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-cs"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-csb"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-cy"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-da"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-de"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-en"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-el"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-eo"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-es"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-et"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fa"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fi"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fo"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fr"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fy"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ga"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gd"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gl"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gu"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gv"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-he"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hi"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hil"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hr"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hsb"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hu"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hy"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ia"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-id"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-is"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-it"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ku"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-la"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-lt"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-lv"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mg"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mi"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mk"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ml"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mn"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mr"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ms"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mt"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nb"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nds"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nl"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nn"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-no"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ny"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-or"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pa"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pl"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pt"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-qu"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ro"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ru"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-rw"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sc"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sk"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sl"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sr"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sv"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sw"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ta"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-te"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tet"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tl"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tn"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tr"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-uk"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-uz"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-vi"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-wa"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-yi"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-zu"
  
FunctionEnd

