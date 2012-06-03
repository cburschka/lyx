# download dictionaries

Function DownloadHunspellDictionary
 # Downloads hunspell dictionaries from a location that is given in the file
 # $INSTDIR\Resources\HunspellDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\HunspellDictionaryNames.txt" r
 ${For} $5 1 114
  FileRead $R5 $String # $String is now the dictionary name
  StrCpy $R3 $String 5 3 # $R3 is now the dictionary language code
  MessageBox MB_OK|MB_ICONEXCLAMATION "$R3"
  
  ${if} $DictCode == $R3
   StrCpy $String $String -2 # delete the linebreak characters at the end
   StrCpy $FileName $String 15 # extract the real file name
   # Download hunspell dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
  
   ${For} $4 1 5
    FileRead $R4 $Search # $Search is now the mirror
    StrCpy $Search $Search -2 # delete the linebreak characters at the end
    Push $R0
    MessageBox MB_OK|MB_ICONEXCLAMATION "http://downloads.sourceforge.net/project/lyxwininstaller/thesaurus/$String&use_mirror=$Search"
    InetLoad::load /TIMEOUT=5000 "http://downloads.sourceforge.net/project/lyxwininstaller/thesaurus/$String&use_mirror=$Search" "$INSTDIR\Resources\dicts\$FileName" /END
    Pop $R0
    ${if} $R0 == "OK"
     ${ExitFor}
    ${endif}
   ${Next}
    
   FileClose $R4
   # if download failed
   ${if} $R0 != "OK"
    MessageBox MB_OK|MB_ICONEXCLAMATION "(AspellDownloadFailed) $R0"
    Goto abortinstall
   ${endif}
  ${endif} # end if $DictCode == $R3
  
 ${Next}
 FileClose $R5

  abortinstall:
  Delete "$INSTDIR\$FileName"

FunctionEnd

#--------------------------------

Function DownloadThesaurusDictionary
 # Downloads thesaurus dictionaries from a location that is given in the file
 # $INSTDIR\Resources\ThesaurusDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\ThesaurusDictionaryNames.txt" r
 ${For} $5 1 44
  FileRead $R5 $String # $String is now the dictionary name
  StrCpy $R3 $String 5 3 # $R3 is now the dictionary language code
  
  ${if} $ThesCode == $R3
   StrCpy $String $String -2 # delete the linebreak characters at the end
   StrCpy $FileName $String 15 # extract the real file name
   # Download thesaurus dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
   
   ${For} $4 1 5
    FileRead $R4 $Search # $Search is now the mirror
    StrCpy $Search $Search -2 # delete the linebreak characters at the end
    Push $R0
    MessageBox MB_OK|MB_ICONEXCLAMATION "http://downloads.sourceforge.net/project/lyxwininstaller/thesaurus/$String&use_mirror=$Search"
    InetLoad::load /TIMEOUT=5000 "http://downloads.sourceforge.net/project/lyxwininstaller/thesaurus/$String&use_mirror=$Search" "$INSTDIR\Resources\thes\$FileName" /END
    Pop $R0
    ${if} $R0 == "OK"
     ${ExitFor}
    ${endif}
   ${Next}
   
   FileClose $R4
   # if download failed
   ${if} $R0 != "OK"
    MessageBox MB_OK|MB_ICONEXCLAMATION "(AspellDownloadFailed) $R0"
    Goto abortinstall
   ${endif}
  ${endif} # end if $ThesCode == $R3
  
 ${Next}
 FileClose $R5

  abortinstall:
  Delete "$INSTDIR\$FileName"

FunctionEnd

#--------------------------------

Function InstallHunspellDictionary
 # install hunspell dictionaries

 # install the dictionary corresponding to the system and the chosen menu language
 # check if the system language and the chosen menu language are the same, if not install
 # both dictionaries
 ${Do}
  StrCpy $DictCode $DictCodes 5
  StrCpy $DictCodes $DictCodes "" 5
  MessageBox MB_OK|MB_ICONEXCLAMATION "$DictCode"
  Call DownloadHunspellDictionary
 ${LoopUntil} $DictCodes == ""
 
FunctionEnd

#--------------------------------

Function InstallThesaurusDictionary
 # install thesaurus dictionaries

 # install the dictionary corresponding to the system and the chosen menu language
 # check if the system language and the chosen menu language are the same, if not install
 # both dictionaries
 ${Do}
  StrCpy $ThesCode $ThesCodes 5
  StrCpy $ThesCodes $ThesCodes "" 5
  MessageBox MB_OK|MB_ICONEXCLAMATION "$ThesCode"
  Call DownloadThesaurusDictionary
 ${LoopUntil} $ThesCodes == ""
 
 # some dictionaries of language variants are identic
 # therefore copy and rename an existing dictionary
 CreateDirectory "$INSTDIR\Resources\backup"
 ${if} ${FileExists} "$INSTDIR\Resources\thes\th_de_DE_v2.dat"
  CopyFiles "$INSTDIR\Resources\thes\th_de_DE_v2.*" "$INSTDIR\Resources\backup"
  Rename "$INSTDIR\Resources\backup\th_de_DE_v2.dat" "$INSTDIR\Resources\backup\th_de_AT_v2.dat"
  Rename "$INSTDIR\Resources\backup\th_de_DE_v2.idx" "$INSTDIR\Resources\backup\th_de_AT_v2.idx"
  CopyFiles "$INSTDIR\Resources\backup\th_de_AT_v2.*" "$INSTDIR\Resources\thes"
 ${endif}
 ${if} ${FileExists} "$INSTDIR\Resources\thes\th_en_US_v2.dat"
  CopyFiles "$INSTDIR\Resources\thes\th_en_US_v2.*" "$INSTDIR\Resources\backup"
  Rename "$INSTDIR\Resources\backup\th_en_US_v2.dat" "$INSTDIR\Resources\backup\th_en_AU_v2.dat"
  Rename "$INSTDIR\Resources\backup\th_en_US_v2.idx" "$INSTDIR\Resources\backup\th_en_AU_v2.idx"
  CopyFiles "$INSTDIR\Resources\backup\th_en_AU_v2.*" "$INSTDIR\Resources\thes"
 ${endif}
 RMDir "$INSTDIR\Resources\backup"
 
FunctionEnd
