// -*- C++ -*-
/**
 * \file frontend_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FRONTEND_HELPERS_H
#define FRONTEND_HELPERS_H

#include "Font.h"
#include "support/docstring.h"

#include <utility>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <algorithm>
#include <utility>
#include <map>

class Buffer;

/** Functions of use to citation and bibtex GUI controllers and views */
namespace lyx {
namespace biblio {

enum CiteEngine {
	ENGINE_BASIC,
	ENGINE_NATBIB_AUTHORYEAR,
	ENGINE_NATBIB_NUMERICAL,
	ENGINE_JURABIB
};


enum CiteStyle {
	CITE,
	CITET,
	CITEP,
	CITEALT,
	CITEALP,
	CITEAUTHOR,
	CITEYEAR,
	CITEYEARPAR
};


enum Search {
	SIMPLE,
	REGEX
};


enum Direction {
	FORWARD,
	BACKWARD
};


/** Each citation engine recognizes only a subset of all possible
 *  citation commands. Given a latex command \c input, this function
 *  returns an appropriate command, valid for \c engine.
 */
std::string const asValidLatexCommand(std::string const & input,
				      CiteEngine const engine);

/// First entry is the bibliography key, second the data
typedef std::map<std::string, docstring> InfoMap;

/// Returns a vector of bibliography keys
std::vector<std::string> const getKeys(InfoMap const &);

/** Returns the BibTeX data associated with a given key.
    Empty if no info exists. */
docstring const getInfo(InfoMap const &, std::string const & key);

/// return the year from the bibtex data record
docstring const getYear(InfoMap const & map, std::string const & key);

/// return the short form of an authorlist
docstring const getAbbreviatedAuthor(InfoMap const & map, std::string const & key);

// return only the family name
docstring const familyName(docstring const & name);

/** Search a BibTeX info field for the given key and return the
    associated field. */
docstring const parseBibTeX(docstring data, std::string const & findkey);

/** Returns an iterator to the first key that meets the search
    criterion, or end() if unsuccessful.

    User supplies :
    the InfoMap of bibkeys info,
    the vector of keys to be searched,
    the search criterion,
    an iterator defining the starting point of the search,
    an enum defining a Simple or Regex search,
    an enum defining the search direction.
*/

std::vector<std::string>::const_iterator
searchKeys(InfoMap const & map,
	   std::vector<std::string> const & keys_to_search,
	   docstring const & search_expression,
	   std::vector<std::string>::const_iterator start,
	   Search,
	   Direction,
	   bool caseSensitive=false);


class CitationStyle {
public:
	///
	CitationStyle(CiteStyle s = CITE, bool f = false, bool force = false)
		: style(s), full(f), forceUCase(force) {}
	/// \param latex_str a LaTeX command, "cite", "Citep*", etc
	CitationStyle(std::string const & latex_str);
	///
	std::string const asLatexStr() const;
	///
	CiteStyle style;
	///
	bool full;
	///
	bool forceUCase;
};


/// Returns a vector of available Citation styles.
std::vector<CiteStyle> const getCiteStyles(CiteEngine const );

/**
   "Translates" the available Citation Styles into strings for this key.
   The returned string is displayed by the GUI.


   [XX] is used in place of the actual reference
   Eg, the vector will contain: [XX], Jones et al. [XX], ...

   User supplies :
   the key,
   the InfoMap of bibkeys info,
   the available citation styles
*/
std::vector<docstring> const
getNumericalStrings(std::string const & key,
		    InfoMap const & map,
		    std::vector<CiteStyle> const & styles);

/**
   "Translates" the available Citation Styles into strings for this key.
   The returned string is displayed by the GUI.

   Eg, the vector will contain:
   Jones et al. (1990), (Jones et al. 1990), Jones et al. 1990, ...

   User supplies :
   the key,
   the InfoMap of bibkeys info,
   the available citation styles
*/
std::vector<docstring> const
getAuthorYearStrings(std::string const & key,
		     InfoMap const & map,
		     std::vector<CiteStyle> const & styles);

} // namespace biblio
} // namespace lyx


class Color_color;


