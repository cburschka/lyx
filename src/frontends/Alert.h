// -*- C++ -*-
/**
 * \file Alert.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LYX_ALERT_H
#define LYX_ALERT_H

#include "support/lstrings.h"

#include <algorithm>

namespace Alert {

/**
 * Prompt for a question. Returns 0-2 for the chosen button.
 * Set default_button to a reasonable value. b1-b3 should have
 * accelerators marked with an '&'. title should be a short summary.
 * Strings should be gettextised. Please think about the poor user.
 *
 * Remember to use boost::format. If you make any of these buttons
 * "Yes" or "No", I will personally come around to your house and
 * slap you with fish, and not in an enjoyable way either.
 */
int prompt(string const & title, string const & question,
           int default_button,
	   string const & b1, string const & b2, string const & b3 = string());

/**
 * Display a warning to the user. Title should be a short (general) summary.
 * Only use this if the user cannot perform some remedial action.
 */
void warning(string const & title, string const & message);

/**
 * Display a warning to the user. Title should be a short (general) summary.
 * Only use this if the user cannot perform some remedial action.
 */
void error(string const & title, string const & message);

/**
 * Informational message. Use very very sparingly. That is, you must
 * apply to me, in triplicate, under the sea, breathing in petrol
 * and reciting the Nicene Creed, whilst running uphill and also
 * eating.
 */
void information(string const & title, string const & message);

/// Asks for a text. DO NOT USE !!
std::pair<bool, string> const
askForText(string const & msg,
	   string const & dflt = string());

}

#endif // LYX_ALERT_H
