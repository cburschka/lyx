// -*- C++ -*-
/**
 * \file ExternalTemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXTERNALTEMPLATE_H
#define EXTERNALTEMPLATE_H

#include <boost/function.hpp>
#include <boost/utility.hpp>

#include <iosfwd>
#include <map>
#include <vector>

class LyXLex;

namespace lyx {
namespace external {

///
struct Template {
	/// We have to have default commands for safety reasons!
	Template();
	///
	void readTemplate(LyXLex &);
	///
	void dumpFormats(std::ostream &) const;

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
	/// What command should be executed to edit the file?
	std::string editCommand;
	/// Should we do automatic production of the output?
	bool automaticProduction;

	/// This is the information needed to support a specific output format
	struct Format {
		/// This constructor has to default a command for safety reasons!
		Format();
		///
		void readFormat(LyXLex &);

		/// The text that should be inserted into the exported file
		std::string product;
		/// The filename of the resulting file
		std::string updateResult;
		/// The format of this file.
		std::string updateFormat;
		/// What features does this external inset require?
		std::string requirement;
		/// A collection of preamble snippets identified by name.
		std::vector<std::string> preambleNames;
	};
	///
	typedef std::map<std::string, Format> Formats;
	///
	Formats formats;
};


/**
   A singleton class that manages the external inset templates
*/
class TemplateManager : boost::noncopyable {
public:
	/// Map from the LyX name of the template to the template structure
	typedef std::map<std::string, Template> Templates;
	/** Map from the LyX name of the preamble definition to the preamble
	 *  definition itself.
	 */
	typedef std::map<std::string, std::string> PreambleDefs;

	static TemplateManager & get();
	Templates & getTemplates();
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
	void readTemplates(std::string const & path);
	void dumpTemplates(std::ostream &) const;
	void dumpPreambleDefs(std::ostream &) const;

	Templates templates;
	PreambleDefs preambledefs;
};

} // namespace external
} // namespace lyx

#endif // EXTERNALTEMPLATE_H
