// -*- C++ -*-
/**
 * \file insetsection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETSECTION_H
#define INSETSECTION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "insettext.h"

/** A colapsable text inset
*/
class InsetSection : public InsetText {
public:
protected:
private:
	string type_;
};

#endif
