#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "chset.h"
#include "support/filetools.h"
#include "lyxlex.h"
#include "debug.h"


CharacterSet::CharacterSet()
{
	map_=0;
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

	name_.erase();
}


bool CharacterSet::loadFile(const string& fname)
{
	freeMap();
    
	if (fname.empty() || fname=="ascii") 
		return true;	// ascii 7-bit

	// open definition file
	lyxerr[Debug::KBMAP]
		<< "Opening keymap file " << fname << ".cdef" << endl;
	string filename = LibFileSearch("kbd", fname.c_str(), "cdef");
	FilePtr f(filename, FilePtr::read);
	if (filename.empty() || !f()) {
		lyxerr << "Unable to open keymap file" << endl;
		return true;		// no definition, use 7-bit ascii
	}

	name_=fname;

	// now read the file
	LyXLex lex(0,0);
	lex.setFile(f());

	bool error=false;
	string str;
	int n;
	
	while(lex.IsOK() && !error) {
		
		switch(lex.lex()){
		case LyXLex::LEX_FEOF :
			lyxerr[Debug::KBMAP] << "End of parsing of .cdef file"
					     << endl;
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
	
			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "Chardef: " << n
				       << " to [" << tempc->str << "]" << endl;
			break;
		}
	}
	
	return false;
}


bool CharacterSet::encodeString(string & str)
{
	Cdef *t=map_;
    
	while(t) {
		if (t->str==str) {
			// Can this fail? Why is ic an unsigned char?
			str = char(t->ic);
			break;
		}
		t=t->next;
	}
	return (t!=0);
}


string CharacterSet::getName()
{
	return name_;
}
