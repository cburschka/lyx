/*
 * helpers.h
 *
 * (C) 2001 LyX Team
 *
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <config.h>

#include <qcombobox.h>
#include <krestrictedline.h> 

#include "vspace.h"

#ifdef CXX_WORKING_NAMESPACES
namespace kde_helpers {
#endif
 
/**
 * \fn setSizeHint
 * \brief sets the minimum size of a widget
 * \param widget the widget 
 * 
 * Sets a widget's minimum size to its sizeHint()
 */
inline void setSizeHint(QWidget *widget)
{
	widget->setMinimumSize(widget->sizeHint());
}
 
/**
 * \fn setComboFromStr
 * \brief sets a combo box item given a string matching one of the entries.
 * \param box the box to change
 * \param str the string to match
 *
 * Set a combobox current item matching the given string. false is returned if
 * the item could not be found.
 */
bool setComboFromStr(QComboBox *box, const string & str);

#ifdef CXX_WORKING_NAMESPACES
}; /* namespace kde_helpers */
#endif

#endif /* HELPERS_H */
