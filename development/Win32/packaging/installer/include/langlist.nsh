/*

langlist.nsh

List of langauges supported by the LyX

*/

!macro Languages COMMAND

  # List of LyX translations with ISO code and language identifier

  ${COMMAND} "Català" ca_ES 1027
  ${COMMAND} "Ceština" cs_CZ 1029
  ${COMMAND} "Chinese (Simplified)" zh_CN 2052
  ${COMMAND} "Chinese (Traditional)" zh_TW 1028
  # ${COMMAND} "Dansk" da_DK 1030
  ${COMMAND} "Deutsch" de_DE 1031
  ${COMMAND} "English" en_EN 1033
  ${COMMAND} "Español" es_ES 1034
  ${COMMAND} "Euskara" eu_EU 1069
  ${COMMAND} "Français" fr_FR 1036
  ${COMMAND} "Galego" gl_ES 1036
  ${COMMAND} "Hebrew" he_IL 1037
  ${COMMAND} "Italiano" it_IT 1057
  ${COMMAND} "Japanese" ja_JP 1041
  # ${COMMAND} "Korean" ko 1042
  ${COMMAND} "Magyar" hu_HU 1038
  # ${COMMAND} "Nederlands" nl_NL 1043
  ${COMMAND} "Norsk (Bokmål)" nb_NO 1044
  ${COMMAND} "Norsk (Nynorsk)" nn_NO 2068
  ${COMMAND} "Português" pt_PT 1046
  ${COMMAND} "Polski"  pl_PL 1045
  ${COMMAND} "Româna" ro_RO 1048
  # ${COMMAND} "Russian" ru_RU 1049
  # ${COMMAND} "Slovencina" sk_SK 1060
  # ${COMMAND} "Slovenšcina" sl_SI 1051
  ${COMMAND} "Suomi" fi_FI 1035
  # ${COMMAND} "Svenska" sv_SE 1053
  # ${COMMAND} "Türkçe" tr_TR 1055
  # ${COMMAND} "Walon" wa_BE 1055

!macroend
