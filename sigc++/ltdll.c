
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <stdio.h>

extern "C" {
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved);
}

#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL( DllMain );
HINSTANCE __hDllInstance_base;

BOOL APIENTRY
DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
  __hDllInstance_base = hInst;
  return TRUE;
}

