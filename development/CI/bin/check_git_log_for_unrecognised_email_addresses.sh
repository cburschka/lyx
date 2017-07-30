#!/bin/bash
#
#
CMD=$1
SINCE_DATE=2015-01-01
FORMAT='%h %ai name=%an email=%ae'
SCRIPT=$0

function show_help() {
    cat <<EOF
SCRIPT=$SCRIPT

This script filters the git log for commits whose
author's e-mail is _not_ on @lyx.org.

This script can be used to check that the LyX developers have
correctly configured their git to provide an e-mail address that's in
the @lyx.org domain. This is needed for proper functioning of the
e-mails being automatically sent to lyx-cvs@lyx.org.

Examples:
	# List contributions from non-@lyx.org-addresses since 2015-01-01
	$SCRIPT --list

    # Compute hash (SHA-512) of list of contributors
	$SCRIPT --compute-hash

    # Check hash of list of contributors against expected hash.
    # This scripts exist with a non-zero error code if hash does not match.
	$SCRIPT --check-hash \
        1330d9fca5e385e9de8933cbf6d391222207b2f6af1cf7ca3175babfd4e5ab46b024ff2d98c7c8630b362be506da9376f9262a524755f9acf655d83dcce8a564

    # Show help text
	$SCRIPT -h

Background:
See e-mail thread on developers@lyx.org started by Scott 2017-07-30.
EOF
}


function fail() {
    printf "An error occured: %s\n" "$*"
    exit 1
}

function list_filtered_git_log() {
    git log --since=$SINCE_DATE --format="$FORMAT" \
        | grep -v 'lyx\.org$'
}

function compute_current_hash() {
    FILTERED_LOG=$( list_filtered_git_log )
    echo "$FILTERED_LOG" | shasum -a 512 | cut -d ' ' -f 1
}

case "$CMD" in
    "-h"|"--help")
        show_help
        exit 0
        ;;

    "-l"|"--list")
        list_filtered_git_log
        ;;

    "--compute-hash")
        CURRENT_HASH=$( compute_current_hash )
        printf "Current hash: %s\n" "$CURRENT_HASH"
        exit 0
        ;;

    "-c"|"--check-hash")
        DESIRED_HASH=$2
        CURRENT_HASH=$( compute_current_hash )
        if [[ "$CURRENT_HASH" == "$DESIRED_HASH" ]]; then
            printf "Hash matches current hash\n:"
            printf "  Current: %s\n" "$CURRENT_HASH"
            exit 0;
        else
            printf "Hash mismatch:\n"
            printf "  Desired: %s\n" "$DESIRED_HASH"
            printf "  Current: %s\n" "$CURRENT_HASH"
            printf "Latest log:\n%s\n" "$FILTERED_LOG"
            exit 1
        fi
        ;;

    *)
        fail "Unrecognised command(s), give '-h' for help: '$*'"
        ;;
esac

