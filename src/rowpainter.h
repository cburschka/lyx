// -*- C++ -*-
/**
 * \file rowpainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ROWPAINTER_H
#define ROWPAINTER_H

#include "support/types.h"

namespace lyx {

class LyXText;
class BufferView;
class PainterInfo;
class ViewMetricsInfo;

namespace frontend { class Painter; }

/// paint visible paragraph of main text
void paintText(BufferView & bv, ViewMetricsInfo const & vi,
			   frontend::Painter & painter);

/// paint the rows of a text inset
void paintTextInset(LyXText const & text, PainterInfo & pi, int x, int y);

/// some space for drawing the 'nested' markers (in pixel)
inline int nestMargin() { return 15; }

/// margin for changebar
inline int changebarMargin() { return 12; }

/// right margin
inline int rightMargin() { return 10; }

} // namespace lyx

#endif // ROWPAINTER_H
