if not exist "%~dpn1~%~x1" goto :go
pdfclose --file  "%~dpn1~%~x1"
del "%~dpn1~%~x1"
:go
copy %1 "%~dpn1~%~x1"
pdfopen --file "%~dpn1~%~x1"
exit