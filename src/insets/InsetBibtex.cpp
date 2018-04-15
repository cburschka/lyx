/**
 * \file InsetBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Richard Heck (BibTeX parser improvements)
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBibtex.h"

#include "BiblioInfo.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "CiteEnginesList.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "Exporter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LaTeXFeatures.h"
#include "output_xhtml.h"
#include "OutputParams.h"
#include "PDFOptions.h"
#include "texstream.h"
#include "TextClass.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/ExceptionMessage.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/PathChanger.h"
#include "support/textutils.h"

#include <limits>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;
namespace os = support::os;


InsetBibtex::InsetBibtex(Buffer * buf, InsetCommandParams const & p)
	: InsetCommand(buf, p)
{}


ParamInfo const & InsetBibtex::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("btprint", ParamInfo::LATEX_OPTIONAL);
		param_info_.add("bibfiles", ParamInfo::LATEX_REQUIRED);
		param_info_.add("options", ParamInfo::LYX_INTERNAL);
		param_info_.add("biblatexopts", ParamInfo::LATEX_OPTIONAL);
	}
	return param_info_;
}


void InsetBibtex::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_EDIT:
		editDatabases();
		break;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p(BIBTEX_CODE);
		try {
			if (!InsetCommand::string2params(to_utf8(cmd.argument()), p)) {
				cur.noScreenUpdate();
				break;
			}
		} catch (ExceptionMessage const & message) {
			if (message.type_ == WarningException) {
				Alert::warning(message.title_, message.details_);
				cur.noScreenUpdate();
			} else
				throw;
			break;
		}

		cur.recordUndo();
		setParams(p);
		cur.forceBufferUpdate();
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


bool InsetBibtex::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {
	case LFUN_INSET_EDIT:
		flag.setEnabled(true);
		return true;

	default:
		return InsetCommand::getStatus(cur, cmd, flag);
	}
}


void InsetBibtex::editDatabases() const
{
	vector<docstring> bibfilelist = getVectorFromString(getParam("bibfiles"));

	if (bibfilelist.empty())
		return;

	int nr_databases = bibfilelist.size();
	if (nr_databases > 1) {
			docstring const engine = usingBiblatex() ? _("Biblatex") : _("BibTeX");
			docstring message = bformat(_("The %1$s[[BibTeX/Biblatex]] inset includes %2$s databases.\n"
						       "If you proceed, all of them will be opened."),
							engine, convert<docstring>(nr_databases));
			int const ret = Alert::prompt(_("Open Databases?"),
				message, 0, 1, _("&Cancel"), _("&Proceed"));

			if (ret == 0)
				return;
	}

	vector<docstring>::const_iterator it = bibfilelist.begin();
	vector<docstring>::const_iterator en = bibfilelist.end();
	for (; it != en; ++it) {
		FileName const bibfile = getBibTeXPath(*it, buffer());
		theFormats().edit(buffer(), bibfile,
		     theFormats().getFormatFromFile(bibfile));
	}
}


bool InsetBibtex::usingBiblatex() const
{
	return buffer().masterParams().useBiblatex();
}


docstring InsetBibtex::screenLabel() const
{
	return usingBiblatex() ? _("Biblatex Generated Bibliography")
			       : _("BibTeX Generated Bibliography");
}


docstring InsetBibtex::toolTip(BufferView const & /*bv*/, int /*x*/, int /*y*/) const
{
	docstring tip = _("Databases:");
	vector<docstring> bibfilelist = getVectorFromString(getParam("bibfiles"));

	tip += "<ul>";
	if (bibfilelist.empty())
		tip += "<li>" + _("none") + "</li>";
	else
		for (docstring const & bibfile : bibfilelist)
			tip += "<li>" + bibfile + "</li>";
	tip += "</ul>";

	// Style-Options
	bool toc = false;
	docstring style = getParam("options"); // maybe empty! and with bibtotoc
	docstring bibtotoc = from_ascii("bibtotoc");
	if (prefixIs(style, bibtotoc)) {
		toc = true;
		if (contains(style, char_type(',')))
			style = split(style, bibtotoc, char_type(','));
	}

	docstring const btprint = getParam("btprint");
	if (!usingBiblatex()) {
		tip += _("Style File:");
		tip += "<ul><li>" + (style.empty() ? _("none") : style) + "</li></ul>";

		tip += _("Lists:") + " ";
		if (btprint == "btPrintAll")
			tip += _("all references");
		else if (btprint == "btPrintNotCited")
			tip += _("all uncited references");
		else
			tip += _("all cited references");
		if (toc) {
			tip += ", ";
			tip += _("included in TOC");
		}
		if (!buffer().parent()
		    && buffer().params().multibib == "child") {
			tip += "<br />";
			tip += _("Note: This bibliography is not output, since bibliographies in the master file "
				 "are not allowed with the setting 'Multiple bibliographies per child document'");
		}
	} else {
		tip += _("Lists:") + " ";
		if (btprint == "bibbysection")
			tip += _("all reference units");
		else if (btprint == "btPrintAll")
			tip += _("all references");
		else
			tip += _("all cited references");
		if (toc) {
			tip += ", ";
			tip += _("included in TOC");
		}
		if (!getParam("biblatexopts").empty()) {
			if (toc)
				tip += "<br />";
			tip += _("Options: ") + getParam("biblatexopts");
		}
	}

	return tip;
}


