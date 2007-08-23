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


namespace lyx {

class Layout;

/// Global typedef
typedef boost::shared_ptr<Layout> LayoutPtr;


} // namespace lyx

#endif
