// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *
 *           This file is Copyright (C) 1996-1998
 *           Lars Gullik Bjønnes
 *
 *======================================================
 */

#ifndef _LITERATE_H
#define _LITERATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "DepTable.h"

///
class Literate: public LaTeX {
public:
	Literate(string const & cmd, string const & file, string const & path,
		 string const & litfile,
		 string const & literate_cmd, string const & literate_filter, 
		 string const & build_cmd, string const & build_filter);
	
        /// runs literate and latex
        int weave(TeXErrors &, MiniBuffer *);

        ///
        int scanLiterateLogFile(TeXErrors &);

        /// runs literate and build
        int build(TeXErrors &, MiniBuffer *);

        ///
        int scanBuildLogFile(TeXErrors &);

private:
        ///
        string litfile;
        
        ///
        string literate_cmd;
 
        ///
        string literate_filter;
 
        ///
        string build_cmd;
 
        ///
        string build_filter;
};

#endif
