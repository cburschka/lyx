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

#ifndef GUIRUNTIME_H
#define GUIRUNTIME_H

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;

/** The LyX GUI independent guiruntime class
  The GUI interface is implemented in the corresponding GUIRunTime_pimpl class.
  */
class GUIRunTime {
public:
    ///
    GUIRunTime();
    ///
    ~GUIRunTime();
    ///
    void processEvents();
    
    struct Pimpl;
    friend struct Pimpl;
    
private:
    Pimpl * pimpl_;
};
#endif
