/* zipunzip.cpp -- IO for compress .zip files using zlib
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant

   This unzip package allow creates .ZIP file, compatible with PKZip 2.04g
     WinZip, InfoZip tools and compatible.
   Multi volume ZipFile (span) are not supported.
   Encryption compatible with pkzip 2.04g only supported
   Old compressions used by old PKZip 1.x are not supported

  For uncompress .zip file, look at unzip.h


   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.html for evolution

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.


*/

/* for more info about .ZIP format, see
      http://www.info-zip.org/pub/infozip/doc/appnote-981119-iz.zip
      http://www.info-zip.org/pub/infozip/doc/
   PkWare has also a specification at :
      ftp://ftp.pkware.com/probdesc.zip
*/


// The original minizip.c and miniunz.c distributed with minizip provide
// two command line tools minizip and miniunz. This file combines these two 
// files and modifies the interface to provide two functions zipFiles and 
// unzipToDir. This file is covered by the original minizip license.
// 

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include "support/FileName.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_IO_H
# include <io.h>
#endif
#ifdef HAVE_SYS_UTIME_H
# include <sys/utime.h>
#endif
#ifdef HAVE_UTIME_H
# include <utime.h>
#endif
#include <fcntl.h>

#include "zip.h"
#include "unzip.h"

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

using namespace std;
using lyx::support::FileName;


#ifdef WIN32
uLong filetime(const char * f, tm_zip * tmzip, uLong * dt)
{
	int ret = 0;
	{
		FILETIME ftLocal;
		HANDLE hFind;
		WIN32_FIND_DATA  ff32;

		hFind = FindFirstFile(f,&ff32);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
			FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
			FindClose(hFind);
			ret = 1;
		}
	}
	return ret;
}

#else
#ifdef unix
uLong filetime(const char * f, tm_zip * tmzip, uLong * /*dt*/)
{
	int ret=0;
	struct stat s;								  /* results of stat() */
	struct tm* filedate;
	time_t tm_t=0;

	if (strcmp(f,"-")!=0) {
		char name[MAXFILENAME+1];
		int len = strlen(f);
		if (len > MAXFILENAME)
			len = MAXFILENAME;

		strncpy(name, f,MAXFILENAME-1);
		/* strncpy doesnt append the trailing NULL, of the string is too long. */
		name[ MAXFILENAME ] = '\0';

		if (name[len - 1] == '/')
			name[len - 1] = '\0';
		/* not all systems allow stat'ing a file with / appended */
		if (stat(name,&s)==0) {
			tm_t = s.st_mtime;
			ret = 1;
		}
	}
	filedate = localtime(&tm_t);

	tmzip->tm_sec  = filedate->tm_sec;
	tmzip->tm_min  = filedate->tm_min;
	tmzip->tm_hour = filedate->tm_hour;
	tmzip->tm_mday = filedate->tm_mday;
	tmzip->tm_mon  = filedate->tm_mon ;
	tmzip->tm_year = filedate->tm_year;

	return ret;
}

#else

uLong filetime(const char * f, tm_zip * tmzip, uLong * dt)
{
	return 0;
}
#endif
#endif

bool zipFiles(string const & zipfile, vector<pair<string, string> > const & files)
{
	int err = 0;
	zipFile zf;
	int errclose;
	void * buf = NULL;

	int size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc(size_buf);
	if (buf==NULL) {
		printf("Error allocating memory\n");
		return false;
	}
	const char * fname = zipfile.c_str();

#ifdef USEWIN32IOAPI
	zlib_filefunc_def ffunc;
	fill_win32_filefunc(&ffunc);
	// false: not append
	zf = zipOpen2(fname, false, NULL, &ffunc);
#else
	zf = zipOpen(fname, false);
#endif

	if (zf == NULL) {
		return false;
	}

	for (vector<pair<string, string> >::const_iterator it = files.begin(); it != files.end(); ++it) {
		FILE * fin;
		int size_read;
		zip_fileinfo zi;
		const char * diskfilename = it->first.c_str();
		const char * filenameinzip = it->second.c_str();
		unsigned long crcFile=0;

		zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
			zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
		zi.dosDate = 0;
		zi.internal_fa = 0;
		zi.external_fa = 0;
		filetime(filenameinzip, &zi.tmz_date, &zi.dosDate);

		err = zipOpenNewFileInZip3(zf, filenameinzip, &zi,
			NULL,0,NULL,0,NULL /* comment*/,
			Z_DEFLATED,
			Z_DEFAULT_COMPRESSION,		  // compression level
			0,
		/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			NULL,						  // password,
			crcFile);

		if (err != ZIP_OK) {
			printf("error in opening %s in zipfile\n",filenameinzip);
			return false;
		}
		fin = fopen(diskfilename, "rb");
		if (fin==NULL) {
			return false;
		}

		do {
			err = ZIP_OK;
			size_read = (int)fread(buf, 1, size_buf, fin);
			if (size_read < size_buf)
				if (feof(fin)==0) {
					printf("error in reading %s\n",filenameinzip);
					return false;
				}

			if (size_read>0) {
				err = zipWriteInFileInZip (zf, buf, size_read);
				if (err<0) {
					printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
					return false;
				}
			}
		} while ((err == ZIP_OK) && (size_read>0));

		if (fin)
			fclose(fin);

		err = zipCloseFileInZip(zf);
		if (err != ZIP_OK) {
			printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
			return false;
		}
	}
	errclose = zipClose(zf, NULL);
	if (errclose != ZIP_OK) {
		printf("error in closing zip file\n");
		return false;
	}
	free(buf);
	return true;
}

