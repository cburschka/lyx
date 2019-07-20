// -*- C++ -*-
/**
 * \file GuiKeySymbol.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIKEYSYM_H
#define GUIKEYSYM_H

class QKeyEvent;

namespace lyx {

/// delayed constructor
void setKeySymbol(KeySymbol * sym, QKeyEvent const * ev);

/// return the LyX key state from Qt's
KeyModifier q_key_state(Qt::KeyboardModifiers state);

} // namespace lyx

#endif // GUIKEYSYM_H
