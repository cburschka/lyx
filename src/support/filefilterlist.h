// -*- C++ -*-
/**
 * \file filefilterlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILE_FILTER_LIST_H
#define FILE_FILTER_LIST_H

#include <string>
#include <vector>

namespace lyx {
namespace support {

/** \c FileFilterList parses a Qt-style list of available file filters
 *  to generate the corresponding vector.
 *  For example "TeX documents (*.tex);;LyX Documents (*.lyx)"
 *  will be parsed to fill a vector of size 2, whilst "*.{p[bgp]m} *.pdf"
 *  will result in a vector of size 1 in which the description field is empty.
 */
class FileFilterList {
public:
	class Filter {
		std::string desc_;
		std::string globs_;
	public:
		Filter(std::string const & d, std::string const & g)
			: desc_(d), globs_(g) {}
		std::string const & description() const { return desc_; }
		std::string const & globs() const { return globs_; }
	};

	/** \param qt_style_filter a list of available file filters.
	 *  Eg. "TeX documents (*.tex);;LyX Documents (*.lyx)".
	 *  The "All files (*)" filter is always added to the list.
	 */
	explicit FileFilterList(std::string const & qt_style_filter = std::string());
	std::vector<Filter> const & filters() const { return filters_; }

	/** \param expand pass each glob through \c convert_brace_glob.
	 *  \returns the equivalent of the string passed to the c-tor.
	 */
	std::string const str(bool expand) const;

private:
	void parse_filter(std::string const & filter);
	std::vector<Filter> filters_;
};

} // namespace support
} // namespace lyx

#endif // NOT FILE_FILTER_LIST_H
