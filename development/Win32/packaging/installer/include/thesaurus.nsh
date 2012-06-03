# download dictionaries

Function DownloadHunspellDictionary
 # Downloads hunspell dictionaries from a location that is given in the file
 # $INSTDIR\Resources\HunspellDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\HunspellDictionaryNames.txt" r
 ${For} $5 1 114
  FileRead $R5 $String # $String is now the dictionary name
  StrCpy $R3 $String 5 # $R3 is now the dictionary language code
  
  ${if} $DictCode == $R3
   StrCpy $String $String -2 # delete the linebreak characters at the end
   # Download hunspell dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
   
   ${For} $4 1 8
    FileRead $R4 $Search # $Search is now the mirror
    StrCpy $Search $Search -2 # delete the linebreak characters at the end
    Push $R0
    InetLoad::load /TIMEOUT=5000 "http://$Search.dl.sourceforge.net/project/lyxwininstaller/hunspell/$String" "$INSTDIR\Resources\dicts\$String" /END
    Pop $R0
    ${if} $R0 == "OK"
     ${ExitFor}
    ${endif}
   ${Next}
   
   FileClose $R4
   # if download failed
   ${if} $R0 != "OK"
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(HunspellFailed)"
    Goto abortinstall
   ${endif}
  ${endif} # end if $DictCode == $R3
  
 ${Next}
 FileClose $R5

  abortinstall:
  Delete "$INSTDIR\$String"

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
   # Download thesaurus dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
   
   ${For} $4 1 8
    FileRead $R4 $Search # $Search is now the mirror
    StrCpy $Search $Search -2 # delete the linebreak characters at the end
    Push $R0
    InetLoad::load /TIMEOUT=5000 "http://$Search.dl.sourceforge.net/project/lyxwininstaller/thesaurus/$String" "$INSTDIR\Resources\thes\$String" /END
    Pop $R0
    ${if} $R0 == "OK"
     ${ExitFor}
    ${endif}
   ${Next}
   
   FileClose $R4
   # if download failed
   ${if} $R0 != "OK"
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(ThesaurusFailed)"
    Goto abortinstall
   ${endif}
  ${endif} # end if $ThesCode == $R3
  
 ${Next}
 FileClose $R5

  abortinstall:
  Delete "$INSTDIR\$String"

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
  Call DownloadHunspellDictionary
 ${LoopUntil} $DictCodes == ""
 
 # some dictionaries need to be renamed
 ${if} ${FileExists} "$INSTDIR\Resources\dicts\db_DE.aff"
  Rename "$INSTDIR\Resources\dicts\db_DE.aff" "$INSTDIR\Resources\dicts\dsb_DE.aff"
  Rename "$INSTDIR\Resources\dicts\db_DE.dic" "$INSTDIR\Resources\dicts\dsb_DE.dic"
 ${endif}
 ${if} ${FileExists} "$INSTDIR\Resources\dicts\hb_DE.aff"
  Rename "$INSTDIR\Resources\dicts\hb_DE.aff" "$INSTDIR\Resources\dicts\hsb_DE.aff"
  Rename "$INSTDIR\Resources\dicts\hb_DE.dic" "$INSTDIR\Resources\dicts\hsb_DE.dic"
 ${endif}
 
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
