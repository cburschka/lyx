// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author: Jürgen Vigna
 *
 * ====================================================== 
 */

#ifndef TABULAR_FUNCS_H
#define TABULAR_FUNCS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "layout.h"
#include "tabular.h"


// Perfect case for a template... (Lgb)
// or perhaps not...
template<class T>
string const write_attribute(string const & name, T const & t);
template<>
string const write_attribute(string const & name, bool const & b);
template<>
string const write_attribute(string const & name, LyXLength const & value);
string const tostr(LyXAlignment const & num);
string const tostr(LyXTabular::VAlignment const & num);
string const tostr(LyXTabular::BoxType const & num);

// I would have liked a fromstr template a lot better. (Lgb)
extern bool string2type(string const str, LyXAlignment & num);
extern bool string2type(string const str, LyXTabular::VAlignment & num);
extern bool string2type(string const str, LyXTabular::BoxType & num);
extern bool string2type(string const str, bool & num);
extern bool getTokenValue(string const & str, const char * token, string &ret);
extern bool getTokenValue(string const & str, const char * token, int & num);
extern bool getTokenValue(string const & str, const char * token,
                          LyXAlignment & num);
extern bool getTokenValue(string const & str, const char * token,
                          LyXTabular::VAlignment & num);
extern bool getTokenValue(string const & str, const char * token,
                          LyXTabular::BoxType & num);
extern bool getTokenValue(string const & str, const char * token, bool & flag);
extern bool getTokenValue(string const & str, const char * token,
                          LyXLength & len);
extern void l_getline(istream & is, string & str);

#endif
