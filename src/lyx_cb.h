// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

#include "LString.h"
#include "lyxfont.h"

class BufferParams;

///
extern bool quitting;
///
extern bool toggleall;

// When still false after reading lyxrc, warn user
//about failing \bind_file command. RVDK_PATCH_5
///
extern bool BindFileSet;
///
extern LyXFont const UserFreeFont(BufferParams const & params);
///
void ShowMessage(Buffer const * buf,
		 string const & msg1,
		 string const & msg2 = string(),
		 string const & msg3 = string(), int delay = 6);

#endif