void InsetBibtex::latex(otexstream & os, OutputParams const & runparams) const
{
	// The sequence of the commands:
	// With normal BibTeX:
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	// With bibtopic:
	// 1. \bibliographystyle{style}
	// 2. \begin{btSect}{database}
	// 3. \btPrint{Cited|NotCited|All}
	// 4. \end{btSect}
	// With Biblatex:
	// \printbibliography[biblatexopts]
	// or
	// \bibbysection[biblatexopts] - if btprint is "bibbysection"

	// chapterbib does not allow bibliographies in the master
	if (!usingBiblatex() && !runparams.is_child
	    && buffer().params().multibib == "child")
		return;

	string style = to_utf8(getParam("options")); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ','))
			style = split(style, bibtotoc, ',');
	}

	if (usingBiblatex()) {
		// Options
		string opts = to_utf8(getParam("biblatexopts"));
		// bibtotoc-Option
		if (!bibtotoc.empty())
			opts = opts.empty() ? "heading=bibintoc" : "heading=bibintoc," + opts;
		// The bibliography command
		docstring btprint = getParam("btprint");
		if (btprint == "btPrintAll")
			os << "\\nocite{*}\n";
		if (btprint == "bibbysection" && !buffer().masterParams().multibib.empty())
			os << "\\bibbysection";
		else
			os << "\\printbibliography";
		if (!opts.empty())
			os << "[" << opts << "]";
		os << "\n";
	} else {// using BibTeX
		// Database(s)
		vector<docstring> const db_out =
			buffer().prepareBibFilePaths(runparams, getBibFiles(), false);
		// Style options
		if (style == "default")
			style = buffer().masterParams().defaultBiblioStyle();
		if (!style.empty() && !buffer().masterParams().useBibtopic()) {
			string base = buffer().masterBuffer()->prepareFileNameForLaTeX(style, ".bst", runparams.nice);
			FileName const try_in_file =
				makeAbsPath(base + ".bst", buffer().filePath());
			bool const not_from_texmf = try_in_file.isReadableFile();
			// If this style does not come from texmf and we are not
			// exporting to .tex copy it to the tmp directory.
			// This prevents problems with spaces and 8bit characters
			// in the file name.
			if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
			    not_from_texmf) {
				// use new style name
				DocFileName const in_file = DocFileName(try_in_file);
				base = removeExtension(in_file.mangledFileName());
				FileName const out_file = makeAbsPath(base + ".bst",
						buffer().masterBuffer()->temppath());
				bool const success = in_file.copyTo(out_file);
				if (!success) {
					LYXERR0("Failed to copy '" << in_file
					       << "' to '" << out_file << "'");
				}
			}
			// FIXME UNICODE
			os << "\\bibliographystyle{"
			   << from_utf8(latex_path(buffer().prepareFileNameForLaTeX(base, ".bst", runparams.nice)))
			   << "}\n";
		}
		// Warn about spaces in bst path. Warn only once.
		static bool warned_about_bst_spaces = false;
		if (!warned_about_bst_spaces && runparams.nice && contains(style, ' ')) {
			warned_about_bst_spaces = true;
			Alert::warning(_("Export Warning!"),
				       _("There are spaces in the path to your BibTeX style file.\n"
						      "BibTeX will be unable to find it."));
		}
		// Handle the bibtopic case
		if (!db_out.empty() && buffer().masterParams().useBibtopic()) {
			os << "\\begin{btSect}";
			if (!style.empty())
				os << "[" << style << "]";
			os << "{" << getStringFromVector(db_out) << "}\n";
			docstring btprint = getParam("btprint");
			if (btprint.empty())
				// default
				btprint = from_ascii("btPrintCited");
			os << "\\" << btprint << "\n"
			   << "\\end{btSect}\n";
		}
		// bibtotoc option
		if (!bibtotoc.empty() && !buffer().masterParams().useBibtopic()) {
			// set label for hyperref, see http://www.lyx.org/trac/ticket/6470
			if (buffer().masterParams().pdfoptions().use_hyperref)
					os << "\\phantomsection";
			if (buffer().masterParams().documentClass().hasLaTeXLayout("chapter"))
				os << "\\addcontentsline{toc}{chapter}{\\bibname}";
			else if (buffer().masterParams().documentClass().hasLaTeXLayout("section"))
				os << "\\addcontentsline{toc}{section}{\\refname}";
		}
		// The bibliography command
		if (!db_out.empty() && !buffer().masterParams().useBibtopic()) {
			docstring btprint = getParam("btprint");
			if (btprint == "btPrintAll") {
				os << "\\nocite{*}\n";
			}
			os << "\\bibliography{" << getStringFromVector(db_out) << "}\n";
		}
	}
}


