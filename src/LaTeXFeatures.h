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


#include <set>
#include <list>
#include <map>

class Buffer;
class BufferParams;
struct Language;

/** The packages and commands that a buffer needs. This class
    contains a list<string>.  Each of the LaTeX packages that a buffer needs
    should be added with void require(string const & name).

    i.e require("amssymb")

    To add support you should only need to require() the package name as
    packages which don't have special requirements are handled automatically.
    If your new package does need special consideration you'll need to alter
    string const getPackages() const;
    Remember to update the validate function in buffer.C and paragraph.C
    when you do so.
*/
class LaTeXFeatures {
public:
	///
	LaTeXFeatures(Buffer const &, BufferParams const &);
	/// The packages needed by the document
	std::string const getPackages() const;
	/// The macros definitions needed by the document
	std::string const getMacros() const;
	///
	std::string const getBabelOptions() const;
	/// The definitions needed by the document's textclass
	std::string const getTClassPreamble() const;
	/// The sgml definitions needed by the document (dobook/linuxdoc)
	std::string const getLyXSGMLEntities() const;
	/// The SGML Required to include the files added with includeFile();
	std::string const getIncludedFiles(std::string const & fname) const;
	/// Include a file for use with the SGML entities
	void includeFile(std::string const & key, std::string const & name);
	/// The float definitions.
	void getFloatDefinitions(std::ostream & os) const;
	/// Print requirements to lyxerr
	void showStruct() const;
	///
	void addExternalPreamble(std::string const &);
	/// Provide a string name-space to the requirements
	void require(std::string const & name);
	/// Is the package required?
	bool isRequired(std::string const & name) const;
	///
	void useFloat(std::string const & name);
	///
	void useLanguage(Language const *);
	///
	bool hasLanguages() const;
	///
	std::string getLanguages() const;
	///
	std::set<std::string> getEncodingSet(std::string const & doc_encoding) const;
	///
	void useLayout(std::string const & lyt);
	///
	Buffer const & buffer() const;
	///
	BufferParams const & bufferParams() const;
	/// the return value is dependent upon both LyXRC and LaTeXFeatures.
	bool useBabel() const;

private:
	std::list<std::string> usedLayouts_;

	/// Static preamble bits from the external material insets
	typedef std::list<std::string> FeaturesList;
	///
	FeaturesList features_;
	///
	FeaturesList preamble_snippets_;
	///
	typedef std::set<Language const *> LanguageList;
	///
	LanguageList UsedLanguages_;
	///
	typedef std::set<std::string> UsedFloats;
	///
	UsedFloats usedFloats_;
	///
	typedef std::map<std::string , std::string> FileMap;
	///
	FileMap IncludedFiles_;
	///
	Buffer const & buffer_;
	///
	BufferParams const & params_;
};

#endif
