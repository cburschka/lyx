# Standard formats for citations.
#
# Author: Richard Heck <rgheck@comcast.net>

Format 26

CiteFormat 
	article %author%, "%title%", {!<i>!}%journal%{!</i>!} {%volume%[[ %volume%{%number%[[, %number%]]}]]} (%year%){%pages%[[, pp. %pages%]]}.{%note%[[ %note%]]}
	
	book {%author%[[%author%, ]][[{%editor%[[%editor%, %ed_text%, ]]}]]}{!<i>!}%title%{!</i>!}{%volume%[[ vol. %volume%]][[{%number%[[no. %number%]]}]]}{%edition%[[%edition%]]} ({%address%[[%address%: ]]}%publisher%, %year%).{%note%[[ %note%]]}

	incollection %author%, \"%title%\", in{%editor%[[ %editor%, %ed_text%,]]} {!<i>!}%booktitle%{!</i>!}{%volume%[[ vol. %volume%]][[{%number%[[no. %number%]]}]]}{%edition%[[%edition%]]} ({%address%[[%address%: ]]}%publisher%, %year%){%pages%[[, pp. %pages%]]}.{%note%[[ %note%]]}

	thesis %author%, %title% ({%address%[[%address%: ]]}%school%, %year%).{%note%[[ %note%]]}
End
