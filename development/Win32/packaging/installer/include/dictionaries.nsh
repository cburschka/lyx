/*
dictionaries.nsh

Handling of hunspell / MyThes dictionaries
*/

# This script contains the following functions:
#
# - FindDictionaries (finds already installed dictionaries)
#
# - DownloadHunspellDictionaries and DownloadThesaurusDictionaries
#    (Downloads hunspell / MyThes dictionaries from a location that is
#     given in the file $INSTDIR\Resources\HunspellDictionaryNames.txt)
#
# - InstallHunspellDictionaries and InstallThesaurusDictionaries
#    (installs the selected hunspell / MyThes dictionaries except of
#     already existing ones), uses:
#    DownloadHunspellDictionaries or DownloadThesaurusDictionaries

# ---------------------------------------

Function FindDictionaries
  # finds already installed dictionaries

  # start with empty strings
  StrCpy $FoundDict ""
  StrCpy $FoundThes ""
  
  # read out the possible spell-checker filenames from the file	
  FileOpen $R5 "$INSTDIR\Resources\HunspellDictionaryNames.txt" r
  ${for} $5 1 71
   # the file has 142 lines, but we only need to check for one of the 2 dictionary files per language
   # therefore check only for every second line
   FileRead $R5 $String   # skip the .aff file
   FileRead $R5 $String   # $String is now the .dic filename
   StrCpy $String $String -2 # remove the linebreak characters
   StrCpy $R3 $String -4 # $R3 is now the dictionary language code
   ${if} ${FileExists} "$INSTDIR\Resources\dicts\$String"
    StrCpy $FoundDict "$R3 $FoundDict"
   ${endif}
  ${next}
  FileClose $R5
  
  # read out the possible thesaurus filenames from the file	
  FileOpen $R5 "$INSTDIR\Resources\ThesaurusDictionaryNames.txt" r
  ${for} $5 1 26
   # the file has 52 lines, but we only need to check for one of the 2 dictionary files per language
   # therefore check only for every second line
   FileRead $R5 $String   # $String is now the dictionary name
   FileRead $R5 $String   # $String is now the dictionary name
   StrCpy $String $String -2 # remove the linebreak characters
   StrCpy $R3 $String 5 3 # $R3 is now the dictionary language code
   ${if} ${FileExists} "$INSTDIR\Resources\thes\$String"
    StrCpy $FoundThes "$R3 $FoundThes"
   ${endif}
  ${next}
  FileClose $R5

FunctionEnd

# ---------------------------------------

Function DownloadHunspellDictionaries
 # Downloads hunspell dictionaries from a location that is given in the file
 # $INSTDIR\Resources\HunspellDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\HunspellDictionaryNames.txt" r
 ${For} $5 1 142       # the file has 142 lines
 
  FileRead $R5 $String # $String is now the dictionary name
  StrCpy $R3 $String -6 # $R3 is now the dictionary language code
  
  ${if} $DictCode == $R3
   StrCpy $String $String -2 # delete the linebreak characters at the end
   # Download hunspell dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
   
   ${For} $4 1 29 # there are 29 mirrors in the file
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

Function DownloadThesaurusDictionaries
 # Downloads thesaurus dictionaries from a location that is given in the file
 # $INSTDIR\Resources\ThesaurusDictionaryNames.txt
 
 # read out the locations from the file	
 FileOpen $R5 "$INSTDIR\Resources\ThesaurusDictionaryNames.txt" r
 ${For} $5 1 52          # the file has 52 lines
 
  FileRead $R5 $String   # $String is now the dictionary name
  StrCpy $R3 $String 5 3 # $R3 is now the dictionary language code
  
  ${if} $ThesCode == $R3
   StrCpy $String $String -2 # delete the linebreak characters at the end
   # Download thesaurus dictionaries,
   # if first download repository is not available try the other ones listed in "DictionaryMirrors.txt"
   FileOpen $R4 "$INSTDIR\Resources\DictionaryMirrors.txt" r
   
   ${For} $4 1 29 # there are 29 mirrors in the file
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

Function InstallHunspellDictionaries
 # installs the selected hunspell dictionaries except of already existing ones

 ${Do}
  # take the first code
  StrCpy $Search ","
  StrCpy $String $DictCodes
  Call StrPoint
  # we always have a "," for each code, so in case in case something
  # went wrong, empty the code list to exit the loop
  ${if} $Pointer == "-1"
   StrCpy $DictCodes ""
  ${endif}
  ${if} $Pointer != "-1"
   StrCpy $DictCode $DictCodes $Pointer
   # remove the taken code from the list
   IntOp $Pointer $Pointer + 1
   StrCpy $DictCodes $DictCodes "" $Pointer
   # don't dowload existing ones thus check if $DictCode is in $FoundDict
   StrCpy $String $FoundDict
   StrCpy $Search $DictCode
   Call StrPoint # function from LyXUtils.nsh
   ${if} $Pointer == "-1"
    # download the dictionaries
    Call DownloadHunspellDictionaries
   ${endif}
  ${endif}
 ${LoopUntil} $DictCodes == ""
 
FunctionEnd

#--------------------------------

Function InstallThesaurusDictionaries
 # installs the selected thesaurus dictionaries except of already existing ones

 ${Do}
  # take the first code
  StrCpy $Search ","
  StrCpy $String $ThesCodes
  Call StrPoint
  # we always have a "," for each code, so in case in case something
  # went wrong, empty the code list to exit the loop
  ${if} $Pointer == "-1"
   StrCpy $ThesCodes ""
  ${endif}
  ${if} $Pointer != "-1"
   StrCpy $ThesCode $ThesCodes $Pointer
   # remove the taken code from the list
   IntOp $Pointer $Pointer + 1
   StrCpy $ThesCodes $ThesCodes "" $Pointer
   # don't dowload existing ones thus check if $ThesCode is in $FoundThes
   StrCpy $String $FoundThes
   StrCpy $Search $ThesCode
   Call StrPoint # function from LyXUtils.nsh
   ${if} $Pointer == "-1"
    # download the dictionaries
    Call DownloadThesaurusDictionaries
   ${endif}
  ${endif}
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

