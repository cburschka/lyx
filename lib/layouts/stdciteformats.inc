# Standard formats for bibliography entries.
#
# This defines how LyX displays bibliographic information in the GUI
# as well as in text/xhtml output. The format of citation references
# is defined in the *.citeengines files, which might override the
# default formatting defined here.
#
# This file is included by the citation engines, so there is no need
# to include it in individual classes.
#
# Author: Richard Kimberly Heck <rikiheck@lyx.org>
#         Jürgen Spitzmüller <spitz@lyx.org>

Format 90

CiteFormat default
	#
	# Translatable bits (need to be marked by _ prefix, if translated to the GUI language,
	# or B_, if translated to the buffer language)
	# Note that preceding and trailing spaces matter.
	#
	B_pptext pp.
	B_edtext ed.
	B_edstext eds.
	B_voltext vol.
	B_numtext no.
	B_in in
	# The following are handled by BiblioInfo. Note that preceding and trailing spaces matter
	B_namesep , [[separate author names in citation, except for last name]]
	B_lastnamesep , and [[separate name of last author in citation]]
	B_pairnamesep  and [[separate two authors in citation]]

	#
	# Macros
	#
	# Scheme of the first author in the bibliography
	!firstnameform {%prefix%[[%prefix% ]]}%surname%{%suffix%[[, %suffix%]]}{%prename%[[, %prename%]]}
	# Scheme of other authors in the bibliography
	!othernameform {%prefix%[[%prefix% ]]}%surname%{%suffix%[[, %suffix%]]}{%prename%[[, %prename%]]}
	# Scheme of the first name in later parts (such as book editor)
	!firstbynameform %prename% {%prefix%[[%prefix% ]]}%surname%{%suffix%[[, %suffix%]]}
	# Scheme of other authors in later parts (such as book editor)
	!otherbynameform %prename% {%prefix%[[%prefix% ]]}%surname%{%suffix%[[, %suffix%]]}
	# Scheme of authors in citation references
	!citenameform {%prefix%[[%prefix% ]]}%surname%
	# pagination
	!pages {%pages%[[, %B_pptext% %pages%]]}
	# ed. or eds.
	!makeed {%ifmultiple:editor%[[%B_edstext%]][[%B_edtext%]]}
	# author or editor, as fullnames, following the schemes above
	!authoredit {%fullnames:author%[[%fullnames:author%, ]][[{%fullnames:editor%[[%fullnames:editor%, %!makeed%, ]]}]]}
	# "vol. 1, no.
	!volnum {%volume%[[ %B_voltext% %volume%, {%number%[[%B_numtext% %number%]]}]]}
	!quotetitle "%title%"
	!emphtitle {!<i>!}%title%{!</i>!}
	!emphjournal {!<i>!}{%journal%[[%journal%]][[{%journaltitle%[[%journaltitle%]]}]]}{!</i>!}
	!locpub {%publisher%[[{%address%[[%address%: ]][[{%location%[[%location%: ]]}]]}%publisher%, ]][[{%address%[[%address%, ]][[{%location%[[%location%, ]]}]]}]]}
	!locinst {%school%[[{%address%[[%address%: ]][[{%location%[[%location%: ]]}]]}%school%, ]][[{%institution%[[{%address%[[%address%: ]][[{%location%[[%location%: ]]}]]}%institution%, ]][[{%address%[[%address%, ]][[{%location%[[%location%, ]]}]]}]]}]]}

	#
	# Entry types. Note that final punctuation will be added later, if needed.
	#
	!insomething %fullnames:author%, %!quotetitle%, %B_in%{%fullbynames:editor%[[ %fullbynames:editor%, %!makeed%,]]} {!<i>!}%booktitle%{!</i>!}%!volnum%{%edition%[[ %edition%]]} (%!locpub%%year%)%!pages%{%note%[[. %note%]]}

	article %fullnames:author%, %!quotetitle%, %!emphjournal% {%volume%[[ %volume%{%number%[[, %number%]]}]]} (%year%)%!pages%{%note%[[. %note%]]}

	!booklike %!authoredit%%!emphtitle%%!volnum%{%edition%[[ %edition%]]} (%!locpub%%year%){%note%[[. %note%]]}
	book %!booklike%
	collection %!booklike%
	proceedings %!booklike%

	incollection %!insomething%
	inproceedings %!insomething%

	!theses %fullnames:author%, %title% (%!locinst%%year%){%note%[[. %note%]]}
	thesis %!theses%
	phdthesis %!theses%
	mastersthesis %!theses%
End
