// -*- C++ -*-
/**
 *  \file PreviewMetrics.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef PREVIEWMETRICS_H
#define PREVIEWMETRICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/utility.hpp>
#include <vector>

namespace grfx {

class PreviewMetrics : boost::noncopyable {
public:
	///
	typedef std::vector<PreviewMetrics>::size_type size_type;

	/// Reads in file and then deletes it.
	PreviewMetrics(string const & file);

	/** Return the data from the metrics file if found,
	 *  or 0.5
	 */
	double ascent_fraction(size_type);

private:
	///
	std::vector<double> store_;
};

} // namespace grfx

#endif // PREVIEWMETRICS_H
