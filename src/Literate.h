// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor 	 
 *	     Copyright (C) 1995 Matthias Ettrich
 *
 * ====================================================== 
 */

#ifndef LITERATE_H
#define LITERATE_H

#ifdef __GNUG__
#pragma interface
#endif

class MiniBuffer;
	
///
class Literate : public LaTeX {
public:
	Literate(string const & cmd, string const & file, string const & path,
		 string const & litfile,
		 string const & literate_cmd, string const & literate_filter, 
		 string const & build_cmd, string const & build_filter);
	
        /// runs literate and latex
        int weave(TeXErrors &, MiniBuffer *);

        /// runs literate and build
        int build(TeXErrors &, MiniBuffer *);


private:
        ///
        int scanLiterateLogFile();

        ///
        int scanBuildLogFile();

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
