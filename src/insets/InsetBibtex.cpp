/**
 * \file InsetBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Richard Heck (BibTeX parser improvements)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBibtex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "DispatchResult.h"
#include "EmbeddedFiles.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TextClass.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Path.h"
#include "support/textutils.h"

#include <limits>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;


InsetBibtex::InsetBibtex(InsetCommandParams const & p)
	: InsetCommand(p, "bibtex"), bibfiles_()
{}


void InsetBibtex::setBuffer(Buffer & buffer)
{
	// FIXME We ought to have a buffer.
	if (buffer_) {
		EmbeddedFileList::iterator it = bibfiles_.begin();
		EmbeddedFileList::iterator en = bibfiles_.end();
		for (; it != en; ++it) {
			try {
				*it = it->copyTo(buffer);
			} catch (ExceptionMessage const & message) {
				Alert::error(message.title_, message.details_);
				// failed to embed
				it->setEmbed(false);
			}
		}
	}
	InsetCommand::setBuffer(buffer);
}


ParamInfo const & InsetBibtex::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("btprint", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("bibfiles", ParamInfo::LATEX_REQUIRED);
		param_info_.add("embed", ParamInfo::LYX_INTERNAL);
		param_info_.add("options", ParamInfo::LYX_INTERNAL);
	}
	return param_info_;
}


void InsetBibtex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBTEX_CODE);
		try {
			if (!InsetCommand::string2params("bibtex", 
					to_utf8(cmd.argument()), p)) {
				cur.noUpdate();
				break;
			}
		} catch (ExceptionMessage const & message) {
			if (message.type_ == WarningException) {
				Alert::warning(message.title_, message.details_);
				cur.noUpdate();
			} else 
				throw message;
			break;
		}
		//
		setParams(p);
		updateBibFiles();
		updateParam();
		buffer().updateBibfilesCache();
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


docstring InsetBibtex::screenLabel() const
{
	return _("BibTeX Generated Bibliography");
}


static string normalizeName(Buffer const & buffer,
	OutputParams const & runparams, string const & name, string const & ext)
{
	string const fname = makeAbsPath(name, buffer.filePath()).absFilename();
	if (FileName(name).isAbsolute() || !FileName(fname + ext).isReadableFile())
		return name;
	if (!runparams.nice)
		return fname;

	// FIXME UNICODE
	return to_utf8(makeRelPath(from_utf8(fname),
					 from_utf8(buffer.masterBuffer()->filePath())));
}


int InsetBibtex::latex(odocstream & os, OutputParams const & runparams) const
{
	// the sequence of the commands:
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	// and with bibtopic:
	// 1. \bibliographystyle{style}
	// 2. \begin{btSect}{database}
	// 3. \btPrint{Cited|NotCited|All}
	// 4. \end{btSect}

	// Database(s)
	// If we are processing the LaTeX file in a temp directory then
	// copy the .bib databases to this temp directory, mangling their
	// names in the process. Store this mangled name in the list of
	// all databases.
	// (We need to do all this because BibTeX *really*, *really*
	// can't handle "files with spaces" and Windows users tend to
	// use such filenames.)
	// Otherwise, store the (maybe absolute) path to the original,
	// unmangled database name.
	EmbeddedFileList::const_iterator it = bibfiles_.begin();
	EmbeddedFileList::const_iterator it_end = bibfiles_.end();
	odocstringstream dbs;
	for (; it != it_end; ++it) {
		string utf8input = removeExtension(it->availableFile().absFilename());
		string database =
			normalizeName(buffer(), runparams, utf8input, ".bib");
		FileName const try_in_file =
			makeAbsPath(database + ".bib", buffer().filePath());
		bool const not_from_texmf = try_in_file.isReadableFile();

		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    not_from_texmf) {

			// mangledFilename() needs the extension
			DocFileName const in_file = DocFileName(try_in_file);
			database = removeExtension(in_file.mangledFilename());
			FileName const out_file = makeAbsPath(database + ".bib",
					buffer().masterBuffer()->temppath());

			bool const success = in_file.copyTo(out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		} else if (!runparams.inComment && runparams.nice && not_from_texmf &&
			   !isValidLaTeXFilename(database)) {
				frontend::Alert::warning(_("Invalid filename"),
						         _("The following filename is likely to cause trouble "
							   "when running the exported file through LaTeX: ") +
							    from_utf8(database));
		}

		if (it != bibfiles_.begin())
			dbs << ',';
		// FIXME UNICODE
		dbs << from_utf8(latex_path(database));
	}
	docstring const db_out = dbs.str();

	// Post this warning only once.
	static bool warned_about_spaces = false;
	if (!warned_about_spaces &&
	    runparams.nice && db_out.find(' ') != docstring::npos) {
		warned_about_spaces = true;

		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the paths to your BibTeX databases.\n"
					      "BibTeX will be unable to find them."));
	}

	// Style-Options
	string style = to_utf8(getParam("options")); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ','))
			style = split(style, bibtotoc, ',');
	}

	// line count
	int nlines = 0;

	if (!style.empty()) {
		string base = normalizeName(buffer(), runparams, style, ".bst");
		FileName const try_in_file = 
			makeAbsPath(base + ".bst", buffer().filePath());
		bool const not_from_texmf = try_in_file.isReadableFile();
		// If this style does not come from texmf and we are not
		// exporting to .tex copy it to the tmp directory.
		// This prevents problems with spaces and 8bit charcaters
		// in the file name.
		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    not_from_texmf) {
			// use new style name
			DocFileName const in_file = DocFileName(try_in_file);
			base = removeExtension(in_file.mangledFilename());
			FileName const out_file = makeAbsPath(base + ".bst",
					buffer().masterBuffer()->temppath());
			bool const success = in_file.copyTo(out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		}
		// FIXME UNICODE
		os << "\\bibliographystyle{"
		   << from_utf8(latex_path(normalizeName(buffer(), runparams, base, ".bst")))
		   << "}\n";
		nlines += 1;
	}

	// Post this warning only once.
	static bool warned_about_bst_spaces = false;
	if (!warned_about_bst_spaces && runparams.nice && contains(style, ' ')) {
		warned_about_bst_spaces = true;
		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the path to your BibTeX style file.\n"
					      "BibTeX will be unable to find it."));
	}

	if (!db_out.empty() && buffer().params().use_bibtopic) {
		os << "\\begin{btSect}{" << db_out << "}\n";
		docstring btprint = getParam("btprint");
		if (btprint.empty())
			// default
			btprint = from_ascii("btPrintCited");
		os << "\\" << btprint << "\n"
		   << "\\end{btSect}\n";
		nlines += 3;
	}

	// bibtotoc-Option
	if (!bibtotoc.empty() && !buffer().params().use_bibtopic) {
		if (buffer().params().documentClass().hasLaTeXLayout("chapter")) {
			if (buffer().params().sides == OneSide) {
				// oneside
				os << "\\clearpage";
			} else {
				// twoside
				os << "\\cleardoublepage";
			}
			os << "\\addcontentsline{toc}{chapter}{\\bibname}";
		} else if (buffer().params().documentClass().hasLaTeXLayout("section"))
			os << "\\addcontentsline{toc}{section}{\\refname}";
	}

	if (!db_out.empty() && !buffer().params().use_bibtopic) {
		docstring btprint = getParam("btprint");
		if (btprint == "btPrintAll") {
			os << "\\nocite{*}\n";
			nlines += 1;
		}
		os << "\\bibliography{" << db_out << "}\n";
		nlines += 1;
	}

	return nlines;
}


EmbeddedFileList const & InsetBibtex::getBibFiles() const
{
	return bibfiles_;
}

namespace {

	// methods for parsing bibtex files

	typedef map<docstring, docstring> VarMap;

	/// remove whitespace characters, optionally a single comma,
	/// and further whitespace characters from the stream.
	/// @return true if a comma was found, false otherwise
	///
	bool removeWSAndComma(idocfstream & ifs) {
		char_type ch;

		if (!ifs)
			return false;

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (!ifs)
			return false;

		if (ch != ',') {
			ifs.putback(ch);
			return false;
		}

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (ifs) {
			ifs.putback(ch);
		}

		return true;
	}


	enum charCase {
		makeLowerCase,
		keepCase
	};

	/// remove whitespace characters, read characer sequence
	/// not containing whitespace characters or characters in
	/// delimChars, and remove further whitespace characters.
	///
	/// @return true if a string of length > 0 could be read.
	///
	bool readTypeOrKey(docstring & val, idocfstream & ifs,
		docstring const & delimChars, docstring const &illegalChars, 
		charCase chCase) {

		char_type ch;

		val.clear();

		if (!ifs)
			return false;

		// skip whitespace
		do {
			ifs.get(ch);
		} while (ifs && isSpace(ch));

		if (!ifs)
			return false;

		// read value
		bool legalChar = true;
		while (ifs && !isSpace(ch) && 
						 delimChars.find(ch) == docstring::npos &&
						 (legalChar = (illegalChars.find(ch) == docstring::npos))
					) 
		{
			if (chCase == makeLowerCase)
				val += lowercase(ch);
			else
				val += ch;
			ifs.get(ch);
		}
		
		if (!legalChar) {
			ifs.putback(ch);
			return false;
		}

		// skip whitespace
		while (ifs && isSpace(ch)) {
			ifs.get(ch);
		}

		if (ifs) {
			ifs.putback(ch);
		}

		return val.length() > 0;
	}

	/// read subsequent bibtex values that are delimited with a #-character.
	/// Concatenate all parts and replace names with the associated string in
	/// the variable strings.
	/// @return true if reading was successfull (all single parts were delimited
	/// correctly)
	bool readValue(docstring & val, idocfstream & ifs, const VarMap & strings) {

		char_type ch;

		val.clear();

		if (!ifs)
			return false;

		do {
			// skip whitespace
			do {
				ifs.get(ch);
			} while (ifs && isSpace(ch));

			if (!ifs)
				return false;

			// check for field type
			if (isDigit(ch)) {

				// read integer value
				do {
					val += ch;
					ifs.get(ch);
				} while (ifs && isDigit(ch));

				if (!ifs)
					return false;

			} else if (ch == '"' || ch == '{') {
				// set end delimiter
				char_type delim = ch == '"' ? '"': '}';

				//Skip whitespace
				do {
					ifs.get(ch);
				} while (ifs && isSpace(ch));
				
				if (!ifs)
					return false;
				
				//We now have the first non-whitespace character
				//We'll collapse adjacent whitespace.
				bool lastWasWhiteSpace = false;
				
 				// inside this delimited text braces must match.
 				// Thus we can have a closing delimiter only
 				// when nestLevel == 0
				int nestLevel = 0;
 
				while (ifs && (nestLevel > 0 || ch != delim)) {
					if (isSpace(ch)) {
						lastWasWhiteSpace = true;
						ifs.get(ch);
						continue;
					}
					//We output the space only after we stop getting 
					//whitespace so as not to output any whitespace
					//at the end of the value.
					if (lastWasWhiteSpace) {
						lastWasWhiteSpace = false;
						val += ' ';
					}
					
					val += ch;

					// update nesting level
					switch (ch) {
						case '{':
							++nestLevel;
							break;
						case '}':
							--nestLevel;
							if (nestLevel < 0) return false;
							break;
					}

					ifs.get(ch);
				}

				if (!ifs)
					return false;

				ifs.get(ch);

				if (!ifs)
					return false;

			} else {

				// reading a string name
				docstring strName;

				while (ifs && !isSpace(ch) && ch != '#' && ch != ',' && ch != '}' && ch != ')') {
					strName += lowercase(ch);
					ifs.get(ch);
				}

				if (!ifs)
					return false;

				// replace the string with its assigned value or
				// discard it if it's not assigned
				if (strName.length()) {
					VarMap::const_iterator pos = strings.find(strName);
					if (pos != strings.end()) {
						val += pos->second;
					}
				}
			}

			// skip WS
			while (ifs && isSpace(ch)) {
				ifs.get(ch);
			}

			if (!ifs)
				return false;

			// continue reading next value on concatenate with '#'
		} while (ch == '#');

		ifs.putback(ch);

		return true;
	}
}


// This method returns a comma separated list of Bibtex entries
void InsetBibtex::fillWithBibKeys(BiblioInfo & keylist,
	InsetIterator const & /*di*/) const
{
	// This bibtex parser is a first step to parse bibtex files
	// more precisely.
	//
	// - it reads the whole bibtex entry and does a syntax check
	//   (matching delimiters, missing commas,...
	// - it recovers from errors starting with the next @-character
	// - it reads @string definitions and replaces them in the
	//   field values.
	// - it accepts more characters in keys or value names than
	//   bibtex does.
	//
	// Officially bibtex does only support ASCII, but in practice
	// you can use the encoding of the main document as long as
	// some elements like keys and names are pure ASCII. Therefore
	// we convert the file from the buffer encoding.
	// We don't restrict keys to ASCII in LyX, since our own
	// InsetBibitem can generate non-ASCII keys, and nonstandard
	// 8bit clean bibtex forks exist.
	EmbeddedFileList const & files = getBibFiles();
	EmbeddedFileList::const_iterator it = files.begin();
	EmbeddedFileList::const_iterator en = files.end();
	for (; it != en; ++ it) {
		idocfstream ifs(it->availableFile().toFilesystemEncoding().c_str(),
			ios_base::in, buffer().params().encoding().iconvName());

		char_type ch;
		VarMap strings;

		while (ifs) {

			ifs.get(ch);
			if (!ifs)
				break;

			if (ch != '@')
				continue;

			docstring entryType;

			if (!readTypeOrKey(entryType, ifs, from_ascii("{("), 
			                   docstring(), makeLowerCase) || !ifs)
				continue;

			if (entryType == from_ascii("comment")) {

				ifs.ignore(numeric_limits<int>::max(), '\n');
				continue;
			}

			ifs.get(ch);
			if (!ifs)
				break;

			if ((ch != '(') && (ch != '{')) {
				// invalid entry delimiter
				ifs.putback(ch);
				continue;
			}

			// process the entry
			if (entryType == from_ascii("string")) {

				// read string and add it to the strings map
				// (or replace it's old value)
				docstring name;
				docstring value;

				if (!readTypeOrKey(name, ifs, from_ascii("="), 
				                   from_ascii("#{}(),"), makeLowerCase) || !ifs)
					continue;

				// next char must be an equal sign
				ifs.get(ch);
				if (!ifs || ch != '=')
					continue;

				if (!readValue(value, ifs, strings))
					continue;

				strings[name] = value;

			} else if (entryType == from_ascii("preamble")) {

				// preamble definitions are discarded.
				// can they be of any use in lyx?
				docstring value;

				if (!readValue(value, ifs, strings))
					continue;

			} else {

				// Citation entry. Try to read the key.
				docstring key;

				if (!readTypeOrKey(key, ifs, from_ascii(","), 
				                   from_ascii("}"), keepCase) || !ifs)
					continue;

				/////////////////////////////////////////////
				// now we have a key, so we will add an entry 
 				// (even if it's empty, as bibtex does)
				//
				// we now read the field = value pairs.
 				// all items must be separated by a comma. If
 				// it is missing the scanning of this entry is
 				// stopped and the next is searched.
				docstring fields;
				docstring name;
				docstring value;
				docstring commaNewline;
				docstring data;
				BibTeXInfo keyvalmap(key, entryType);
				
				bool readNext = removeWSAndComma(ifs);
 
				while (ifs && readNext) {

					// read field name
					if (!readTypeOrKey(name, ifs, from_ascii("="), 
					                   from_ascii("{}(),"), makeLowerCase) || !ifs)
						break;

					// next char must be an equal sign
					ifs.get(ch);
					if (!ifs)
						break;
					if (ch != '=') {
						ifs.putback(ch);
						break;
					}

					// read field value
					if (!readValue(value, ifs, strings))
						break;

					keyvalmap[name] = value;
					data += "\n\n" + value;
					keylist.addFieldName(name);
					readNext = removeWSAndComma(ifs);
				}

				// add the new entry
				keylist.addEntryType(entryType);
				keyvalmap.setAllData(data);
				keylist[key] = keyvalmap;
			}
		} //< searching '@'
	} //< for loop over files
}


