// -*- C++ -*-
/**
 * \file lyxlayout_ptr_fwd.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXLAYOUT_PTR_FWD_H
#define LYXLAYOUT_PTR_FWD_H

#include <boost/shared_ptr.hpp>

class LyXLayout;

/// Global typedef
typedef boost::shared_ptr<LyXLayout> LyXLayout_ptr;

#endif
