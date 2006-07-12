/*

Aspell dictionaries

*/

;--------------------------------
;Macros

!macro Dictionary file lang_name lang_code size

  Section /o "${lang_name}"
  
    AddSize ${size}
  
    Push ${lang_code}
    Push "${lang_name}"
    Push ${file}
    
    Call DownloadDictionary
    
  SectionEnd
  
!macroend

;--------------------------------
;Sections

Section -AspellDicts

  Call GetAspellHive
  Pop $AspellHive

SectionEnd

!define DICT '!insertmacro Dictionary'

${DICT} af-0.50-0 "Afrikaans" af 924
${DICT} bg-0.50-0 "Bulgarian" bg 9059
${DICT} ca-20040130-1 "Catalan" ca 1107
${DICT} hr-0.51-0 "Croatian" hr 2006
${DICT} cs-20040614-1 "Czech" cs 2096
${DICT} da-1.4.42-1 "Danish" da 3094
${DICT} nl-0.50-2 "Dutch" nl 1546
${DICT} en-6.0-0 "English" en 1144
${DICT} eo-0.50-2 "Esperanto" eo 3863
${DICT} et-0.1.21-1 "Estonian" et 931
${DICT} fi-0.7-0 "Finnish" fi 663
${DICT} fr-0.50-3 "French" fr 4428
${DICT} gl-0.50-0 "Galician" gl 3041
${DICT} de-20030222-1 "German" de 2929
${DICT} el-0.50-3 "Greek" el 3269
${DICT} he-0.9-0 "Hebrew" he 1485
${DICT} hu-0.99.4.2-0 "Hungarian" hu 1636
${DICT} is-0.51.1-0 "Icelandic" is 1556
;${DICT} ia-0.50-1 "Interlingua" ia 605
${DICT} it-2.2_20050523-0 "Italian" it 699
;${DICT} la-20020503-0 "Latin" la 179
${DICT} no-0.50-2 "Norwegian" no 1300
${DICT} nn-0.50.1-1 "Norwegian Nynorsk" nn 1962
${DICT} pl-0.51-0 "Polish" pl 14018
${DICT} pt-0.50-2 "Portuguese" pt 2326
${DICT} ro-0.50-2 "Romanian" ro 338
${DICT} ru-0.99f7-1 "Russian" ru 1477
${DICT} gd-0.1.1-1 "Scottish" gd 187
${DICT} sk-0.52-0 "Slovak" sk 4709
${DICT} sl-0.50-0 "Slovenian" sl 5252
${DICT} sr-0.02 "Serbian" sr 3755
${DICT} es-0.50-2 "Spanish" es 3018
${DICT} sv-0.51-0 "Swedish" sv 1040
${DICT} tr-0.50-0 "Turkish" tr 548
;${DICT} hsb-0.01-1 "Upper Sorbian" hsb 374
${DICT} uk-1.1-0 "Ukrainian" uk 583
${DICT} cy-0.50-3 "Welsh" cy 1801

;--------------------------------
;Functions

Function GetAspellHive

  Push $R0

  ReadRegStr $R0 HKLM "Software\Aspell" "Base Path"

  ${if} $R0 != ""
    StrCpy $R0 "machine"
  ${else}
    StrCpy $R0 "user"
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

    ;Download using HTTP
    NSISdl::download "http://wiki.lyx.org/uploads/Windows/Aspell6/aspell6-$R0.exe" "$PLUGINSDIR\aspell6-$R0.exe" /END
    Pop $R3

    ${if} $R3 != "success"
      ;Download failed
      MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_DICT) ($R3)" IDYES dict_download
      Goto dict_noinstall
    ${endif}

    install_dict:

      ExecWait '"$PLUGINSDIR\aspell6-$R0.exe" /NoDirChange /AutoClose'

      ${if} $AspellHive == "machine"
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