FileNamePairList InsetBibtex::getBibFiles() const
{
	FileName path(buffer().filePath());
	PathChanger p(path);

	// We need to store both the real FileName and the way it is entered
	// (with full path, rel path or as a single file name).
	// The latter is needed for biblatex's central bibfile macro.
	FileNamePairList vec;

	vector<docstring> bibfilelist = getVectorFromString(getParam("bibfiles"));
	vector<docstring>::const_iterator it = bibfilelist.begin();
	vector<docstring>::const_iterator en = bibfilelist.end();
	for (; it != en; ++it) {
		FileName const file = getBibTeXPath(*it, buffer());

		if (!file.empty())
			vec.push_back(make_pair(*it, file));
		else
			LYXERR0("Couldn't find " + to_utf8(*it) + " in InsetBibtex::getBibFiles()!");
	}

	return vec;
}

namespace {

	// methods for parsing bibtex files

	typedef map<docstring, docstring> VarMap;

	/// remove whitespace characters, optionally a single comma,
	/// and further whitespace characters from the stream.
	/// @return true if a comma was found, false otherwise
	///
	bool removeWSAndComma(ifdocstream & ifs) {
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
	bool readTypeOrKey(docstring & val, ifdocstream & ifs,
		docstring const & delimChars, docstring const & illegalChars,
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
	bool readValue(docstring & val, ifdocstream & ifs, const VarMap & strings) {

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
			if (isDigitASCII(ch)) {

				// read integer value
				do {
					val += ch;
					ifs.get(ch);
				} while (ifs && isDigitASCII(ch));

				if (!ifs)
					return false;

			} else if (ch == '"' || ch == '{') {
				// set end delimiter
				char_type delim = ch == '"' ? '"': '}';

				// Skip whitespace
				do {
					ifs.get(ch);
				} while (ifs && isSpace(ch));

				if (!ifs)
					return false;

				// We now have the first non-whitespace character
				// We'll collapse adjacent whitespace.
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
					// We output the space only after we stop getting
					// whitespace so as not to output any whitespace
					// at the end of the value.
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
							if (nestLevel < 0)
								return false;
							break;
					}

					if (ifs)
						ifs.get(ch);
				}

				if (!ifs)
					return false;

				// FIXME Why is this here?
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
} // namespace


void InsetBibtex::collectBibKeys(InsetIterator const & /*di*/, FileNameList & checkedFiles) const
{
	parseBibTeXFiles(checkedFiles);
}


void InsetBibtex::parseBibTeXFiles(FileNameList & checkedFiles) const
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

	BiblioInfo keylist;

	FileNamePairList const files = getBibFiles();
	FileNamePairList::const_iterator it = files.begin();
	FileNamePairList::const_iterator en = files.end();
	for (; it != en; ++ it) {
		FileName const bibfile = it->second;
		if (find(checkedFiles.begin(), checkedFiles.end(), bibfile) != checkedFiles.end())
			// already checked this one. Skip.
			continue;
		else
			// record that we check this.
			checkedFiles.push_back(bibfile);
		ifdocstream ifs(bibfile.toFilesystemEncoding().c_str(),
			ios_base::in, buffer().masterParams().encoding().iconvName());

		char_type ch;
		VarMap strings;

		while (ifs) {

			ifs.get(ch);
			if (!ifs)
				break;

			if (ch != '@')
				continue;

			docstring entryType;

			if (!readTypeOrKey(entryType, ifs, from_ascii("{("), docstring(), makeLowerCase)) {
				lyxerr << "BibTeX Parser: Error reading entry type." << std::endl;
				continue;
			}

			if (!ifs) {
				lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
				continue;
			}

			if (entryType == from_ascii("comment")) {
				ifs.ignore(numeric_limits<int>::max(), '\n');
				continue;
			}

			ifs.get(ch);
			if (!ifs) {
				lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
				break;
			}

			if ((ch != '(') && (ch != '{')) {
				lyxerr << "BibTeX Parser: Invalid entry delimiter." << std::endl;
				ifs.putback(ch);
				continue;
			}

			// process the entry
			if (entryType == from_ascii("string")) {

				// read string and add it to the strings map
				// (or replace it's old value)
				docstring name;
				docstring value;

				if (!readTypeOrKey(name, ifs, from_ascii("="), from_ascii("#{}(),"), makeLowerCase)) {
					lyxerr << "BibTeX Parser: Error reading string name." << std::endl;
					continue;
				}

				if (!ifs) {
					lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
					continue;
				}

				// next char must be an equal sign
				ifs.get(ch);
				if (!ifs || ch != '=') {
					lyxerr << "BibTeX Parser: No `=' after string name: " <<
							name << "." << std::endl;
					continue;
				}

				if (!readValue(value, ifs, strings)) {
					lyxerr << "BibTeX Parser: Unable to read value for string: " <<
							name << "." << std::endl;
					continue;
				}

				strings[name] = value;

			} else if (entryType == from_ascii("preamble")) {

				// preamble definitions are discarded.
				// can they be of any use in lyx?
				docstring value;

				if (!readValue(value, ifs, strings)) {
					lyxerr << "BibTeX Parser: Unable to read preamble value." << std::endl;
					continue;
				}

			} else {

				// Citation entry. Try to read the key.
				docstring key;

				if (!readTypeOrKey(key, ifs, from_ascii(","), from_ascii("}"), keepCase)) {
					lyxerr << "BibTeX Parser: Unable to read key for entry type:" <<
							entryType << "." << std::endl;
					continue;
				}

				if (!ifs) {
					lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
					continue;
				}

				/////////////////////////////////////////////
				// now we have a key, so we will add an entry
 				// (even if it's empty, as bibtex does)
				//
				// we now read the field = value pairs.
 				// all items must be separated by a comma. If
 				// it is missing the scanning of this entry is
 				// stopped and the next is searched.
				docstring name;
				docstring value;
				docstring data;
				BibTeXInfo keyvalmap(key, entryType);

				bool readNext = removeWSAndComma(ifs);

				while (ifs && readNext) {

					// read field name
					if (!readTypeOrKey(name, ifs, from_ascii("="),
					                   from_ascii("{}(),"), makeLowerCase) || !ifs)
						break;

					// next char must be an equal sign
					// FIXME Whitespace??
					ifs.get(ch);
					if (!ifs) {
						lyxerr << "BibTeX Parser: Unexpected end of file." << std::endl;
						break;
					}
					if (ch != '=') {
						lyxerr << "BibTeX Parser: Missing `=' after field name: " <<
								name << ", for key: " << key << "." << std::endl;
						ifs.putback(ch);
						break;
					}

					// read field value
					if (!readValue(value, ifs, strings)) {
						lyxerr << "BibTeX Parser: Unable to read value for field: " <<
								name << ", for key: " << key << "." << std::endl;
						break;
					}

					keyvalmap[name] = value;
					data += "\n\n" + value;
					keylist.addFieldName(name);
					readNext = removeWSAndComma(ifs);
				}

				// add the new entry
				keylist.addEntryType(entryType);
				keyvalmap.setAllData(data);
				keylist[key] = keyvalmap;
			} //< else (citation entry)
		} //< searching '@'
	} //< for loop over files

	buffer().addBiblioInfo(keylist);
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
	docstring bibfiles = getParam("bibfiles");
	if (tokenPos(bibfiles, ',', db) != -1)
		return false;
	if (!bibfiles.empty())
		bibfiles += ',';
	setParam("bibfiles", bibfiles + db);
	return true;
}


bool InsetBibtex::delDatabase(docstring const & db)
{
	docstring bibfiles = getParam("bibfiles");
	if (contains(bibfiles, db)) {
		int const n = tokenPos(bibfiles, ',', db);
		docstring bd = db;
		if (n > 0) {
			// this is not the first database
			docstring tmp = ',' + bd;
			setParam("bibfiles", subst(bibfiles, tmp, docstring()));
		} else if (n == 0)
			// this is the first (or only) database
			setParam("bibfiles", split(bibfiles, bd, ','));
		else
			return false;
	}
	return true;
}


void InsetBibtex::validate(LaTeXFeatures & features) const
{
	BufferParams const & mparams = features.buffer().masterParams();
	if (mparams.useBibtopic())
		features.require("bibtopic");
	else if (!mparams.useBiblatex() && mparams.multibib == "child")
		features.require("chapterbib");
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().flavor == OutputParams::HTML)
		features.addCSSSnippet("div.bibtexentry { margin-left: 2em; text-indent: -2em; }\n"
			"span.bibtexlabel:before{ content: \"[\"; }\n"
			"span.bibtexlabel:after{ content: \"] \"; }");
}


