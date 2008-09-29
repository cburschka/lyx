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
#include <string>


namespace lyx {

class Buffer;
class BufferParams;
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
	///
	LaTeXFeatures(Buffer const &, BufferParams const &,
		      OutputParams const &);
	/// The color packages
	std::string const getColorOptions() const;
	/// The packages needed by the document
	std::string const getPackages() const;
	/// The macros definitions needed by the document
	std::string const getMacros() const;
	///
	std::string const getBabelOptions() const;
	/// The definitions needed by the document's textclass
	docstring const getTClassPreamble() const;
	/// The sgml definitions needed by the document (docbook)
	docstring const getLyXSGMLEntities() const;
	/// The SGML Required to include the files added with includeFile();
	docstring const getIncludedFiles(std::string const & fname) const;
	/// Include a file for use with the SGML entities
	void includeFile(docstring const & key, std::string const & name);
	/// The float definitions.
	void getFloatDefinitions(std::ostream & os) const;
	/// Print requirements to lyxerr
	void showStruct() const;
	///
	void addPreambleSnippet(std::string const &);
	/// Add a feature name requirements
	void require(std::string const & name);
	/// Add a set of feature names requirements
	void require(std::set<std::string> const & names);
	/// Which of the required packages are installed?
	static void getAvailable();
	/// Is the (required) package available?
	static bool isAvailable(std::string const & name);
	/// Has the package been required?
	bool isRequired(std::string const & name) const;
	/* Is it necessary to load the package? This is true if
	   isRequired is true and the feature is not provided by the
	   textclass.
	*/
	bool mustProvide(std::string const & name) const;
	///
	void useFloat(std::string const & name, bool subfloat = false);
	///
	void useLanguage(Language const *);
	///
	bool hasLanguages() const;
	///
	std::string getLanguages() const;
	///
	std::set<std::string> getEncodingSet(std::string const & doc_encoding) const;
	///
	void useLayout(docstring const & lyt);
	///
	Buffer const & buffer() const;
	///
	void setBuffer(Buffer const &);
	///
	BufferParams const & bufferParams() const;
	/// the return value is dependent upon both LyXRC and LaTeXFeatures.
	bool useBabel() const;
	/// Runparams that will be used for exporting this file.
	OutputParams const & runparams() const { return runparams_; }

private:
	std::list<docstring> usedLayouts_;

	/// The features that are needed by the document
	typedef std::set<std::string> Features;
	///
	Features features_;
	/// Static preamble bits from the external material insets
	typedef std::list<std::string> SnippetList;
	///
	SnippetList preamble_snippets_;
	/// The available (required) packages
	typedef std::set<std::string> Packages;
	///
	static Packages packages_;
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
};


} // namespace lyx

#endif
