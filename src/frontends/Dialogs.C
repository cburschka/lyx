/**
 * \file Dialogs.C
 * Copyright 1995-2002 The LyX Team.
 * See the file COPYING.
 * \author Angus Leeming <leeming@lyx.org>
 *
 * Common to all frontends' Dialogs
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

// Note that static boost signals break some compilers, so this wrapper
// initialises the signal dynamically when it is first invoked.
template<typename Signal>
class BugfixSignal {
public:
        Signal & operator()() { return thesignal(); }
        Signal const & operator()() const { return thesignal(); }

private:
        Signal & thesignal() const
        {
                if (!signal_.get())
                        signal_.reset(new Signal);
                return *signal_;
        }

        mutable boost::scoped_ptr<Signal> signal_;
};


boost::signal0<void> & Dialogs::redrawGUI()
{
        static BugfixSignal<boost::signal0<void> > thesignal;
        return thesignal();
}
