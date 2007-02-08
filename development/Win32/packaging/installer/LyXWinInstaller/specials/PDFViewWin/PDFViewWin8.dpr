program PDFViewWin8;
// this program opens and closes PDF-files with Acrobat 8
// and with Adobe Reader 8

{The problematic is the following:
 A PDF-file should be modified while it is opened with Acrobat.
 This is not possible because Acrobat understands itself as editor, not as
 reader and therefore opens PDFs always with write access, so that other
 programs cannot modifiy them.
 The idea to solve the problem is the following:
 The file that should be shown in Acrobat is copied and then renamed -
 the suffix "-preview" is attached. The renamed copy is opened by Acrobat
 while the unrenamed version can be modified. When the modified version should
 be displayed, the eventually opened renamed version is closed in Acrobat and
 the modified version is copied, renamed and opened in Acrobat.
 To open/close files in Acrobat, the programs "pdfopen" and "pdfclose",
 written by Fabrice Popineau, which are used in PDFViewWin7, cannot be used
 for Acrobat 8. In this program the PDFs are closed by closing its program
 window.
 In Acrobat 8 it is not possible to go to the position of a reopened modified
 file were it was viewed the last time.}

{$APPTYPE CONSOLE}

uses
  Windows,SysUtils,ShellApi,Forms,Messages;

var Input,InputNew,FileName : string;
    FileTest : boolean;
    InputNewName : PChar;
    i : integer;

function RenameFile(const OldName, NewName: string): boolean;
//renames files, taken from
//http://www.dsdt.info/tipps/?id=128&search=RenameFile
var
  sh: TSHFileOpStruct;
begin
  sh.Wnd := Application.Handle;
  sh.wFunc := fo_Rename;
  //terminate with null byte to set list ending
  sh.pFrom := PChar(OldName + #0);
  sh.pTo := PChar(NewName + #0);
  sh.fFlags := fof_Silent or fof_MultiDestFiles;
  Result:=ShFileOperation(sh)=0;
end; //end function }


procedure KillProcess(WindowHandle: HWND);
//kills a process given by its handle
var
  processID: INTEGER;
  processHandle: THandle;
  Result: DWORD;
begin
 SendMessageTimeout(WindowHandle, WM_CLOSE, 0, 0,
                    SMTO_ABORTIFHUNG or SMTO_NORMAL, 5000, Result);
 if isWindow(WindowHandle) then //when window exists
 begin
  PostMessage(WindowHandle, WM_QUIT, 0, 0);
  //get process ID of the window
  GetWindowThreadProcessID(WindowHandle, @processID);
  if processID <> 0 then //when correct ID
  begin
   //get process handle
   processHandle:= OpenProcess(PROCESS_TERMINATE or PROCESS_QUERY_INFORMATION,
                                False, processID);
   if processHandle <> 0 then //terminate the process when handle was found
   begin
    TerminateProcess(processHandle, 0);
    CloseHandle(ProcessHandle);
   end;
  end;
 end; //end if isWindow
end; //end procedure


begin //begin program

 //Read given filename
 Input:= ParamStr(1);
 //InputNew = original filename with ending "-preview" (e.g. test-preview.pdf)
 InputNew:= copy(Input,1,Length(Input)-4); //remove ".pdf"
 InputNew:= InputNew+'-preview.pdf';
 //check if renamed file exists
 FileTest:= FileExists(InputNew);
 if FileTest = true then //when file exists
 begin
  //extract filename
  For i:= Length(InputNew) downto 1 do
  begin
   if (InputNew[i] = '\') or (InputNew[i] = '/') then
   // '/' because some programs use Unix paths
   begin
    FileName:= copy(InputNew,i+1,Length(InputNew)-i);
    break;
   end; 
  end;
  //close old file
  //for Adobe Reader / Acrobat 8
  InputNewName:= PChar(FileName+' - Adobe Reader');
  KillProcess(FindWindow(nil,InputNewName));
  InputNewName:= PChar(FileName+'- Adobe Acrobat Standard');
  KillProcess(FindWindow(nil,InputNewName));
  InputNewName:= PChar(FileName+'- Adobe Acrobat Professional');
  KillProcess(FindWindow(nil,InputNewName));
  //for Acrobat 7
  //(this doesn't work for Acrobat 7 because it keeps the file opened
  // also when the window of the PDF is closed (use PDFViewWin7 instead) )
  InputNewName:= PChar('Adobe Acrobat Professional - ['+FileName+']');
  KillProcess(FindWindow(nil,InputNewName));
  InputNewName:= PChar('Adobe Acrobat Standard - ['+FileName+']');
  KillProcess(FindWindow(nil,InputNewName));
  //if one or more files are opened
  InputNewName:= PChar(FileName);
  KillProcess(FindWindow(nil,InputNewName));
  //delete old file
  DeleteFile(InputNew);
 end; //end if FileTest = true
 //rename file
 RenameFile(Input,InputNew);
 //open renamed file
 ShellExecute(Application.Handle,PChar('open'),
              PChar(InputNew),nil,nil,SW_SHOWNORMAL);

              
end. //end program
