// os_os2.C

// Various OS specific functions
#include <config.h>

#ifdef __GNUG__
#pragma implementation "os.h"
#endif

#include "os.h"
#include "support/filetools.h"
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSNLS
#define INCL_DOSERRORS
#include <os2.h>

string os::binpath_ = string();
string os::binname_ = string();
string os::tmpdir_ = string();
os::shell_type os::_shell = os::UNIX;
unsigned long os::cp_ = 0;

void os::init(int * argc, char ** argv[]) {
	if (argc != 0 /* This is a hack! */) {
		_wildcard(argc, argv);
		PTIB ptib = new TIB[1];
		PPIB ppib = new PIB[1];
		APIRET rc = DosGetInfoBlocks(&ptib, &ppib);
		if (rc != NO_ERROR)
			exit(rc);
		char* tmp = new char[256];
		// This is the only reliable way to retrieve the executable name.
		rc = DosQueryModuleName(ppib->pib_hmte, 256L, tmp);
		if (rc != NO_ERROR)
			exit(rc);
		string p(tmp);
		p = slashify_path(p);
		binname_ = OnlyFilename(p);
		binname_.erase(binname_.length()-4, string::npos);
		binpath_ = OnlyPath(p);

		// OS/2 cmd.exe has another use for '&'
		string sh = OnlyFilename(GetEnvPath("EMXSHELL"));
		if (sh.empty()) {
			// COMSPEC is set, unless user unsets
			sh = OnlyFilename(GetEnvPath("COMSPEC"));
			if (sh.empty())
				sh = "cmd.exe";
		}
		sh = lowercase(sh);	// DosMapCase() is an overkill here
		if (contains(sh, "cmd.exe")
		    || contains(sh, "4os2.exe"))
			_shell = os::CMD_EXE;
		else
			_shell = os::UNIX;
	}
	static bool initialized = false;
	if (initialized) return;
	initialized = true;
	ULONG CPList[3] = {0}, CPList_size;
	APIRET rc = DosQueryCp(3 * sizeof(ULONG), CPList, &CPList_size);
	if (rc != NO_ERROR)
		exit(rc);
	// CPList[0] == process current codepage,
	// CPList[1] == system default codepage, the rest are auxilary.
	// Once cp_ is correctly set, you can call other routines.
	cp_ = CPList[1];
}

void os::warn(string /*mesg*/) {
	return;
}

string os::current_root() {
	APIRET rc;
	ULONG drv_num, drv_map;
	rc = DosQueryCurrentDisk(&drv_num, &drv_map);
	if (rc != NO_ERROR)
		exit(rc);
	char drive = 'A' + drv_num -1;
	string tmp(1, drive);
	tmp  += ":/";
	return tmp;
}

string::size_type os::common_path(string const &p1, string const &p2) {
	static bool initialized = false;
	if (!initialized) {
		init(0, 0);
		initialized = true;
	}
	COUNTRYCODE cntry;
	cntry.country = 0;
	cntry.codepage = cp_;
	string temp1 = slashify_path(p1);
	string temp2 = slashify_path(p2);
	char * tmp1 = const_cast<char*> (temp1.c_str());
	char * tmp2 = const_cast<char*> (temp2.c_str());
	/* rc = */ DosMapCase(p1.length(), &cntry, tmp1);
	// if (rc != NO_ERROR)
	//	exit(rc);
	/* rc = */ DosMapCase(p2.length(), &cntry, tmp2);
	// if (rc != NO_ERROR)
	//	exit(rc);
	// This algorithm works only if paths are slashified on DBCS systems.
	string::size_type i = 0,
			p1_len = p1.length(),
			p2_len = p2.length();
	while (i < p1_len && i < p2_len && tmp1[i] == tmp2[i]) ++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && tmp1[i] != '/' && i == p2_len)
	    || (i < p2_len && tmp2[i] != '/' && i == p1_len)) {
		if (i) --i;     // here was the last match
		while (i && tmp1[i] != '/') --i;
	}
	return i;
}

string os::slashify_path(string p) {
	static bool initialized = false;
	static bool leadbyte[256] = {false};
	if (!initialized) {
		init(0, 0);
		COUNTRYCODE cntry;
		cntry.country = 0;
		cntry.codepage = cp_;
		unsigned char *DBCSinfo = new unsigned char[12];
		/* rc = */ DosQueryDBCSEnv(12, &cntry, (char*) DBCSinfo);
		// if (rc != NO_ERROR)
		//	exit(rc);
		for (int j = 1; j < 12; j += 2)
			DBCSinfo[j]++;
		unsigned char i = 0;
		bool isLeadByte = false;
		while (*DBCSinfo != 0) {
			if (i == *DBCSinfo) {
				isLeadByte = !isLeadByte;
				DBCSinfo++;
			}
			leadbyte[i++] = isLeadByte;
		}
		initialized = true;
	}
	string::iterator lit = p.begin();
	string::iterator end = p.end();
	for (; lit < end; ++lit) {
		if (leadbyte[(*lit)])
			lit += 2;
		if ((*lit) == '\\')
			(*lit) = '/';
	}
	p = subst(p, "//", "/");
	return p;
}


string os::external_path(string const &p) {
	return p;
}


string os::internal_path(string const &p) {
	return p;
}


bool os::is_absolute_path(string const & p)
{
	return (p.length() > 1
		&& isalpha(static_cast<unsigned char>(p[0]))
		&& p[1] == ':');
}
