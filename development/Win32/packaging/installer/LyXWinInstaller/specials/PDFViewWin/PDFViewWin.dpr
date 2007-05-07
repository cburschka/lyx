program PDFViewWin;

{$APPTYPE CONSOLE}

uses
  Windows,SysUtils,ShellApi,Forms;

var Input,InputNew : string;
    FileTest : boolean;

    
function ExecWait(const CommandLine: string;
                 const Visible: boolean = false;
                 const MaxSeconds: integer = 60): boolean;
//Executes programs and waits until they are terminated
var
SI: TStartupInfo;
PI: TProcessInformation;
ExitCode: DWORD;
begin
 result := false;
 GetStartupInfo(SI);
 if not Visible then
 begin
  SI.dwFlags := STARTF_USESHOWWINDOW;
  SI.wShowWindow := SW_HIDE;
 end;
 if (CreateProcess(nil, pchar(CommandLine), nil, nil, False, 0, nil, nil, SI, PI)) then
 begin
  case WaitForSingleObject(PI.hProcess, MaxSeconds * 1000) of
       WAIT_OBJECT_0: GetExitCodeProcess(PI.hProcess, ExitCode);
       WAIT_ABANDONED: TerminateProcess(PI.hProcess, ExitCode);
       WAIT_TIMEOUT: TerminateProcess(PI.hProcess, ExitCode);
  end;
  result := ExitCode = 0;
  CloseHandle(PI.hProcess);
  CloseHandle(PI.hThread);
 end;
end; // end function


function RenameFile(const OldName, NewName: string): boolean;
//renames files
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
end; //end function


begin //begin program 

 //Read given filename
 Input:= ParamStr(1);
 //InputNew = original filename with ending "-preview" (e.g. test-preview.pdf)
 InputNew:= copy(Input,1,Length(Input)-4); //remove ".pdf"
 InputNew:= InputNew+'-preview.pdf';
 //check if renamed file exists
 FileTest:= FileExists(InputNew);
 if FileTest = true then
 begin
  //close old file
  ExecWait('pdfclose --file "'+InputNew+'"');
  //delete old file
  DeleteFile(InputNew);
 end;
 //rename file
 RenameFile(Input,InputNew);
 ExecWait('pdfopen --file "'+InputNew+'" --back');

end. //end program 
