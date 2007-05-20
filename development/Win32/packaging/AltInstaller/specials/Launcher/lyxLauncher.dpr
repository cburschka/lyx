program lyxLauncher;
// this program opens LyX and hides its console window
// author: Uwe Stöhr

{The problematic is the following:
 When the lyx.exe is started, also a console window is shown.
 But closing the console window, also closes LyX,
 therefore the console will be hidden by this program.}

{$APPTYPE CONSOLE}

uses
  Windows, SysUtils, ShellApi, Forms, Dialogs;

var Path : string;
    hLyX : THandle;

procedure HideWindow(ProgWin: string);
// hides a given program window
var
  Handle : THandle;
begin

 // find handle of the program window
 // Repeat until the handle is available
 // because Lyx needs some time to start
 Repeat
  Handle := FindWindow(nil,Pchar(ProgWin));
 Until Handle <> 0;
 // minimize the window
 // SendMessage(Handle, WM_SYSCOMMAND, SC_MINIMIZE, 1);
 // hide the window from taskbar
 ShowWindow(Handle, SW_HIDE);

end; //end procedure

procedure ReadPath(FileName: string; LaunchName: string;
                   ExecName: string; var PathR: string);
// reads the path to the lyx.exe from a given text file
var InString : string;
    FileSize,Index,Last : Int64;
    hFile : THandle;
    PInString : PChar;

begin

 try //try..except for procedure
  // open the text file
  hFile:= Windows.CreateFile(PChar(FileName),GENERIC_READ,0,nil,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,0);
  if hFile= INVALID_HANDLE_VALUE then
  begin
   MessageDlg('The file "' + FileName + '" could not be found!',
               mtError,[mbOK],0);
   exit;
  end;
  
  try //try..finally for hFile
   FileSize:= FileSeek(hFile,0,2); //get file size
   if FileSize = -1 then
    RaiseLastOSError;

   //move file content to InString
   FileSeek(hFile,0,0);
   SetLength(InString,FileSize);
   PInString:= PChar(InString);
   FileRead(hFile,PInString^,FileSize);

   //search the string backwards for the first appearance of ":"
   Index:= FileSize;
   Repeat
    dec(Index);
    if InString[Index] = ':' then
     Break;
   Until (InString[Index] = #10) or (Index = 1);

   //if the last line of lyx.bat doesn't contain a ":" (a path)
   if (InString[Index] = #10) or (Index = 1) then
   begin
    MessageDlg('The file lyx.bat is corrupted!',mtError,[mbOK],0);
    exit;
   end;

   //jump before the ":" to the drive letter
   dec(Index);
   //search for the LaunchName = end of the path
   Last:= Pos(LaunchName,InString);
   //the InString contains between Index and Last the wanted path
   PathR:= Copy(InString,Index,Last - Index);
   //attach LyX's executable to the path
   PathR:= Path + ExecName;

  finally //close the text file
   Windows.CloseHandle(hFile);
  end; //end finally

 except //when an error occurred somewhere in the procedure
  MessageDlg('The file "' + FileName + '" is corrupted!',mtError,[mbOK],0);
 end; //end except

end; //end procedure


begin //begin program

 //hide the window of this application
 ShowWindow(Application.Handle,SW_HIDE);

 // read path of the lyxLauncher.exe from the file lyx.bat 
 ReadPath('lyx.bat', 'lyxLauncher.exe', 'lyx.exe', Path);

 // start LyX
 hLyX:= ShellExecute(Application.Handle,PChar('open'),
                     PChar(Path),nil,nil,SW_SHOWNORMAL);
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

 // hide console window of lyx.exe
 HideWindow(Path);

end. //end program