void InsetBibtex::updateBuffer(ParIterator const &, UpdateType) {
	buffer().registerBibfiles(getBibFiles());
}


int InsetBibtex::plaintext(odocstringstream & os,
       OutputParams const & op, size_t max_length) const
{
	docstring const reflabel = buffer().B_("References");

	// We could output more information here, e.g., what databases are included
	// and information about options. But I don't necessarily see any reason to
	// do this right now.
	if (op.for_tooltip || op.for_toc || op.for_search) {
		os << '[' << reflabel << ']' << '\n';
		return PLAINTEXT_NEWLINE;
	}

	BiblioInfo bibinfo = buffer().masterBibInfo();
	bibinfo.makeCitationLabels(buffer());
	vector<docstring> const & cites = bibinfo.citedEntries();

	size_t start_size = os.str().size();
	docstring refoutput;
	refoutput += reflabel + "\n\n";

	// Tell BiblioInfo our purpose
	CiteItem ci;
	ci.context = CiteItem::Export;

	// Now we loop over the entries
	vector<docstring>::const_iterator vit = cites.begin();
	vector<docstring>::const_iterator const ven = cites.end();
	for (; vit != ven; ++vit) {
		if (start_size + refoutput.size() >= max_length)
			break;
		BiblioInfo::const_iterator const biit = bibinfo.find(*vit);
		if (biit == bibinfo.end())
			continue;
		BibTeXInfo const & entry = biit->second;
		refoutput += "[" + entry.label() + "] ";
		// FIXME Right now, we are calling BibInfo::getInfo on the key,
		// which will give us all the cross-referenced info. But for every
		// entry, so there's a lot of repitition. This should be fixed.
		refoutput += bibinfo.getInfo(entry.key(), buffer(), ci) + "\n\n";
	}
	os << refoutput;
	return refoutput.size();
}


