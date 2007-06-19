/*

Aspell dictionaries

*/

#--------------------------------
#Macros

!macro Dictionary FILE LANGNAME LANGCODE SIZE

  Section /o "${LANGNAME}"
  
    AddSize ${SIZE}
  
    Push ${LANGCODE}
    Push "${LANGNAME}"
    Push ${FILE}
    
    Call DownloadDictionary
    
  SectionEnd
  
!macroend

#--------------------------------
#Sections

Section -AspellDicts

  Call GetAspellHive
  Pop $AspellHive

SectionEnd

!define DICT '!insertmacro Dictionary'

${DICT} af-0.50-0 "Afrikaans" af 916
${DICT} ar-1.2-0 "Arabic" ar 4345
${DICT} no-0.50-2 "Bokmål" no 1283
${DICT} bg-4.0-0 "Bulgarian" bg 5344
${DICT} ca-20040130-1 "Català" ca 1095
${DICT} cs-20040614-1 "Ceština" cs 2069
${DICT} da-1.4.42-1 "Dansk" da 3041
${DICT} de-20030222-1 "Deutsch" de 2874
${DICT} en-6.0-0 "English" en 1130
${DICT} es-0.50-2 "Español" es 2954
${DICT} eo-0.50-2 "Esperanto" eo 3771
${DICT} et-0.1.21-1 "Estonian" et 923
${DICT} fr-0.50-3 "Français" fr 4333
${DICT} gl-0.50-0 "Galego" gl 2973
${DICT} el-0.50-3 "Greek" el 3206
${DICT} he-1.0-0 "Hebrew" he 2026
${DICT} hr-0.51-0 "Hrvatski" hr 1967
${DICT} is-0.51.1-0 "Íslenska" is 1537
${DICT} it-2.2_20050523-0 "Italiano" it 696
${DICT} hu-0.99.4.2-0 "Magyar" hu 1608
${DICT} nl-0.50-2 "Nederlands" nl 1526
${DICT} nn-0.50.1-1 "Nynorsk" nn 1942
${DICT} pl-0.51-0 "Polski" pl 13685
${DICT} pt-0.50-2 "Português" pt 2291
${DICT} ro-0.50-2 "Româna" ro 342
${DICT} ru-0.99f7-1 "Russian" ru 1460
${DICT} gd-0.1.1-1 "Scottish" gd 195
${DICT} sr-0.02 "Serbian" sr 3684
${DICT} sk-0.52-0 "Slovencina" sk 4602
${DICT} sl-0.50-0 "Slovenšcina" sl 5227
${DICT} fi-0.7-0 "Suomi" fi 660
${DICT} sv-0.51-0 "Svenska" sv 1029
${DICT} tr-0.50-0 "Türkçe" tr 549
${DICT} uk-1.1-0 "Ukrainian" uk 582
${DICT} cy-0.50-3 "Welsh" cy 1770

#--------------------------------
#Functions

Function GetAspellHive

  #Check whether the system or local version of Aspell should be used
  #The patched Aspell uses the same logic

  Push $R0

  ReadRegStr $R0 HKCU "Software\Aspell" "Base Path"

  ${if} $R0 == ""
    StrCpy $R0 HKLM
  ${else}
    StrCpy $R0 HKCU
  ${endif}

  Exch $R0

FunctionEnd

Function DownloadDictionary

  Exch $R0
  Exch 1
  Exch $R1
  Exch 2
  Exch $R2
  Push $R3

  dict_download:

    #Download
    InetLoad::load "${DOWNLOAD_ASPELLDICTS}/aspell6-$R0.exe" "$PLUGINSDIR\aspell6-$R0.exe" /END
    Pop $R3

    ${if} $R3 != "OK"
      #Download failed, try an alternative link
      InetLoad::load "${ALT_DOWNLOAD_ASPELLDICTS}/aspell6-$R0.exe" "$PLUGINSDIR\aspell6-$R0.exe" /END
      Pop $R3
    ${endif}

    ${if} $R3 != "OK"
      #Download failed again
      MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_DICT) ($R3)" IDYES dict_download
      Goto dict_noinstall
    ${endif}

    install_dict:

      ExecWait '"$PLUGINSDIR\aspell6-$R0.exe" /NoDirChange /AutoClose'

      ${if} $AspellHive == HKLM
        ReadRegStr $R3 HKLM "Software\Aspell\Dictionaries" $R2
      ${else}
  	    ReadRegStr $R3 HKCU "Software\Aspell\Dictionaries" $R2
      ${endif}

	    ${if} $R3 == ""
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_NOTINSTALLED_DICT)" IDYES install_dict
      ${endif}

      Delete "$PLUGINSDIR\aspell6-$R0.exe"

    dict_noinstall:

  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0

FunctionEnd
