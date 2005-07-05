/**
 * \file lyxtime.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxtime.h"
#include "lyxrc.h"

using std::string;

namespace lyx {

time_type current_time()
{
	return time(0);
}


string const formatted_time(time_type t, string const & fmt)
{
	struct tm * loc_tm = localtime(&t);
	char date[50];
	strftime(date, sizeof(date), fmt.c_str(), loc_tm);
	return string(date);
}


string const formatted_time(time_type t)
{
	return formatted_time(t, lyxrc.date_insert_format);
}

} // namespace lyx
