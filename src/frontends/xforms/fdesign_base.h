// -*- C++ -*-
/**
 * \file fdesign_base.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FDESIGN_BASE_H
#define FDESIGN_BASE_H


#include "forms_fwd.h"

/// A base class for the fdesign-generated structs
struct FD_base {
	virtual ~FD_base();
	FL_FORM * form;
};

#endif // FDESIGN_BASE_H
