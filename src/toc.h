// -*- C++ -*-
/**
 * \file toc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * Nice functions and objects to handle TOCs
 */

#ifndef TOC_H
#define TOC_H

class Cursor;

namespace lyx {
namespace toc {

/// the type of outline operation
enum OutlineOp {
	Up, // Move this header with text down
	Down,   // Move this header with text up
	In, // Make this header deeper
	Out // Make this header shallower
};


void outline(OutlineOp, Cursor &);


} // namespace toc
} // namespace lyx

#endif // CONTROLTOC_H
