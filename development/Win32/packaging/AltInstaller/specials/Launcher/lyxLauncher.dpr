program lyxLauncher;
// this program opens LyX and hides its console window
// author: Uwe Stöhr

{The problematic is the following:
 When the lyx.exe is started, also a console window is shown.
 But closing the console window, also closes LyX,
 therefore the console will be hidden by this program.}

{$APPTYPE CONSOLE}

uses
  Windows, SysUtils, ShellApi, Dialogs;

{$R *.res}

var Path,FileName : string;
    hConsole : THandle;


procedure StartLyX(hConsole: THandle; FileName,Path: string);
// starts LyX

var Params : PChar;
    hLyX : THandle;
    Folder : string;
begin

 // if a filename is given, quote it to avoid the path with spaces problem
 if FileName <> '' then
  Params:= PChar('"' + FileName + '"')
 else
  Params:= nil;

 // get folder of the lyx.exe
 Folder:= StringReplace(Path,'\lyx.exe','',[rfIgnoreCase]);
 // quote path to avoid the path with spaces problem
 Folder:= '"' + Folder + '"';
 
 // start LyX
 hLyX:= ShellExecute(hConsole,PChar('open'),PChar(Path),
                     Params,PChar(Folder),SW_SHOWNORMAL);
 if hLyX = ERROR_FILE_NOT_FOUND  then
 begin
  MessageDLG('The file'#13#10 + Path + #13#10
             + 'could not be found!',mtError,[mbOK],0);
  exit;
 end;
 if hLyX = SE_ERR_ACCESSDENIED  then
 begin
  MessageDLG('Windows denied access on the file'#13#10 + Path,
             mtError,[mbOK],0);
  exit;
 end;

end; // end procedure


procedure HideWindow(ProgWin: string);
// hides a given program window

var Handle : THandle;
begin

 // find handle of the program window
 // Repeat until the handle is available
 // because Lyx needs some time to start
 Repeat
  Sleep(1000); // wait 1 second to give LyX time to open
  Handle := FindWindow(nil,Pchar(ProgWin));
 Until Handle <> 0;

 // hide the window from taskbar
 ShowWindow(Handle, SW_HIDE);

end; //end procedure


begin //begin program

 //Read path to this application
 Path:= ParamStr(0);

 //get handle of this console window
 // This application is called by the lyx.bat with the name "LyX"
 hConsole := FindWindow(nil,Pchar('LyX'));
 // hide the window of this console application
 ShowWindow(hConsole,SW_HIDE);

 // do the same for the real name of this console application
 // because it depends on the computer speed if the "LyX" console window
 // was closed before it could be processed
 hConsole := FindWindow(nil,Pchar(Path));
 ShowWindow(hConsole,SW_HIDE);

 // replace in the path "lyxLauncher.exe" by "lyx.exe"
 Path:= StringReplace(Path, 'lyxLauncher', 'lyx', [rfIgnoreCase]);

 // read given filename of a LyX-document
 FileName:= ParamStr(1);

 // start LyX
 StartLyX(hConsole,FileName,Path);

 // hide console window of lyx.exe
 HideWindow(Path);

end. //end program

