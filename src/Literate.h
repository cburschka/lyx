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
	Literate(LString const & cmd, LString const & file, LString const & path,
		 LString const & litfile,
		 LString const & literate_cmd, LString const & literate_filter, 
		 LString const & build_cmd, LString const & build_filter);
	
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
        LString litfile;
        
        ///
        LString literate_cmd;
 
        ///
        LString literate_filter;
 
        ///
        LString build_cmd;
 
        ///
        LString build_filter;
};

#endif