// FIXME
// docstring InsetBibtex::entriesAsXHTML(vector<docstring> const & entries)
// And then here just: entriesAsXHTML(buffer().masterBibInfo().citedEntries())
docstring InsetBibtex::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	BiblioInfo const & bibinfo = buffer().masterBibInfo();
	bool const all_entries = getParam("btprint") == "btPrintAll";
	vector<docstring> const & cites =
	    all_entries ? bibinfo.getKeys() : bibinfo.citedEntries();

	docstring const reflabel = buffer().B_("References");

	// tell BiblioInfo our purpose
	CiteItem ci;
	ci.context = CiteItem::Export;
	ci.richtext = true;
	ci.max_key_size = UINT_MAX;

	xs << html::StartTag("h2", "class='bibtex'")
		<< reflabel
		<< html::EndTag("h2")
		<< html::StartTag("div", "class='bibtex'");

	// Now we loop over the entries
	vector<docstring>::const_iterator vit = cites.begin();
	vector<docstring>::const_iterator const ven = cites.end();
	for (; vit != ven; ++vit) {
		BiblioInfo::const_iterator const biit = bibinfo.find(*vit);
		if (biit == bibinfo.end())
			continue;

		BibTeXInfo const & entry = biit->second;
		string const attr = "class='bibtexentry' id='LyXCite-"
		    + to_utf8(html::cleanAttr(entry.key())) + "'";
		xs << html::StartTag("div", attr);

		// don't print labels if we're outputting all entries
		if (!all_entries) {
			xs << html::StartTag("span", "class='bibtexlabel'")
				<< entry.label()
				<< html::EndTag("span");
		}

		// FIXME Right now, we are calling BibInfo::getInfo on the key,
		// which will give us all the cross-referenced info. But for every
		// entry, so there's a lot of repitition. This should be fixed.
		xs << html::StartTag("span", "class='bibtexinfo'")
		   << XHTMLStream::ESCAPE_AND
		   << bibinfo.getInfo(entry.key(), buffer(), ci)
		   << html::EndTag("span")
		   << html::EndTag("div")
		   << html::CR();
	}
	xs << html::EndTag("div");
	return docstring();
}


void InsetBibtex::write(ostream & os) const
{
	params().Write(os, &buffer());
}


string InsetBibtex::contextMenuName() const
{
	return "context-bibtex";
}


} // namespace lyx
