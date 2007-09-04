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

class Color_color;

/** Functions of use to the character GUI controller and view */
namespace lyx {

namespace support { class FileFilterList; }

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
template<class Pair>
std::vector<typename Pair::second_type> const
getSecond(std::vector<Pair> const & pr)
{
	 std::vector<typename Pair::second_type> tmp(pr.size());
	 std::transform(pr.begin(), pr.end(), tmp.begin(),
					 boost::bind(&Pair::second, _1));
	 return tmp;
}

///
typedef std::pair<docstring, std::string> LanguagePair;

/** If the caller is the character dialog, add "No change" and "Reset"
 *  to the vector.
 */
std::vector<LanguagePair> const getLanguageData(bool character_dlg);
/// sort colors for the gui
std::vector<Color_color> const getSortedColors(std::vector<Color_color> colors);

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
