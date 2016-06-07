// -*- C++ -*-
/**
 * \file ExternalTemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXTERNALTEMPLATE_H
#define EXTERNALTEMPLATE_H

#include "ExternalTransforms.h"

#include <vector>

namespace lyx {

namespace support { class FileName; }

class Lexer;

enum PreviewMode {
	PREVIEW_OFF = 0,
	PREVIEW_GRAPHICS,
	PREVIEW_INSTANT
};

namespace external {

class Template {
public:
	/// We have to have default commands for safety reasons!
	Template();
	///
	void readTemplate(Lexer &);
	///
	void dumpFormats(std::ostream &) const;

	class Option {
	public:
		Option(std::string const & name_, std::string const & opt_)
			: name(name_), option(opt_) {}
		std::string name;
		std::string option;
	};

	/// What is the name of this template in the LyX format?
	std::string lyxName;
	/// What will the button in the GUI say?
	std::string guiName;
	/// A short help text
	std::string helpText;
	/** The format of the input file. Can be "*", in which case we try and
	 *   ascertain the format from the contents of the file.
	 */
	std::string inputFormat;
	/// A file extension regular expression for the file browser
	std::string fileRegExp;
	/// Should we do automatic production of the output?
	bool automaticProduction;
	/// How should we preview the data in LyX?
	PreviewMode preview_mode;
	/// A collection of transforms that we can use to transform the data.
	std::vector<TransformID> transformIds;

	/// This is the information needed to support a specific output format
	class Format {
	public:
		Format();
		///
		void readFormat(Lexer &);

		/// The text that should be inserted into the exported file
		std::string product;
		/// The filename of the resulting file
		std::string updateResult;
		/// The format of this file.
		std::string updateFormat;
		/// What features does this external inset require?
		std::vector<std::string> requirements;
		/// A collection of preamble snippets identified by name.
		std::vector<std::string> preambleNames;
		/// A list of options to the basic command.
		std::vector<Option> options;

		/// The factory functions for each supported transformation.
		std::map<TransformID, TransformStore> command_transformers;
		std::map<TransformID, TransformStore> option_transformers;

		typedef std::map<std::string,
				 std::vector<std::string> > FileMap;
		/// Referenced files for some formats
		FileMap referencedFiles;
	};
	///
	typedef std::map<std::string, Format> Formats;
	///
	Formats formats;
};

/**
 *  A singleton class that manages the external inset templates
 */
class TemplateManager {
public:
	/// Map from the LyX name of the template to the template structure
	typedef std::map<std::string, Template> Templates;
	/** Map from the LyX name of the preamble definition to the preamble
	 *  definition itself.
	 */
	typedef std::map<std::string, std::string> PreambleDefs;

	static TemplateManager & get();

	Templates const & getTemplates() const;
	/** return the template by LyX name.
	 *  If it isn't found, return 0.
	 */
	Template const * getTemplateByName(std::string const & name) const;
	/** return the preamble definition by LyX name.
	 *  If it isn't found, return an empty std::string.
	 */
	std::string const getPreambleDefByName(std::string const & name) const;
private:
	TemplateManager();
	/// noncopyable
	TemplateManager(TemplateManager const &);
	void operator=(TemplateManager const &);

	void readTemplates(support::FileName const & path);
	void dumpTemplates(std::ostream &) const;
	void dumpPreambleDefs(std::ostream &) const;

	Templates templates;
	PreambleDefs preambledefs;
};

} // namespace external
} // namespace lyx

#endif // EXTERNALTEMPLATE_H
