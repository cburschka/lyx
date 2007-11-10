// -*- C++ -*-
/**
 * \file FileFilterList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FILE_FILTER_LIST_H
#define FILE_FILTER_LIST_H

#include "support/docstring.h"

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
	// FIXME UNICODE: globs_ should be unicode...
	class Filter {
		docstring desc_;
		std::vector<std::string> globs_;
	public:
		/* \param description text describing the filters.
		 * \param one or more wildcard patterns, separated by
		 * whitespace.
		 */
		Filter(docstring const & description,
		       std::string const & globs);

		docstring const & description() const { return desc_; }

		typedef std::vector<std::string>::const_iterator glob_iterator;
		glob_iterator begin() const { return globs_.begin(); }
		glob_iterator end() const { return globs_.end(); }
	};

	/** \param qt_style_filter a list of available file filters.
	 *  Eg. "TeX documents (*.tex);;LyX Documents (*.lyx)".
	 *  The "All files (*)" filter is always added to the list.
	 */
	explicit FileFilterList(docstring const & qt_style_filter =
				docstring());

	typedef std::vector<Filter>::size_type size_type;

	bool empty() const { return filters_.empty(); }
	size_type size() const { return filters_.size(); }
	Filter & operator[](size_type i) { return filters_[i]; }
	Filter const & operator[](size_type i) const { return filters_[i]; }

	/** \returns the equivalent of the string passed to the c-tor
	 *  although any brace expressions are expanded.
	 *  (E.g. "*.{png,jpg}" -> "*.png *.jpg")
	 */
	docstring const as_string() const;

private:
	void parse_filter(std::string const & filter);
	std::vector<Filter> filters_;
};

} // namespace support
} // namespace lyx

#endif // NOT FILE_FILTER_LIST_H
