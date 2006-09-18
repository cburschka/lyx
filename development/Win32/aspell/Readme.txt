Aspell on Windows

You need cmake and cygwin (cvs-HEAD:with Unix line endings!!).

- check out the rel_0_60-branch version of Aspell
  http://savannah.gnu.org/cvs/?group=aspell
  cvs -z9 -d:pserver:anonymous@cvs.savannah.gnu.org:\
  /sources/aspell co -r rel_0_60-branch aspell

- first build with cygwin to autogenerate 
  all required  files:
  ./autogen
  ./configure
  ./make
  
- copy the files from the LyX SVN aspell/win32

- with cygwin patch aspell, call in aspell/:
    patch -p0 -i win32/win32-aspell-0.60.patch
  
- make a separate build directory and run 
  on a standard Windows command promp
    cmake <Aspell source directory>\win32
  build and install debug AND release libraries 

- build mingw makefiles files in 
  - cmd.exe shell:
    cmake -G"MinGW Makefiles" <Aspell source directory>\win32 
  - in a MSYS shell
    cmake -G"MSYS Makefiles"  <Aspell source directory>\win32 
  
  
The patched Aspell reads the location of the dictonary from 
the registry. You can either install the dictionaries using 
the official 1.4.2 installer or download them at:
http://wiki.lyx.org/Windows/Aspell6




