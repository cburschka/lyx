// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 * ====================================================== */

#ifndef GUIRUNTIME_PIMPL_H
#define GUIRUNTIME_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/GUIRunTime.h"

/**
   The GUI interface for runtime GUI stuff.
*/
struct GUIRunTime::Pimpl {
public:
    ///
    static void processEvents();
    ///
    static void runTime();
};
#endif
