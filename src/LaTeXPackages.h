// -*- C++ -*-
/**
 * \file LaTeXPackages.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LATEXPACKAGES_H
#define LATEXPACKAGES_H

#include <string>
#include <set>


namespace lyx {


/** The list of avilable LaTeX packages
 */
class LaTeXPackages {
public:
	/// Which of the required packages are installed?
	static void getAvailable();
	/// Is the (required) package available?
	static bool isAvailable(std::string const & name);
	/// Is the (required) package available at least as of
	/// version y/m/d?
	static bool isAvailableAtLeastFrom(std::string const & name,
					   int const y, int const m, int const d = 1);
private:
	/// The available (required) packages
	typedef std::set<std::pair<std::string,std::string>> Packages;
	///
	static Packages packages_;
};


} // namespace lyx

#endif
