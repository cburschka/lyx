// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

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
extern LyXFont UserFreeFont(BufferParams const & params);

#endif

