/**
 * \file Alert.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "support/lstrings.h"
#include <algorithm>
 
namespace Alert {
	/// show an alert message
	void alert(string const & s1, string const & s2 = string(), 
		string const & s3 = string());

	/// show an alert message and strerror(errno)
	void err_alert(string const & s1, string const & s2 = string());

	/// ask a question 
	bool askQuestion(string const & s1, string const & s2 = string(),
		 string const & s3 = string(), bool default_value = true);

	/// Returns 1 for yes, 2 for no, 3 for cancel.
	int askConfirmation(string const & s1, string const & s2 = string(), 
		string const & s3 = string(), int default_value = 1);
 
	/// Asks for a text
	std::pair<bool, string> const askForText(string const & msg, 
		string const & dflt = string());
};
