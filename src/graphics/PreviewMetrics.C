/*
 *  \file PreviewMetrics.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "PreviewMetrics.h"

#include "debug.h"
#include "lyxlex.h"
#include "support/lyxlib.h"


namespace grfx {

namespace {

keyword_item MetricsTags[] = {
	{ "%%Page", 1 }
};
 
} // namespace anon


PreviewMetrics::PreviewMetrics(string const & file)
{
	LyXLex lex(MetricsTags, 1);
	lex.setFile(file);

	if (!lex.isOK()) {
		lyxerr[Debug::GRAPHICS]
			<< "PreviewMetricsFile(" << file << ")\n"
			<< "Unable to open file." << std::endl;
		return;
	}

	int line = 0;
	while (lex.isOK()) {

		int le = lex.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lex.printError("Unknown tag `$$Token'");
			continue;
		case LyXLex::LEX_FEOF:
			continue;
		default: break;
		}

		if (le != 1 || !lex.next())
			continue;

		int store[7];
		for (int i = 0; i < 7; ++i) {
			if (lex.next())
				store[i] = lex.getInteger();
			if (lex.isOK())
				continue;

			lyxerr[Debug::GRAPHICS]
				<< "PreviewMetricsFile(" << file << ")\n"
				<< "Error reading file." << std::endl;
			break;
		}

		if (!lex.isOK())
			break;

		// These 7 numbers are
		// bb_ll_x bb_ll_y bb_ur_x bb_ur_y ht dp wd
		// We are interested only in the ratio ht:dp
		double const a = double(store[4]);
		double const d = double(store[5]);
		double const af = a / (a + d);
		store_.push_back(af);
	}

	lyx::unlink(file);
}


double PreviewMetrics::ascent_fraction(size_type counter)
{
	if (store_.empty() || counter >= store_.size())
		return 0.5;

	return store_[counter];
}


} // namespace grfx
