/**
 * \file lyxtime.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxtime.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"

#include <QDateTime>
#include <QLocale>

using namespace std;

namespace lyx {
namespace support {

time_t current_time()
{
	return time(0);
}


string const formatted_time(time_t t, string const & fmt)
{
	struct tm * loc_tm = localtime(&t);
	char date[50];
	strftime(date, sizeof(date), fmt.c_str(), loc_tm);
	return string(date);
}


docstring formatted_datetime(time_t t, string const & fmt)
{
	QString qres;
	if (fmt.empty())
		qres = QLocale().toString(QDateTime::fromTime_t(t),
		                          QLocale::ShortFormat);
	else
		qres = QLocale().toString(QDateTime::fromTime_t(t), toqstr(fmt));
	return qstring_to_ucs4(qres);
}


time_t from_asctime_utc(string t)
{
	// Example for the format: "Sun Nov  6 10:39:39 2011\n"
	// Generously remove trailing '\n' (and other whitespace if needed)
	t = trim(t, " \t\r\n");
	// toDateTime() is too stupid to recognize variable amounts of
	// whitespace (needed because ctime() outputs double spaces before
	// single digit day numbers and hours)
	t = subst(t, "  ", " ");
	QString const format("ddd MMM d H:mm:ss yyyy");
	QLocale loc("C");
	QDateTime loc_dt = loc.toDateTime(toqstr(t), format);
	if (!loc_dt.isValid()) {
		LYXERR(Debug::LOCALE, "Could not parse `" << t
				<< "´ (invalid format)");
		return static_cast<time_t>(-1);
	}
	loc_dt.setTimeSpec(Qt::UTC);
	return loc_dt.toTime_t();
}

} // namespace support
} // namespace lyx
