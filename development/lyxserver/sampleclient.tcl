# this one avoids absolute path names: \
exec wish -f $0 "$@"
# =========================================================================
#   File: sampleclient.tcl, chb, Sun 05.11.1995 (19:24)
#   sampleclient.tcl,v 1.1.1.1 1996/08/19 14:39:38 larsbj Exp
#   This file contains examples for communicating to LyX via the
#   LyXserver interface. It contains a panel of shortcuts to
#   demonstrate how users can define their own macros. It also shows
#   how new commands can be built into LyX using the 'notify' mechanism.
# =========================================================================


# --- 1. check cmdargs (pipename) -----------------------------------------

if { [llength $argv] > 0 } {
    set inpipe  [lrange $argv 0 0].in
    set outpipe [lrange $argv 0 0].out
} else {
    set inpipe  $env(HOME)/.lyxpipe.in
    set outpipe $env(HOME)/.lyxpipe.out
}


# --- 2. check if pipe is there; if not inform about lyxrc ----------------

proc inform {} {
    global inpipe outpipe
    puts "$inpipe or $outpipe not found"
    puts "Either LyX is not running or it is not configured"
    puts "to start the server. To start the server, insert"
    puts "     \serverpipe \"<pipename>\""
    puts "in your .lyxrc"
    exit
}

if { ![file exists $inpipe] }           inform
if { "[file type $inpipe]" != "fifo" }  inform
if { ![file exists $outpipe] }          inform
if { "[file type $outpipe]" != "fifo" } inform


# --- 3. check if addinput is defined, if not; fake it --------------------

if { [info command addinput] == "" } {
    puts "Warning: this wish does not have the addinput command"
    puts "         reading back information from LyX is disabled"

    proc addinput {opt file cmd} {}
    proc removeinput {file} {}
}


# --- 4. define addinput callbacks ----------------------------------------

set outbuf ""

proc outputhandler {fd} {
    global lyxnotify
    gets $fd outbuf
    set type [lrange [split $outbuf :] 0 0]
    if { "$type" == "NOTIFY" } {
	set val [lrange [split $outbuf :] 1 1]
	# puts "$val"
	if { [catch { set lyxnotify($val) }] == 0 } {
	    eval $lyxnotify($val)
	}
    }
}

set outfd [open $outpipe r]

proc out_on {} {
    global outfd
    addinput -read $outfd "outputhandler %F"
}

proc out_off {} {
    global outfd
    removeinput $outfd
}

out_on


# --- 5. make windows -----------------------------------------------------

toplevel .t
button .t.b1 -text "->"   -command {lyxsend_form {\\rightarrow }}
button .t.b2 -text "<-"   -command {lyxsend_form {\\leftarrow }}
button .t.b3 -text "-->"  -command {lyxsend_form {\\longrightarrow }}
button .t.b4 -text "<--"  -command {lyxsend_form {\\longleftarro w}}
button .t.b5 -text "<->"  -command {lyxsend_form {\\leftrightarrow }}
button .t.b6 -text "<-->" -command {lyxsend_form {\\longleftrightarrow }}
button .t.b7 -text "..."  -command {lyxsend_tex  {\\ldots }}
label  .t.t  -text "TeX Arrows:"

pack .t.t  -fill x -in .t
pack .t.b1 -fill x -in .t
pack .t.b2 -fill x -in .t
pack .t.b3 -fill x -in .t
pack .t.b4 -fill x -in .t
pack .t.b5 -fill x -in .t
pack .t.b6 -fill x -in .t
pack .t.b7 -fill x -in .t

label  .func -text "New Functions"
button .xp   -text "Transpose Chars" -command "lyx_transpose"
button .dw   -text "Delete word"     -command "lyx_delword"
button .sp   -text "Save position"   -command "lyx_savepos"
button .rp   -text "Restore pos"     -command "lyx_restorepos"
button .gi   -text "Get Buffer info" -command "lyx_getinfo"
button .q    -text "Quit"            -command "exit"

pack .func -fill x
pack .xp   -fill x
pack .dw   -fill x
pack .sp   -fill x
pack .rp   -fill x
pack .gi   -fill x
pack .q    -fill x


# --- 6. functions --------------------------------------------------------

# ---
#  Format for communication:
#
#    client->LyX:          "LYXCMD:<client>:<command>:<argument>" >$inpipe
#    LyX->client (answer): "INFO:<client>:<command>:<data>"       <$outpipe
#    LyX->client (notify): "NOTIFY:<key-sequence>"                <$outpipe
#    (notifies are asynchroneous and are handled by 'outputhandler'
#     above)
# ---

