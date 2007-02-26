# Aspell dictionaries

Function DownloadDictionary
	
 FileOpen $R5 "$INSTDIR\Resources\AspellDictionaryNames.txt" r
 ${Do}
  FileRead $R5 $String ; $String is now the dictionary name
  StrCpy $R3 $String 2 ; $R3 is now the dictionary language code
  ${if} $R3 == "tr"
  ${andif} $DictCode != "tr" ; if nothing was found 
   FileClose $R5
   StrCpy $String ""
   StrCpy $AspellInstallYes "$RunNumber$AspellInstallYes"
   Goto abortinstall
  ${endif}
 ${LoopUntil} $DictCode == $R3
 FileClose $R5

  StrCpy $String $String -2 ; delete the linebreak characters at the end
 
  # Download aspell dictionaries,
  # if first download repository is not available try the other ones listed in "AspellRepositories.txt"
  FileOpen $R5 "$INSTDIR\Resources\AspellRepositories.txt" r
  ${For} $4 1 4
   FileRead $R5 $Search ; $Search is now the AspellLocation
   StrCpy $Search $Search -2 ; delete the linebreak characters at the end
   Push $R0
   InetLoad::load /TIMEOUT=5000 "$Search/aspell6-$String.exe" "$INSTDIR\aspell6-$String.exe" /END
   Pop $R0
   # test if the downloaded file is really the expected one, because if the file didn't exist on the download server,
   # berlios.de downloads a text file with the name of the non-existing file that contains the line "File doesn't exist" 
   FileOpen $R4 "$INSTDIR\aspell6-$String.exe" r
   FileRead $R4 $Search
   FileClose $R4
   StrCpy $Search $Search -1 ; delete the unix linebreak character at the end
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

#--------------------------------

Function InstallAspellDictionary
	
 StrCpy $AspellInstallYes ""

 # install the english dictionary if not already installed
 StrCpy $DictCode "en"
 StrCpy $RunNumber "1"
 ${if} $AspellBaseReg == "HKLM" ; $AspellBaseReg is either "HKLM" or if Aspell is already installed only for the current user "HKCU"
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
 StrCpy $0 $DictCode ; $0 is now the language code of the chosen LyX menu language
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
 ${endif} ; endif $AspellInstallYes == "1"
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
  ${endif} ; endif $LangCodeSys == "en"
 ${else} ; else ${if} $LangCodeSys != $DictCode
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
	 
FunctionEnd
 
#---------------------------

Function un.UninstAspell

    ReadRegStr $1 HKLM "Software\Aspell" "Base Path"
    # delete Aspells' install folder
    RMDir /r $1
    # unregister Aspell and its dictionaries
    DeleteRegKey HKLM "Software\Aspell"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-af"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-am"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-az"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-be"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-bg"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-bn"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-br"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ca"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-cs"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-csb"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-cy"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-da"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-de"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-en"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-el"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-eo"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-es"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-et"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fa"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fi"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fo"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-fr"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ga"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gd"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gl"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gu"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-gv"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-he"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hi"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hil"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hr"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hsb"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-hu"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ia"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-id"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-is"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-it"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ku"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-la"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-lt"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-lv"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mg"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mi"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mk"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mn"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mr"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ms"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-mt"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nb"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nds"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nl"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-nn"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-no"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ny"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-or"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pa"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pl"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-pt"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-qu"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ro"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ru"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-rw"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sc"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sk"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sl"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sr"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sv"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-sw"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-ta"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-te"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tet"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tl"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tn"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-tr"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-uk"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-uz"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-vi"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-wa"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-yi"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspell6-Dictionary-zu"
  
FunctionEnd