FileName InsetBibtex::getBibTeXPath(docstring const & filename, Buffer const & buf)
{
	string texfile = changeExtension(to_utf8(filename), "bib");
	// note that, if the filename can be found directly from the path, 
	// findtexfile will just return a FileName object for that path.
	FileName file(findtexfile(texfile, "bib"));
	if (file.empty())
		file = FileName(makeAbsPath(texfile, buf.filePath()));
	return file;
}
 

bool InsetBibtex::addDatabase(docstring const & db)
{
	FileName const texPath = getBibTeXPath(db, buffer());
	EmbeddedFileList::const_iterator efp = 
		bibfiles_.findFile(texPath.absFilename());
	if (efp != bibfiles_.end())
		//already have it
		return false;

	EmbeddedFile file(texPath.absFilename(), buffer().filePath());
	bibfiles_.push_back(file);
	updateParam();
	return true;
}


bool InsetBibtex::delDatabase(docstring const & db)
{
	FileName const texPath = getBibTeXPath(db, buffer());
	EmbeddedFileList::iterator efp = 
		bibfiles_.findFile(texPath.absFilename());
	if (efp == bibfiles_.end())
		// don't have it
		return false;
	
	bibfiles_.erase(efp);
	updateParam();
	return true;
}


void InsetBibtex::validate(LaTeXFeatures & features) const
{
	if (features.bufferParams().use_bibtopic)
		features.require("bibtopic");
}