// adapted from miniunz.c

/* change_file_date : change the date/time of a file
	filename : the filename of the file where date/time must be modified
	dosdate : the new date at the MSDos format (4 bytes)
	tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char * filename, uLong dosdate, tm_unz tmu_date)
{
#ifdef WIN32
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

	hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
	LocalFileTimeToFileTime(&ftLocal,&ftm);
	SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
	CloseHandle(hFile);
#else
#ifdef unix
	utimbuf ut;
	tm newdate;

	newdate.tm_sec = tmu_date.tm_sec;
	newdate.tm_min=tmu_date.tm_min;
	newdate.tm_hour=tmu_date.tm_hour;
	newdate.tm_mday=tmu_date.tm_mday;
	newdate.tm_mon=tmu_date.tm_mon;
	if (tmu_date.tm_year > 1900)
		newdate.tm_year=tmu_date.tm_year - 1900;
	else
		newdate.tm_year=tmu_date.tm_year ;
	newdate.tm_isdst=-1;

	ut.actime=ut.modtime=mktime(&newdate);
	utime(filename,&ut);
#endif
#endif
}


int do_extract_currentfile(unzFile uf,
	const int * popt_extract_without_path,
	int * /* popt_overwrite */,
	const char * password,
	const char * dirname)
{
	char full_path[1024];
	char* filename_inzip = full_path;
	char* filename_withoutpath;
	char* p;
	int err=UNZ_OK;
	FILE *fout=NULL;
	void* buf;
	uInt size_buf;

	strcpy(full_path, dirname);
	while ((*filename_inzip) != '\0') filename_inzip++;
	if (*(filename_inzip-1) != '/' && *(filename_inzip-1) != '\\') {
		*filename_inzip = '/';
		*(++filename_inzip) = '\0';
	}

	unz_file_info file_info;
	//uLong ratio=0;
	err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,256,NULL,0,NULL,0);

	if (err!=UNZ_OK) {
		printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc(size_buf);
	if (buf==NULL) {
		printf("Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0') {
		if (((*p)=='/') || ((*p)=='\\'))
			filename_withoutpath = p+1;
		p++;
	}
	// this is a directory
	if ((*filename_withoutpath)=='\0') {
		if ((*popt_extract_without_path)==0) {
			// printf("Create directory %s\n", filename_inzip);
			FileName(filename_inzip).createPath();
		}
	}
	// this is a filename
	else {
		err = unzOpenCurrentFilePassword(uf,password);
		if (err!=UNZ_OK) {
			printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
		} else {
			fout=fopen(full_path, "wb");

			/* some zipfile don't contain directory alone before file */
			if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
				(filename_withoutpath!=(char*)filename_inzip)) {
				char c=*(filename_withoutpath-1);
				*(filename_withoutpath-1)='\0';
				// printf("Create directory %s\n", full_path);
				FileName(full_path).createPath();
				*(filename_withoutpath-1)=c;
				fout=fopen(full_path,"wb");
			}

			if (fout==NULL) {
				printf("error opening %s\n", full_path);
			}
		}

		if (fout!=NULL) {

			do {
				err = unzReadCurrentFile(uf,buf,size_buf);
				if (err<0) {
					printf("error %d with zipfile in unzReadCurrentFile\n",err);
					break;
				}
				if (err>0)
					if (fwrite(buf,err,1,fout)!=1) {
						printf("error in writing extracted file\n");
						err=UNZ_ERRNO;
						break;
					}
			} while (err>0);
			if (fout)
				fclose(fout);

			if (err==0)
				change_file_date(full_path, file_info.dosDate,
					file_info.tmu_date);
		}

		if (err==UNZ_OK) {
			err = unzCloseCurrentFile (uf);
			if (err!=UNZ_OK) {
				printf("error %d with zipfile in unzCloseCurrentFile\n",err);
			}
		}
		else
			unzCloseCurrentFile(uf);	  /* don't lose the error */
	}

	free(buf);
	return err;
}


bool unzipToDir(string const & zipfile, string const & dirname)
{
	unzFile uf=NULL;
#ifdef USEWIN32IOAPI
	zlib_filefunc_def ffunc;
#endif

	const char * zipfilename = zipfile.c_str();

#ifdef USEWIN32IOAPI
	fill_win32_filefunc(&ffunc);
	uf = unzOpen2(zipfilename, &ffunc);
#else
	uf = unzOpen(zipfilename);
#endif

	if (uf==NULL) {
		return false;
	}

	uLong i;
	unz_global_info gi;
	int err;
	//FILE* fout=NULL;
	int opt_extract_without_path = 0;
	int opt_overwrite = 1;
	char * password = NULL;

	err = unzGetGlobalInfo (uf, &gi);
	if (err != UNZ_OK) {
		printf("error %d with zipfile in unzGetGlobalInfo \n",err);
		return false;
	}

	for (i=0; i < gi.number_entry; i++) {
		if (do_extract_currentfile(uf, &opt_extract_without_path,
			&opt_overwrite,
			password, dirname.c_str()) != UNZ_OK)
			break;

		if ((i+1)<gi.number_entry) {
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK) {
				printf("error %d with zipfile in unzGoToNextFile\n",err);
				break;
			}
		}
	}

	unzCloseCurrentFile(uf);
    return true;
}

