#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "chset.h"
#include "filetools.h"
#include "lyxlex.h"
#include "error.h"


CharacterSet::CharacterSet()
{
	map_=NULL;
}


CharacterSet::~CharacterSet()
{
	freeMap();
}


void CharacterSet::freeMap()
{
	Cdef* t;
	while(map_) {
		t=map_;
		map_=map_->next;
		delete t;
	}

	name_.clean();
}


bool CharacterSet::loadFile(const LString& fname)
{
	freeMap();
    
	if (fname.empty() || fname=="ascii") 
		return true;	// ascii 7-bit

	// open definition file
	lyxerr.debug("Opening keymap file "+ fname+ ".cdef",Error::KBMAP);
	LString filename = LibFileSearch("kbd", fname.c_str(), "cdef");
	FilePtr f(filename, FilePtr::read);
	if (filename.empty() || !f()) {
		lyxerr.print("Unable to open keymap file");
		return true;		// no definition, use 7-bit ascii
	}

	name_=fname;

	// now read the file
	LyXLex lex(NULL,0);
	lex.setFile(f());

	bool error=false;
	LString str;
	int n;
	
	while(lex.IsOK() && !error) {
		
		switch(lex.lex()){
		case LyXLex::LEX_FEOF :
			lyxerr.debug("End of parsing of .cdef file",
				     Error::KBMAP);
			break;
		default:
			// Get Integer
			n=lex.GetInteger();
			if (n<0) {
				error=true;
				continue;
			}
	
			// Get String
			lex.next(true);
			str=lex.GetString();

			Cdef* tempc=new Cdef;
			tempc->str=str;
			tempc->ic=n;
			tempc->next=map_;
			map_=tempc;
	
			if (lyxerr.debugging(Error::KBMAP))
				fprintf(stderr, "Chardef: %d to [%s]\n", 
					n, tempc->str.c_str());
			break;
		}
	}
	
	return false;
}


bool CharacterSet::encodeString(LString& str)
{
	Cdef *t=map_;
    
	while(t) {
		if (t->str==str) {
			str=LString(t->ic);
			break;
		}
		t=t->next;
	}
	return (t!=NULL);
}


LString CharacterSet::getName()
{
	return name_;
}