void InsetBibtex::updateBibFiles() const
{
	// need to do this to keep old info while also not
	// copying over any files that have been deleted.
	EmbeddedFileList oldlist = bibfiles_;
	bibfiles_.clear();
	
	docstring bibfile;
	docstring embfile;
	
	docstring bibfiles = getParam("bibfiles");
	docstring embfiles = getParam("embed");
	
	bibfiles = split(bibfiles, bibfile, ',');
	embfiles = split(embfiles, embfile, ',');
	
	while (!bibfile.empty()) {
		FileName bib = getBibTeXPath(bibfile, buffer());
		EmbeddedFileList::iterator efp = oldlist.findFile(bib.absFilename());
		if (efp != oldlist.end()) {
			// already have this one, but embedding status may have been changed
			bool old_status = efp->embedded();
			bool new_status = !embfile.empty();
			efp->setEmbed(new_status);
			try {
				// copy file if embedding status changed.
				efp->enable(buffer().embedded(), buffer(), old_status != new_status);
			} catch (ExceptionMessage const & message) {
				Alert::error(message.title_, message.details_);
				// failed to change embeddeing status
				efp->setEmbed(old_status);
			}
			bibfiles_.push_back(*efp);
		} else {
			EmbeddedFile file(bib.absFilename(), buffer().filePath());
			file.setEmbed(!embfile.empty());
			try {
				file.enable(buffer().embedded(), buffer(), true);
			} catch (ExceptionMessage const & message) {
				Alert::error(message.title_, message.details_);
				// failed to embed
				file.setEmbed(false);
			}
			bibfiles_.push_back(file);
		}
		// Get next file name
		bibfiles = split(bibfiles, bibfile, ',');
		embfiles = split(embfiles, embfile, ',');
	}
}


