// -*- C++ -*-
/**
 * \file xforms/forms_fwd.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, leeming@lyx.org
 */

#ifndef FORMS_FWD_H
#define FORMS_FWD_H

#ifdef __GNUG__
#pragma interface
#endif

/// Can't forward declare the typedef, but this works fine.
class forms_;
typedef forms_ FL_FORM;

class flobjs_;
typedef flobjs_ FL_OBJECT;

typedef int FL_Coord;

#endif // FORMS_FWD_H