proc lyxsend_form {string} {
    global inpipe outpipe outfd
#    out_off
#    exec echo "LYXCMD:sampleclient:get-latex:" >$inpipe
#    gets $outfd buf
#    set chr [lrange [split $buf :] 3 3]
#    out_on
#
#    if { "$chr" != "F" } {
#	exec echo "LYXCMD:sampleclient:set-formula:" >$inpipe
#    }
#    exec echo "LYXCMD:sampleclient:insert-self:$string" >$inpipe
#    if { "$chr" != "F" } {
#	exec echo "LYXCMD:sampleclient:set-formula:" >$inpipe
#    }

    exec echo "LYXCMD:sampleclient:insert-inset-formula-latex-del:$string" >$inpipe

}

proc lyxsend_tex {string} {
    global inpipe outpipe outfd
#    out_off
#    exec echo "LYXCMD:sampleclient:get-latex:" >$inpipe
#    gets $outfd buf
#    set chr [lrange [split $buf :] 3 3]
#    out_on
#
#   if { "$chr" != "L" } {
#	exec echo "LYXCMD:sampleclient:set-tex:" >$inpipe
#    }
#    exec echo "LYXCMD:sampleclient:insert-self:$string" >$inpipe
#    if { "$chr" != "L" } {
#	exec echo "LYXCMD:sampleclient:set-tex:" >$inpipe
#    }
    exec echo "LYXCMD:sampleclient:insert-inset-latex-del:$string" >$inpipe

}

proc lyx_transpose {} {
    global inpipe outpipe outfd
    out_off
    exec echo "LYXCMD:sampleclient:char-after:" >$inpipe
    gets $outfd buf
    set chr [lrange [split $buf :] 3 3]
    if { "$chr" == "EOF" } {out_on; return}
    if { [string length $chr] > 1 } {
	set chr [string range $chr 1 1]
    }
    exec echo "LYXCMD:sampleclient:delete-forward:" >$inpipe
    exec echo "LYXCMD:sampleclient:left:" >$inpipe
    if { "$chr" == "{ }" } {
	exec echo "LYXCMD:sampleclient:insert-self: " >$inpipe
    } else {
	exec echo "LYXCMD:sampleclient:insert-self:$chr" >$inpipe
    }
    out_on
}

set lyxnotify(C-t) lyx_transpose

proc lyx_delword {} {
    global inpipe outpipe outfd
    exec echo "LYXCMD:sampleclient:word-right-select:" >$inpipe
    exec echo "LYXCMD:sampleclient:left-select:" >$inpipe
    # !!! should check for end of line here:
    #     if ( char-after == char-in-word ) right;
    exec echo "LYXCMD:sampleclient:delete-forward:" >$inpipe
}

set lyxnotify(M-d) lyx_delword

set posx ""
set posy ""

proc lyx_savepos {} {
    global posx posy inpipe outpipe outfd

    out_off
    exec echo "LYXCMD:sampleclient:get-xy:" >$inpipe
    gets $outfd buf
    set arg [lrange [split $buf :] 3 3]
    # TCL is SO pathetic!
    set arg [string range $arg 1 [expr [string length $arg]-2]]
    set posx [lrange $arg 0 0]
    set posy [lrange $arg 1 1]
    out_on
}

set "lyxnotify({C-x slash})" lyx_savepos

proc lyx_restorepos {} {
    global posx posy inpipe
    if { "$posx" == "" } return
    exec echo "LYXCMD:sampleclient:set-xy:$posx $posy" >$inpipe
    exec echo "LYXCMD:sampleclient:recenter:" >$inpipe
}

set "lyxnotify({C-x j})" lyx_restorepos

proc lyx_getinfo {} {
    global inpipe outpipe outfd
    out_off
    exec echo "LYXCMD:sampleclient:get-name:" >$inpipe
    gets $outfd buf
    set fname [lrange [split $buf :] 3 3]
    exec echo "LYXCMD:sampleclient:get-layout:" >$inpipe
    gets $outfd buf
    set layout [lrange [split $buf :] 3 3]
    exec echo "LYXCMD:sampleclient:get-font:" >$inpipe
    gets $outfd buf
    set fnt [lrange [split $buf :] 3 3]
    exec echo "LYXCMD:sampleclient:get-latex:" >$inpipe
    gets $outfd buf
    set ltx [lrange [split $buf :] 3 3]
    exec echo "LYXCMD:sampleclient:get-xy:" >$inpipe
    gets $outfd buf
    set arg [lrange [split $buf :] 3 3]
    set arg [string range $arg 1 [expr [string length $arg]-2]]

    puts "---------------"
    puts "Buffer:         $fname"
    puts "Current layout: $layout"
    puts "Font flag:      $fnt"
    puts "Latex flag:     $ltx"
    puts "Position:       $arg"

    out_on
}


# === End of file: sampleclient.tcl =======================================
