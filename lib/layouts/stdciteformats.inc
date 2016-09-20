# Standard formats for citations.
#
# Author: Richard Heck <rgheck@comcast.net>

Format 60

CiteFormat default
	# translatable bits
	_pptext pp.
	_edtext ed.
	_voltext vol.
	_numtext no.
	_in in

	# macros
	!pages {%pages%[[, %_pptext% %pages%]]}
	!authoredit {%author%[[%author%, ]][[{%editor%[[%editor%, %_edtext%, ]]}]]}
	!volnum {%volume%[[ %_voltext% %volume%, {%number%[[%_numtext% %number%]]}]]}
	!quotetitle "%title%"
	!emphtitle {!<i>!}%title%{!</i>!}
	!emphjournal {!<i>!}{%journal%[[%journal%]][[{%journaltitle%[[%journaltitle%]]}]]}{!</i>!}
	!location {%address%[[%address%: ]][[{%location%[[%location%: ]]}]]}

	!insomething %author%, %!quotetitle%, %_in%{%editor%[[ %editor%, %_edtext%,]]} {!<i>!}%booktitle%{!</i>!}%!volnum%{%edition%[[ %edition%]]} (%!location%%publisher%, %year%)%!pages%.{%note%[[ %note%]]}

	article %author%, %!quotetitle%, %!emphjournal% {%volume%[[ %volume%{%number%[[, %number%]]}]]} (%year%)%!pages%.{%note%[[ %note%]]}

	book %!authoredit%%!emphtitle%%!volnum%{%edition%[[ %edition%]]} (%!location%%publisher%, %year%).{%note%[[ %note%]]}

	incollection %!insomething%
	inproceedings %!insomething%

	!theses %author%, %title% (%!location%{%school%[[%school%]][[%institution%]]}, %year%).{%note%[[ %note%]]}
	thesis %!theses%
	phdthesis %!theses%
	mastersthesis %!theses%
End
