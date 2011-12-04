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


time_t from_ctime(string t)
{
	// Example for the format: "Sun Nov  6 10:39:39 2011\n"
	// Generously remove trailing '\n' (and other whitespace if needed)
	t = trim(t, " \t\r\n");
#if QT_VERSION >= 0x040400
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
	return loc_dt.toTime_t();
#elif defined(_WIN32)
#error "The minimum required Qt version on windows is Qt 4.4."
#else
	// strptime() is not available on windows (defined by POSIX)

	// strptime() uses the current locale, so we need to switch to "C"
	LYXERR(Debug::LOCALE, "Setting LC_ALL and LC_TIME to C");
	string oldLC_ALL = getEnv("LC_ALL");
	string oldLC_TIME = getEnv("LC_TIME");
	if (!setEnv("LC_ALL", "C"))
		LYXERR(Debug::LOCALE, "\t... LC_ALL failed!");
	if (!setEnv("LC_TIME", "C"))
		LYXERR(Debug::LOCALE, "\t... LC_TIME failed!");

	struct tm loc_tm;
	char const * const format = "%a%n%b%n%d%n%T%n%Y";
	char * remainder = strptime(t.c_str(), format, &loc_tm);

	LYXERR(Debug::LOCALE, "Resetting LC_ALL and LC_TIME");
	if(!setEnv("LC_TIME", oldLC_TIME))
		LYXERR(Debug::LOCALE, "\t... LC_TIME failed!");
	if (!setEnv("LC_ALL", oldLC_ALL))
		LYXERR(Debug::LOCALE, "\t... LC_ALL failed!");

	if (!remainder) {
		LYXERR(Debug::LOCALE, "Could not parse `" << t
				<< "´ (invalid format)");
		return static_cast<time_t>(-1);
	}
	if (*remainder != '\0') {
		LYXERR(Debug::LOCALE, "Could not parse `" << t
				<< "´ (excess characters)");
		return static_cast<time_t>(-1);
	}
	return mktime(&loc_tm);
#endif
}

} // namespace support
} // namespace lyx