void InsetBibtex::updateParam()
{
	docstring bibfiles;
	docstring embed;

	bool first = true;

	EmbeddedFileList::iterator it = bibfiles_.begin();
	EmbeddedFileList::iterator en = bibfiles_.end();
	for (; it != en; ++it) {
		if (!first) {
			bibfiles += ',';
			embed += ',';
		} else
			first = false;
		bibfiles += from_utf8(it->outputFilename(buffer().filePath()));
		if (it->embedded())
			embed += from_utf8(it->inzipName());
	}
	setParam("bibfiles", bibfiles);
	setParam("embed", embed);
}


void InsetBibtex::registerEmbeddedFiles(EmbeddedFileList & files) const
{
	if (bibfiles_.empty())
		updateBibFiles();

	EmbeddedFileList::const_iterator it = bibfiles_.begin();
	EmbeddedFileList::const_iterator en = bibfiles_.end();
	for (; it != en; ++it)
		files.registerFile(*it, this, buffer());
}


void InsetBibtex::updateEmbeddedFile(EmbeddedFile const & file)
{
	// look for the item and update status
	string const filename = file.absFilename();
	EmbeddedFileList::iterator it = bibfiles_.findFile(filename);
	if (it == bibfiles_.end())
		return;
	*it = file;
	updateParam();
}


} // namespace lyx
