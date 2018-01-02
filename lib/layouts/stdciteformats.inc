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
# Author: Richard Heck <rgheck@comcast.net>
#         Jürgen Spitzmüller <spitz@lyx.org>

Format 66

CiteFormat default
	#
	# Translatable bits
	#
	_pptext pp.
	_edtext ed.
	_edstext eds.
	_voltext vol.
	_numtext no.
	_in in
	# The following are handled by BiblioInfo. Note that preceding and trailing spaces matter
	_namesep , [[separate author names in citation, except for last name]]
	_lastnamesep , and [[separate name of last author in citation]]
	_pairnamesep  and [[separate two authors in citation]]

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
	!pages {%pages%[[, %_pptext% %pages%]]}
	# ed. or eds.
	!makeed {%ifmultiple:editor%[[%_edstext%]][[%_edtext%]]}
	# author or editor, as fullnames, following the schemes above
	!authoredit {%fullnames:author%[[%fullnames:author%, ]][[{%fullnames:editor%[[%fullnames:editor%, %!makeed%, ]]}]]}
	# "vol. 1, no.
	!volnum {%volume%[[ %_voltext% %volume%, {%number%[[%_numtext% %number%]]}]]}
	!quotetitle "%title%"
	!emphtitle {!<i>!}%title%{!</i>!}
	!emphjournal {!<i>!}{%journal%[[%journal%]][[{%journaltitle%[[%journaltitle%]]}]]}{!</i>!}
	!location {%address%[[%address%: ]][[{%location%[[%location%: ]]}]]}

	#
	# Entry types. Note that final punctuation will be added later, if needed.
	#
	!insomething %fullnames:author%, %!quotetitle%, %_in%{%fullbynames:editor%[[ %fullbynames:editor%, %!makeed%,]]} {!<i>!}%booktitle%{!</i>!}%!volnum%{%edition%[[ %edition%]]} (%!location%%publisher%, %year%)%!pages%{%note%[[. %note%]]}

	article %fullnames:author%, %!quotetitle%, %!emphjournal% {%volume%[[ %volume%{%number%[[, %number%]]}]]} (%year%)%!pages%{%note%[[. %note%]]}

	!booklike %!authoredit%%!emphtitle%%!volnum%{%edition%[[ %edition%]]} (%!location%%publisher%, %year%){%note%[[. %note%]]}
	book %!booklike%
	collection %!booklike%
	proceedings %!booklike%

	incollection %!insomething%
	inproceedings %!insomething%

	!theses %fullnames:author%, %title% (%!location%{%school%[[%school%]][[%institution%]]}, %year%){%note%[[. %note%]]}
	thesis %!theses%
	phdthesis %!theses%
	mastersthesis %!theses%
End
