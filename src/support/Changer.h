// -*- C++ -*-
/**
 * \file Changer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CHANGER_H
#define LYX_CHANGER_H

#include "support/unique_ptr.h"


namespace lyx {

// Forward declaration for support/RefChanger.h
struct Revertible {
	virtual ~Revertible() {}
	virtual void revert() {}
	virtual void keep() {}
};

using Changer = unique_ptr<Revertible>;


}

#endif //LYX_CHANGER_H
