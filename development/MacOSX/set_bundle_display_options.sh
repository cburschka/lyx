# Copied from Lyx-Mac-binary-release.sh
# TODO: In order to avoid having two versions,
# call this file from Lyx-Mac-binary-release.sh

FILE=$1
LyxName=$2
IMAGEPATH=$3
X_BOUNDS=$4
Y_BOUNDS=$5

# Creates the structure
mkdir -p "${FILE}"
touch "${FILE}/${LyxName}.app"
touch "${FILE}/Applications"

# Copy the background and make sure the file is visible
# For some reason (OS X cache?), we need to remove the previous
# file
rm -f "$FILE/background.png"
cp "$IMAGEPATH" "$FILE/background.png"
xattr -c "$FILE/background.png"

Y_POSITION=$((Y_BOUNDS - 65))
Y_BOUNDS=$((Y_BOUNDS + 20))
LYX_X_POSITION=$((X_BOUNDS / 4))
LYX_Y_POSITION=$Y_POSITION
APP_X_POSITION=$((3 * X_BOUNDS / 4))
APP_Y_POSITION=$Y_POSITION
WITH_DOCUMENTS=$(test -d "${FILE}/Documents" && echo true || echo false)
osascript <<-EOF
tell application "Finder"
    set f to POSIX file ("$FILE" as string) as alias
    set image to POSIX file ("$IMAGEPATH" as string) as alias
    tell folder f
        open
        tell container window
            set toolbar visible to false
            set statusbar visible to false
            set current view to icon view
            delay 1 -- sync
            set the bounds to {20, 50, $X_BOUNDS, $Y_BOUNDS}
        end tell
        delay 1 -- sync
        set icon size of the icon view options of container window to 64
        set arrangement of the icon view options of container window to not arranged
        if ${WITH_DOCUMENTS} then
           set position of item "Documents" to {$LYX_X_POSITION,0}
        end if
        set position of item "${LyxName}.app" to {$LYX_X_POSITION,$LYX_Y_POSITION}
        set position of item "Applications" to {$APP_X_POSITION,$APP_Y_POSITION}
        set background picture of the icon view options of container window to file "background.png"
        set the bounds of the container window to {0, 0, $X_BOUNDS, $Y_BOUNDS}
        update without registering applications
        delay 5 -- sync
        close
    end tell
    delay 5 -- sync
end tell
EOF

# Hide again the background
chflags hidden "$FILE/background.png"

