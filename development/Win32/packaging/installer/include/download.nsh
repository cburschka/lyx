/*

Support code for file downloads

*/

!macro DownloadFile RET ID FILENAME

  #Downloads a file using the InetLoad plug-in (HTTP or FTP)
  
  #RET = Return value (OK if succesful)
  #ID = Name of the download in settings.nsh
  #FILENAME = Location to store file

  #Try first mirror server
  InetLoad::load "${DOWNLOAD_${ID}}" "$PLUGINSDIR\${FILENAME}" /END
  Pop ${RET} #Return value (OK if succesful)

  ${if} ${RET} != "OK"
    #Download failed, try second mirror server
    InetLoad::load "${DOWNLOADALT_${ID}}" "$PLUGINSDIR\${FILENAME}" /END
    Pop ${RET}
  ${endif}

!macroend
