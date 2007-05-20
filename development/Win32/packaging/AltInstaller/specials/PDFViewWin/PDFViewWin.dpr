program PDFViewWin;
// this program opens and closes PDF-files with Acrobat or Adobe Reader
// author: Uwe Stöhr

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
 the modified version is copied, renamed and opened in Acrobat.}

{$APPTYPE CONSOLE}

uses
  Windows, SysUtils, ShellApi;

{$R *.res}

var Input,InputNew : string;
    FileTest : boolean;
    hConsole : THandle;

    
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


function RenameFile(const OldName,NewName: string; hConsole: THandle): boolean;
//renames files, taken from
//http://www.dsdt.info/tipps/?id=128&search=RenameFile
var
  sh: TSHFileOpStruct;
begin
  sh.Wnd:= hConsole;
  sh.wFunc:= fo_Rename;
  //terminate with null byte to set list ending
  sh.pFrom:= PChar(OldName + #0);
  sh.pTo:= PChar(NewName + #0);
  sh.fFlags:= fof_Silent or fof_MultiDestFiles;
  Result:= ShFileOperation(sh)=0;
end; //end function


begin //begin program 

 //Read path to this application
 Input:= ParamStr(0);

 //get handle of this console window
 hConsole := FindWindow(nil,Pchar(Input));
 // hide the window of this console application
 ShowWindow(hConsole,SW_HIDE);

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
 RenameFile(Input,InputNew,hConsole);
 ExecWait('pdfopen --file "'+InputNew+'" --back');

end. //end program 
