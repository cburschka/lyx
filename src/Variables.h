// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 the LyX Team.
*
* ====================================================== */

#ifndef VARIABLES_H
#define VARIABLES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <map>


class Variables {
public:
  virtual ~Variables() { };
  // 
  void set(string const &, string const &);
  // 
  virtual string get(string const &) const;
  //
  bool isset(string const & var) const;
  // 
  string expand(string) const;
private:
  //
  typedef std::map<string,string> Vars;
  //
  Vars vars_;
};

#endif
