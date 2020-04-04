@echo off
set "psCommand="(new-object -COM 'Shell.Application').BrowseForFolder(0,'Please choose the imagemagick folder.',0,0).self.path""
for /f "usebackq delims=" %%I in (`powershell %psCommand%`) do set "folder=%%I"

if not exist "%folder%\magick.exe" (
  echo selected wrong directory, magick.exe does not exist
  pause
  exit
)

rd /s /q "%folder%\images"
rd /s /q "%folder%\www"
del "%folder%\compare.exe"
del "%folder%\composite.exe"
del "%folder%\conjure.exe"
del "%folder%\convert.exe"
del "%folder%\ffmpeg.exe"
del "%folder%\identify.exe"
del "%folder%\IMDisplay.exe"
del "%folder%\mogrify.exe"
del "%folder%\montage.exe"
del "%folder%\stream.exe"

@echo done
pause