#include <config.h>

#include "lengthcommon.h"

int const num_units = LyXLength::UNIT_NONE;

// I am not sure if "mu" should be possible to select (Lgb)
char const * unit_name[num_units] = { "sp", "pt", "bp", "dd",
				      "mm", "pc", "cc", "cm",
				      "in", "ex", "em", "mu",
				      "%",  "c%", "p%", "l%" };


LyXLength::UNIT unitFromString(string const & data)
{
	int i = 0;
	while (i < num_units && data != unit_name[i])
		++i;
	return static_cast<LyXLength::UNIT>(i);
}
