# Standard formats for citations.
#
# Author: Richard Heck <rgheck@comcast.net>

Format 35

CiteFormat
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
	!emphjournal {!<i>!}%journal%{!</i>!}

	!insomething %author%, %!quotetitle%, %_in%{%editor%[[ %editor%, %_edtext%,]]} {!<i>!}%booktitle%{!</i>!}%!volnum%{%edition%[[%edition%]]} ({%address%[[%address%: ]]}%publisher%, %year%)%!pages%.{%note%[[ %note%]]}

	article %author%, %!quotetitle%, %!emphjournal% {%volume%[[ %volume%{%number%[[, %number%]]}]]} (%year%)%!pages%.{%note%[[ %note%]]}

	book %!authoredit%%!emphtitle%%!volnum%{%edition%[[%edition%]]} ({%address%[[%address%: ]]}%publisher%, %year%).{%note%[[ %note%]]}

	incollection %!insomething%
	inproceedings %!insomething%

	thesis %author%, %title% ({%address%[[%address%: ]]}%school%, %year%).{%note%[[ %note%]]}
End
