#!/usr/bin/wish8.5

wm attributes . -topmost 1

image create photo .stop -format GIF -file stop.gif
button .stop_autotests -image .stop -text "STOP LyX Tests" -compound left -command {
    set status 0
    if {[catch {exec killall --wait run-tests.sh single-test.sh lyx} results]} {
	if {[lindex $::errorCode 0] eq "CHILDSTATUS"} {
	    set status [lindex $::errorCode 2]
	} else {
	    # Some kind of unexpected failure
	}
    }
    exit
}
pack .stop_autotests
