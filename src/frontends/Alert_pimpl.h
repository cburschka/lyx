/**
 * \file Alert_pimpl.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "debug.h"

// GUI-specific implementations
void alert_pimpl(string const & s1, string const & s2, string const & s3);
bool askQuestion_pimpl(string const & s1, string const & s2, string const & s3);
int askConfirmation_pimpl(string const & s1, string const & s2, string const & s3);
std::pair<bool, string> const askForText_pimpl(string const & msg, string const & dflt);
