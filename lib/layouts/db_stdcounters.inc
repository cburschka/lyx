# Author : José Matos <jamatos@lyx.org>

# This include file contains all the counters that are defined as standard
# in docbook LyX layouts.

Counter
	Name                 sect1
	Within               chapter
End

Counter
	Name                 sect2
	Within               sect1
End

Counter
	Name                 sect3
	Within               sect2
End

Counter
	Name                 sect4
	Within               sect3
End

Counter
	Name                 sect5
	Within               sect4
End
