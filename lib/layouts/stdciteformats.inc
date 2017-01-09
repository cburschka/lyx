# Standard formats for citations.
#
# Author: Richard Heck <rgheck@comcast.net>
#         Jürgen Spitzmüller <spitz@lyx.org>

Format 63

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
	!firstnameform %surname%, %prename%
	# Scheme of other authors in the bibliography
	!othernameform %surname%, %prename%
	# Scheme of the first name in later parts (such as book editor)
	!firstbynameform %prename% %surname%
	# Scheme of other authors in later parts (such as book editor)
	!otherbynameform %prename% %surname%
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
