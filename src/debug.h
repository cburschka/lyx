// -*- C++ -*-

#ifndef LYXDEBUG_H
#define LYXDEBUG_H

#include "LString.h"

/** Ideally this should have been a namespace, but since we try to be
    compilable on older C++ compilators too, we use a struct instead.
    This is all the different debug levels that we have.
*/
struct Debug {
	///
	enum type {
		///
		NONE = 0,
		///
		INFO       = (1 << 0),   // 1
		///
		INIT       = (1 << 1),   // 2
		///
		KEY        = (1 << 2),   // 4
		///
		TOOLBAR    = (1 << 3),   // 8
		///
		PARSER     = (1 << 4),   // 16
		///
		LYXRC      = (1 << 5),   // 32
		///
		KBMAP      = (1 << 6),   // 64
		///
		LATEX      = (1 << 7),   // 128
		///
		MATHED     = (1 << 8),   // 256 // Alejandro, please use this.
		///
		FONT       = (1 << 9),   // 512
		///
		TCLASS     = (1 << 10),  // 1024
		///
		LYXVC      = (1 << 11),  // 2048
		///
		LYXSERVER  = (1 << 12),  // 4096
		///
		ROFF       = (1 << 13) 
	};
	///
	static const type ANY = type(INFO | INIT | KEY | TOOLBAR |
				     PARSER | LYXRC | KBMAP | LATEX |
				     MATHED | FONT | TCLASS | LYXVC |
				     LYXSERVER | ROFF);
	///
	friend inline void operator|=(Debug::type & d1, Debug::type d2);
	
	/** A function to convert symbolic string names on debug levels
	    to their numerical value.
	*/
	static Debug::type value(string const & val) {
		type l = Debug::NONE;
		string v(val);
		while (!v.empty()) {
			string::size_type st = v.find(',');
			string tmp(v.substr(0, st));
			if (tmp.empty()) break;
			if (val == "NONE") l |= Debug::NONE;
			else if (val == "INFO") l |= Debug::INFO;
			else if (val == "INIT") l |= Debug::INIT;
			else if (val == "KEY") l |= Debug::KEY; 
			else if (val == "TOOLBAR") l |= Debug::TOOLBAR;
			else if (val == "PARSER") l |= Debug::PARSER; 
			else if (val == "LYXRC") l |= Debug::LYXRC; 
			else if (val == "KBMAP") l |= Debug::KBMAP;  
			else if (val == "LATEX") l |= Debug::LATEX;  
			else if (val == "MATHED") l |= Debug::MATHED; 
			else if (val == "FONT") l |= Debug::FONT;   
			else if (val == "TCLASS") l |= Debug::TCLASS; 
			else if (val == "LYXVC") l |= Debug::LYXVC;  
			else if (val == "LYXSERVER") l |= Debug::LYXSERVER;
			else if (val == "ROFF") l |= Debug::ROFF;
			else break; // unknown string
			if (st == string::npos) break;
			v.erase(0, st + 1);
		}
		return l;
	}
};
///
inline void operator|= (Debug::type & d1, Debug::type d2)
{
	d1 = static_cast<Debug::type>(d1 | d2);
}


#include "support/DebugStream.h"

///
ostream & operator<<(ostream & o, Debug::type t);

extern DebugStream lyxerr;

#endif


