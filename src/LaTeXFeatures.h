// -*- C++ -*-
/**
 * \file LaTeXFeatures.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LATEXFEATURES_H
#define LATEXFEATURES_H

#include "OutputParams.h"
#include "support/docstring.h"

#include <set>
#include <list>
#include <map>


namespace lyx {

class Buffer;
class BufferParams;
class InsetLayout;
class Language;

/** The packages and commands that a buffer needs. This class
 *  contains a list<string>.  Each of the LaTeX packages that a buffer needs
 *  should be added with void require(string const & name).
 *
 *  i.e require("amssymb")
 *
 *  To add support you should only need to require() the package name as
 *  packages which don't have special requirements are handled automatically.
 *  If your new package does need special consideration you'll need to alter
 *  string const getPackages() const;
 *  Remember to update the validate function in Buffer.cpp and Paragraph.cpp
 *  when you do so.
 */
class LaTeXFeatures {
public:
	/// Which Language package do we use?
	enum LangPackage {
		LANG_PACK_NONE,
		LANG_PACK_BABEL,
		LANG_PACK_POLYGLOSSIA,
		LANG_PACK_CUSTOM
	};
	///
	LaTeXFeatures(Buffer const &, BufferParams const &,
		      OutputParams const &);
	/// The color packages
	std::string const getColorOptions() const;
	/// The requested package options
	std::string const getPackageOptions() const;
	/// The packages needed by the document
	std::string const getPackages() const;
	/// The macros definitions needed by the document
	docstring const getMacros() const;
	/// Extra preamble code before babel is called
	std::string const getBabelPresettings() const;
	/// Extra preamble code after babel is called
	std::string const getBabelPostsettings() const;
	/// Do we need to pass the languages to babel directly?
	bool needBabelLangOptions() const;
	/// Load AMS packages when appropriate
	std::string const loadAMSPackages() const;
	/// The definitions needed by the document's textclass
	docstring const getTClassPreamble() const;
	/// The language dependent definitions needed by the document's textclass
	docstring const getTClassI18nPreamble(bool use_babel, bool use_polyglossia) const;
	///
	docstring const getTClassHTMLStyles() const;
	///
	docstring const getTClassHTMLPreamble() const;
	/// The sgml definitions needed by the document (docbook)
	docstring const getLyXSGMLEntities() const;
	/// The SGML Required to include the files added with includeFile();
	docstring const getIncludedFiles(std::string const & fname) const;
	/// Include a file for use with the SGML entities
	void includeFile(docstring const & key, std::string const & name);
	/// The float definitions.
	void getFloatDefinitions(odocstream & os) const;
	/// Print requirements to lyxerr
	void showStruct() const;
	///
	void addPreambleSnippet(std::string const & snippet, 
	                        bool allowdupes = false);
	///
	std::string getPreambleSnippets() const;
	///
	void addCSSSnippet(std::string const &);
	///
	std::string getCSSSnippets() const;
	/// Add a feature name requirements
	void require(std::string const & name);
	/// Add a set of feature names requirements
	void require(std::set<std::string> const & names);
	/// Is the (required) package available?
	static bool isAvailable(std::string const & name);
	/// Has the package been required?
	bool isRequired(std::string const & name) const;
	/** Is this feature already provided
	 *  e.g. by the document class?
	*/
	bool isProvided(std::string const & name) const;
	/** Is it necessary to load the package? This is true if
	    isRequired is true and the feature is not already provided
	*/
	bool mustProvide(std::string const & name) const;
	///
	void useFloat(std::string const & name, bool subfloat = false);
	///
	void useLanguage(Language const *);
	///
	bool hasLanguages() const;
	/// check if all used languages are supported by polyglossia
	bool hasOnlyPolyglossiaLanguages() const;
	/// check if a language is supported only by polyglossia
	bool hasPolyglossiaExclusiveLanguages() const;
	///
	std::string getBabelLanguages() const;
	///
	std::map<std::string, std::string> getPolyglossiaLanguages() const;
	///
	std::set<std::string> getEncodingSet(std::string const & doc_encoding) const;
	///
	void getFontEncodings(std::vector<std::string> & encodings) const;
	///
	void useLayout(docstring const & lyt);
	///
	void useInsetLayout(InsetLayout const & lay);
	///
	Buffer const & buffer() const;
	///
	void setBuffer(Buffer const &);
	///
	BufferParams const & bufferParams() const;
	/** Which language package do we require? \p englishbabel determines
	 *  if we require babel even if English is the only language.
	 */
	LangPackage langPackage() const;
	/// Convenience function to test if we use babel
	bool useBabel() const { return langPackage() == LANG_PACK_BABEL; }
	/// Convenience function to test if we use polyglossia
	bool usePolyglossia() const { return langPackage() == LANG_PACK_POLYGLOSSIA; }
	/// are we in a float?
	bool inFloat() const { return in_float_; }
	/// are we in a float?
	void inFloat(bool const b) { in_float_ = b; }
	/// Runparams that will be used for exporting this file.
	OutputParams const & runparams() const { return runparams_; }
	/// Resolve alternatives like "esint|amsmath|wasysym"
	void resolveAlternatives();
	/// Expand multiple requirements like "textcomp,lyxmathsym,amstext"
	void expandMultiples();
	///
	void setHTMLTitle(docstring const & t) { htmltitle_ = t; }
	///
	docstring const & htmlTitle() const { return htmltitle_; }

private:
	///
	void useLayout(docstring const &, int);
	///
	std::list<docstring> usedLayouts_;
	///
	std::list<docstring> usedInsetLayouts_;
	/// The features that are needed by the document
	typedef std::set<std::string> Features;
	///
	Features features_;
	/// Static preamble bits, from external templates, or anywhere else
	typedef std::list<std::string> SnippetList;
	///
	SnippetList preamble_snippets_;
	///
	SnippetList css_snippets_;
	///
	typedef std::set<Language const *> LanguageList;
	/// used languages (only those that are supported by babel)
	LanguageList UsedLanguages_;
	///
	typedef std::map<std::string, bool> UsedFloats;
	///
	UsedFloats usedFloats_;
	///
	typedef std::map<docstring, std::string> FileMap;
	///
	FileMap IncludedFiles_;
	/** Buffer of the file being processed.
	 *  This may be a child buffer of the to-be-exported file and
	 *  therefore may not be the buffer that belongs to params_.
	 *  Only needed by InsetInclude::validate().
	 */
	Buffer const * buffer_;
	///
	BufferParams const & params_;
	/** Some insets need to know details about the to-be-produced file
	 *  in validate().
	 */
	OutputParams const & runparams_;
	///
	bool in_float_;
	///
	docstring htmltitle_;
};


} // namespace lyx

#endif
