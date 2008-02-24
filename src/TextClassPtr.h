// -*- C++ -*-
/**
 * \file TextClassPtr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXTCLASS_PTR_H
#define TEXTCLASS_PTR_H

#include <boost/shared_ptr.hpp>

namespace lyx {

class TextClass;

/// Global typedef
/** Shared pointer for possibly modular layout. Needed so that paste,
 *  for example, will still be able to retain the pointer, even when
 *  the buffer itself is closed.
 */
typedef boost::shared_ptr<TextClass> TextClassPtr;

} // namespace lyx

#endif