/** Functions of use to the character GUI controller and view */
namespace lyx {
namespace frontend {

///
enum FONT_STATE {
	///
	IGNORE,
	///
	EMPH_TOGGLE,
	///
	UNDERBAR_TOGGLE,
	///
	NOUN_TOGGLE,
	///
	INHERIT
};

///
typedef std::pair<docstring, Font::FONT_FAMILY> FamilyPair;
///
typedef std::pair<docstring, Font::FONT_SERIES> SeriesPair;
///
typedef std::pair<docstring, Font::FONT_SHAPE>  ShapePair;
///
typedef std::pair<docstring, Font::FONT_SIZE>   SizePair;
///
typedef std::pair<docstring, FONT_STATE> BarPair;
///
typedef std::pair<docstring, Color_color> ColorPair;

///
std::vector<FamilyPair>   const getFamilyData();
///
std::vector<SeriesPair>   const getSeriesData();
///
std::vector<ShapePair>    const getShapeData();
///
std::vector<SizePair>     const getSizeData();
///
std::vector<BarPair>      const getBarData();
///
std::vector<ColorPair>    const getColorData();


///
typedef std::pair<docstring, std::string> LanguagePair;

/** If the caller is the character dialog, add "No change" and "Reset"
 *  to the vector.
 */
std::vector<LanguagePair> const getLanguageData(bool character_dlg);
/// sort colors for the gui
std::vector<Color_color> const getSortedColors(std::vector<Color_color> colors);

} // namespace frontend


namespace support { class FileFilterList; }


namespace frontend {

/** Launch a file dialog and return the chosen file.
    filename: a suggested filename.
    title: the title of the dialog.
    pattern: *.ps etc.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
docstring const
browseFile(docstring const & filename,
	   docstring const & title,
	   support::FileFilterList const & filters,
	   bool save = false,
	   std::pair<docstring, docstring> const & dir1 =
	   std::make_pair(docstring(), docstring()),
	   std::pair<docstring, docstring> const & dir2 =
	   std::make_pair(docstring(), docstring()));


/** Wrapper around browseFile which tries to provide a filename
    relative to relpath.  If the relative path is of the form "foo.txt"
    or "bar/foo.txt", then it is returned as relative. OTOH, if it is
    of the form "../baz/foo.txt", an absolute path is returned. This is
    intended to be useful for insets which encapsulate files/
*/
docstring const
browseRelFile(docstring const & filename,
	      docstring const & refpath,
	      docstring const & title,
	      support::FileFilterList const & filters,
	      bool save = false,
	      std::pair<docstring, docstring> const & dir1 =
	      std::make_pair(docstring(), docstring()),
	      std::pair<docstring, docstring> const & dir2 =
	      std::make_pair(docstring(), docstring()));


/** Wrapper around browseFile which tries to provide a filename
 *  relative to the user or system directory. The dir, name and ext
 *  parameters have the same meaning as in the
 *  support::LibFileSearch function.
 */
docstring const
browseLibFile(docstring const & dir,
	      docstring const & name,
	      docstring const & ext,
	      docstring const & title,
	      support::FileFilterList const & filters);


/** Launch a file dialog and return the chosen directory.
    pathname: a suggested pathname.
    title: the title of the dialog.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
docstring const
browseDir(docstring const & pathname,
	   docstring const & title,
	   std::pair<docstring, docstring> const & dir1 =
	   std::make_pair(docstring(), docstring()),
	   std::pair<docstring, docstring> const & dir2 =
	   std::make_pair(docstring(), docstring()));


/// Returns a vector of units that can be used to create a valid LaTeX length.
std::vector<docstring> const getLatexUnits();


/** Functions to extract vectors of the first and second elems from a
    vector<pair<A,B> >
*/
template<class Pair>
std::vector<typename Pair::first_type> const
getFirst(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::first_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(),
		       boost::bind(&Pair::first, _1));
	return tmp;
}

template<class Pair>
std::vector<typename Pair::second_type> const
getSecond(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::second_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(),
		       boost::bind(&Pair::second, _1));
	return tmp;
}

/** Build filelists of all availabe bst/cls/sty-files. Done through
 *  kpsewhich and an external script, saved in *Files.lst.
 */
void rescanTexStyles();

/// rebuild the textree
void texhash();

/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a name_with_path
 */
void getTexFileList(std::string const & filename, std::vector<std::string> & contents);

/// get the options of stylefile
std::string const getListOfOptions(std::string const & classname, std::string const & type);

/// get a class with full path from the list
std::string const getTexFileFromList(std::string const & classname, std::string const & type);

} // namespace frontend
} // namespace lyx

#endif // FRONTEND_HELPERS_H
