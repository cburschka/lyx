program PDFViewWin7;
// this program opens and closes PDF-files with Acrobat 5, 6, or 7
// and with Acrobat Reader 5 and Adobe Reader 6, or 7

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
 To open/close files in Acrobat, OLE-Objects are used because the latest
 manual "Developing Applications Using Interapplication Communication" from
 Adobe Acrobat SDK Version 8 states:
 "Although DDE is supported, you should use OLE automation instead of DDE
  whenever possible because DDE is not a COM technology."}

{$APPTYPE CONSOLE}

uses
  Windows,SysUtils,ShellApi,Forms,ComObj,Variants;

var Input,InputNew : string;
    FileTest : boolean;
    App, AVDoc : Variant;
    CoInitFlags : Integer = -1;
    VarTest : IDispatch;
    test : PVariant;


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
end; //end function


begin //begin program

 Application.Initialize;
 //Read given filename
 Input:= ParamStr(1);
 //InputNew = original filename with ending "-preview" (e.g. test-preview.pdf)
 InputNew:= copy(Input,1,Length(Input)-4); //remove ".pdf"
 InputNew:= InputNew+'-preview.pdf';
 //check if renamed file exists
 FileTest:= FileExists(InputNew);
 //Create OLE-object for the program Acrobat or Adobe Viewer
 App:=CreateOleObject('AcroExch.App');
 //test if given file already exists
 if FileTest = true then
 begin
  //close old file
  AVDoc:=App.GetActiveDoc; //handle of the active document
  VarTest:=AVDoc;
  test:= PVariant(VarTest);
  if test <> PVariant(0) then //when handle is existing
  begin
   try
    AVDoc.Close(true);
   except
    Application.Terminate;
   end;
  end;
  //delete old file
  DeleteFile(InputNew);
 end; //end if FileTest
 //rename file
 RenameFile(Input,InputNew);
 //open renamed file in Acobat or Adobe Viewer
 App.Show;                      //show window
 App.Restore(true);             //restore window size to make window active
 App.Maximize(true);            //maximize window
 AVDoc:=CreateOleObject('AcroExch.AVDoc'); //create OLE object for file
 AVDoc.Open(''+InputNew+'','');            //open file

end. //end program
