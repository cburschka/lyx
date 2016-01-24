/**
 * \file ExternalSupport.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ExternalSupport.h"
#include "ExternalTemplate.h"
#include "ExternalTransforms.h"
#include "InsetExternal.h"

#include "Buffer.h"
#include "Converter.h"
#include "ErrorList.h"
#include "Exporter.h"
#include "Format.h"
#include "Mover.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace external {

Template const * getTemplatePtr(InsetExternalParams const & params)
{
	TemplateManager const & etm = TemplateManager::get();
	return etm.getTemplateByName(params.templatename());
}


void editExternal(InsetExternalParams const & params, Buffer const & buffer)
{
	formats.edit(buffer, params.filename, 
		formats.getFormatFromFile(params.filename));
}


namespace {

string const subst_path(string const & input,
			string const & placeholder,
			string const & path,
			bool use_latex_path,
			latex_path_extension ext = PROTECT_EXTENSION,
			latex_path_dots dots = LEAVE_DOTS)
{
	if (input.find(placeholder) == string::npos)
		return input;
	// Don't use external_path here when use_latex_path is false, as the
	// path will be compared with another one in internal style later
	// in Converters::move.
	string const path2 = use_latex_path ?
		latex_path(path, ext, dots) : path;
	return subst(input, placeholder, path2);
}

} // namespace anon


string const doSubstitution(InsetExternalParams const & params,
			    Buffer const & buffer, string const & s,
			    bool use_latex_path,
			    bool external_in_tmpdir,
			    Substitute what)
{
	string result = s;
	if (what != PATHS && contains(result, "$$pngOrjpg")) {
		// This is for raster images and pdflatex:
		// Since pdflatex supports both jpg and png, we choose the best format:
		// jpg if the original file is jpg to retain the compression, else png.
		string format = formats.getFormatFromFile(params.filename);
		if (format == "jpg")
			result = subst(result, "$$pngOrjpg", "jpg");
		else
			result = subst(result, "$$pngOrjpg", "png");
	}

	if (what == FORMATS)
		return result;

	Buffer const * masterBuffer = buffer.masterBuffer();
	string const parentpath = external_in_tmpdir ?
		masterBuffer->temppath() :
		buffer.filePath();
	string const filename = external_in_tmpdir ?
		params.filename.mangledFileName() :
		params.filename.outputFileName(parentpath);
	string const basename = changeExtension(
			onlyFileName(filename), string());
	string const absname = makeAbsPath(filename, parentpath).absFileName();

	if (what != ALL_BUT_PATHS) {
		string const filepath = onlyPath(filename);
		string const abspath = onlyPath(absname);
		string const masterpath = external_in_tmpdir ?
			masterBuffer->temppath() :
			masterBuffer->filePath();
		// FIXME UNICODE
		string relToMasterPath = onlyPath(
				to_utf8(makeRelPath(from_utf8(absname),
							     from_utf8(masterpath))));
		if (relToMasterPath == "./")
			relToMasterPath.clear();
		// FIXME UNICODE
		string relToParentPath = onlyPath(
				to_utf8(makeRelPath(from_utf8(absname),
							     from_utf8(parentpath))));
		if (relToParentPath == "./")
			relToParentPath.clear();

		result = subst_path(result, "$$FPath", filepath,
				    use_latex_path,
				    PROTECT_EXTENSION,
				    ESCAPE_DOTS);
		result = subst_path(result, "$$AbsPath", abspath,
				    use_latex_path,
				    PROTECT_EXTENSION,
				    ESCAPE_DOTS);
		result = subst_path(result, "$$RelPathMaster",
				    relToMasterPath, use_latex_path,
				    PROTECT_EXTENSION,
				    ESCAPE_DOTS);
		result = subst_path(result, "$$RelPathParent",
				    relToParentPath, use_latex_path,
				    PROTECT_EXTENSION,
				    ESCAPE_DOTS);
		if (FileName::isAbsolute(filename)) {
			result = subst_path(result, "$$AbsOrRelPathMaster",
					    abspath, use_latex_path,
					    PROTECT_EXTENSION,
					    ESCAPE_DOTS);
			result = subst_path(result, "$$AbsOrRelPathParent",
					    abspath, use_latex_path,
					    PROTECT_EXTENSION,
					    ESCAPE_DOTS);
		} else {
			result = subst_path(result, "$$AbsOrRelPathMaster",
					    relToMasterPath, use_latex_path,
					    PROTECT_EXTENSION,
					    ESCAPE_DOTS);
			result = subst_path(result, "$$AbsOrRelPathParent",
					    relToParentPath, use_latex_path,
					    PROTECT_EXTENSION,
					    ESCAPE_DOTS);
		}
	}

	if (what == PATHS)
		return result;

	result = subst_path(result, "$$FName", filename, use_latex_path,
			    EXCLUDE_EXTENSION);
	result = subst_path(result, "$$Basename", basename, use_latex_path,
			    PROTECT_EXTENSION, ESCAPE_DOTS);
	result = subst_path(result, "$$Extension",
			'.' + getExtension(filename), use_latex_path);
	result = subst_path(result, "$$Tempname", params.tempname().absFileName(), use_latex_path);
	result = subst_path(result, "$$Sysdir",
				package().system_support().absFileName(), use_latex_path);

	// Handle the $$Contents(filename) syntax
	if (contains(result, "$$Contents(\"")) {
		// Since use_latex_path may be true we must extract the file
		// name from s instead of result and do the substitutions
		// again, this time with use_latex_path false.
		size_t const spos = s.find("$$Contents(\"");
		size_t const send = s.find("\")", spos);
		string const file_template = s.substr(spos + 12, send - (spos + 12));
		string const file = doSubstitution(params, buffer,
						   file_template, false,
						   external_in_tmpdir, what);
		string contents;

		FileName const absfile(
			makeAbsPath(file, masterBuffer->temppath()));
		if (absfile.isReadableFile())
			// FIXME UNICODE
			contents = to_utf8(absfile.fileContents("UTF-8"));

		size_t const pos = result.find("$$Contents(\"");
		size_t const end = result.find("\")", pos);
		result.replace(pos, end + 2, contents);
	}

	return result;
}


namespace {

/** update the file represented by the template.
    If \p external_in_tmpdir == true, then the generated file is
    placed in the buffer's temporary directory.
*/
void updateExternal(InsetExternalParams const & params,
		    string const & format,
		    Buffer const & buffer,
		    ExportData & exportdata,
		    bool external_in_tmpdir,
		    bool dryrun)
{
	Template const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return; // FAILURE
	Template const & et = *et_ptr;

	if (!et.automaticProduction)
		return; // NOT_NEEDED

	Template::Formats::const_iterator cit = et.formats.find(format);
	if (cit == et.formats.end())
		return; // FAILURE

	Template::Format const & outputFormat = cit->second;
	if (outputFormat.updateResult.empty())
		return; // NOT_NEEDED

	string from_format = et.inputFormat;
	if (from_format.empty())
		return; // NOT_NEEDED

	if (from_format == "*") {
		if (params.filename.empty())
			return; // NOT_NEEDED

		// Try and ascertain the file format from its contents.
		from_format = formats.getFormatFromFile(params.filename);
		if (from_format.empty())
			return; // FAILURE
	}

	string const to_format = doSubstitution(params, buffer,
		outputFormat.updateFormat, false, external_in_tmpdir, FORMATS);
	if (to_format.empty())
		return; // NOT_NEEDED

	// The master buffer. This is useful when there are multiple levels
	// of include files
	Buffer const * masterBuffer = buffer.masterBuffer();

	// We copy the source file to the temp dir and do the conversion
	// there if necessary
	bool const isDir = params.filename.isDirectory();
	FileName const temp_file(
		makeAbsPath(params.filename.mangledFileName(),
				     masterBuffer->temppath()));
	if (!params.filename.empty() && !isDir) {
		unsigned long const from_checksum = params.filename.checksum();
		unsigned long const temp_checksum = temp_file.checksum();

		if (from_checksum != temp_checksum) {
			Mover const & mover = getMover(from_format);
			if (!mover.copy(params.filename, temp_file)) {
				LYXERR(Debug::EXTERNAL, "external::updateExternal. "
					<< "Unable to copy " << params.filename << " to " << temp_file);
				return; // FAILURE
			}
		}
	}

	// the generated file (always in the temp dir)
	string const to_file = doSubstitution(params, buffer,
					      outputFormat.updateResult,
					      false, true);
	FileName const abs_to_file(
		makeAbsPath(to_file, masterBuffer->temppath()));

	if (!dryrun) {
		// Record the referenced files for the exporter.
		// The exporter will copy them to the export dir.
		typedef Template::Format::FileMap FileMap;
		FileMap::const_iterator rit  = outputFormat.referencedFiles.begin();
		FileMap::const_iterator rend = outputFormat.referencedFiles.end();
		for (; rit != rend; ++rit) {
			vector<string>::const_iterator fit  = rit->second.begin();
			vector<string>::const_iterator fend = rit->second.end();
			for (; fit != fend; ++fit) {
				FileName const source(makeAbsPath(
						doSubstitution(params, buffer, *fit,
							       false, true),
						masterBuffer->temppath()));
				// The path of the referenced file is never the
				// temp path, but the filename may be the mangled
				// or the real name. Therefore we substitute the
				// paths and names separately.
				string file = subst(*fit, "$$FName",
						"$$FPath$$Basename$$Extension");
				file = doSubstitution(params, buffer, file, false, false,
						      PATHS);
				file = doSubstitution(params, buffer, file,
						      false, external_in_tmpdir,
						      ALL_BUT_PATHS);
				// if file is a relative name, it is interpreted
				// relative to the master document.
				if (makeAbsPath(file, masterBuffer->filePath()) !=
					params.filename.absFileName())
						exportdata.addExternalFile(rit->first, source, file);
			}
		}
	}

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if from_file is newer than to_file
	// or if from_file is a directory (bug 9925)
	if (!isDir && compare_timestamps(temp_file, abs_to_file) < 0)
		return; // SUCCESS

	// FIXME (Abdel 12/08/06): Is there a need to show these errors?
	ErrorList el;
	bool const success =
		theConverters().convert(&buffer, temp_file, abs_to_file,
				   params.filename, from_format, to_format, el,
				   Converters::try_default | Converters::try_cache);

	if (!success) {
		LYXERR(Debug::EXTERNAL, "external::updateExternal. "
			<< "Unable to convert from " << from_format << " to " << to_format);
	}

	// return success
}


string const substituteCommands(InsetExternalParams const & params,
				string const & input, string const & format);

string const substituteOptions(InsetExternalParams const & params,
			       string const & input, string const & format);

} // namespace anon


void writeExternal(InsetExternalParams const & params,
		   string const & format,
		   Buffer const & buffer, otexstream & os,
		   ExportData & exportdata,
		   bool external_in_tmpdir,
		   bool dryrun)
{
	Template const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return;
	Template const & et = *et_ptr;

	Template::Formats::const_iterator cit = et.formats.find(format);
	if (cit == et.formats.end()) {
		LYXERR(Debug::EXTERNAL, "External template format '" << format
			<< "' not specified in template " << params.templatename());
		return;
	}

	if (!dryrun || contains(cit->second.product, "$$Contents"))
		updateExternal(params, format, buffer, exportdata,
			       external_in_tmpdir, dryrun);

	bool const use_latex_path = format == "LaTeX";
	string str = doSubstitution(params, buffer, cit->second.product,
				    use_latex_path, external_in_tmpdir);

	string const absname = makeAbsPath(
		params.filename.outputFileName(buffer.filePath()), buffer.filePath()).absFileName();

	if (!dryrun && !external_in_tmpdir) {
		if (!isValidLaTeXFileName(absname)) {
			lyx::frontend::Alert::warning(_("Invalid filename"),
				         _("The following filename will cause troubles "
					       "when running the exported file through LaTeX: ") +
					     from_utf8(absname));
		}
		if (!isValidDVIFileName(absname)) {
			lyx::frontend::Alert::warning(_("Problematic filename for DVI"),
				         _("The following filename can cause troubles "
					       "when running the exported file through LaTeX "
						   "and opening the resulting DVI: ") +
					     from_utf8(absname), true);
		}
	}

	str = substituteCommands(params, str, format);
	str = substituteOptions(params, str, format);
	// FIXME UNICODE
	os << from_utf8(str);
	return;
}

namespace {

// Empty template, specialised below.
template <typename TransformType>
string const substituteIt(string const &,
			  TransformID,
			  string const &,
			  Template::Format const &,
			  InsetExternalParams const &);


template <>
string const substituteIt<TransformCommand>(string const & input,
					    TransformID id,
					    string const & /* formatname */,
					    Template::Format const & format,
					    InsetExternalParams const & params)
{
	typedef map<TransformID, TransformStore> Transformers;
	Transformers::const_iterator it = format.command_transformers.find(id);
	if (it == format.command_transformers.end())
		return input;

	TransformStore const & store = it->second;

	TransformCommand::ptr_type ptr;
	if (id == Rotate)
		ptr = store.getCommandTransformer(params.rotationdata);
	else if (id == Resize)
		ptr = store.getCommandTransformer(params.resizedata);

	if (!ptr.get())
		return input;

	string result =
		subst(input, ptr->front_placeholder(), ptr->front());
	return subst(result, ptr->back_placeholder(),  ptr->back());
}


template <>
string const substituteIt<TransformOption>(string const & input,
					   TransformID id,
					   string const & fname,
					   Template::Format const & format,
					   InsetExternalParams const & params)
{
	typedef map<TransformID, TransformStore> Transformers;
	Transformers::const_iterator it = format.option_transformers.find(id);
	if (it == format.option_transformers.end())
		return input;

	TransformStore const & store = it->second;

	TransformOption::ptr_type ptr;
	switch (id) {
	case Clip:
		ptr = store.getOptionTransformer(params.clipdata);
		break;
	case Extra:
		ptr = store.getOptionTransformer(params.extradata.get(fname));
		break;
	case Rotate:
		ptr = store.getOptionTransformer(params.rotationdata);
		break;
	case Resize:
		ptr = store.getOptionTransformer(params.resizedata);
		break;
	}

	if (!ptr.get())
		return input;

	return subst(input, ptr->placeholder(), ptr->option());
}


template <typename TransformerType>
string const transformIt(InsetExternalParams const & params,
			 string const & s, string const & formatname)
{
	Template const * const et = getTemplatePtr(params);
	if (!et || et->transformIds.empty())
		return s;

	Template::Formats::const_iterator fit = et->formats.find(formatname);
	if (fit == et->formats.end())
		return s;

	string result = s;
	Template::Format const & format =  fit->second;

	typedef vector<TransformID> TransformsIDs;
	TransformsIDs::const_iterator it  = et->transformIds.begin();
	TransformsIDs::const_iterator end = et->transformIds.end();
	for (; it != end; ++it) {
		result = substituteIt<TransformerType>(result, *it, formatname,
						       format, params);
	}
	return result;
}


string const substituteCommands(InsetExternalParams const & params,
				string const & input, string const & format)
{
	return transformIt<TransformCommand>(params, input, format);
}


string const substituteOption(InsetExternalParams const & params,
			      string const & input, string const & format)
{
	string opt = transformIt<TransformOption>(params, input, format);

	if (format == "LaTeX" || format == "PDFLaTeX")
		return sanitizeLatexOption(opt);
	if (format == "DocBook")
		return sanitizeDocBookOption(opt);
	return opt;
}


string const substituteOptions(InsetExternalParams const & params,
			       string const & input, string const & format)
{
	string output = input;

	Template const * const et = getTemplatePtr(params);
	if (!et || et->transformIds.empty())
		return output;

	Template::Formats::const_iterator fit = et->formats.find(format);
	if (fit == et->formats.end() || fit->second.options.empty())
		return output;

	typedef vector<Template::Option> Options;
	Options const & options = fit->second.options;
	Options::const_iterator it  = options.begin();
	Options::const_iterator end = options.end();
	for (; it != end; ++it) {
		string const opt = substituteOption(params, it->option, format);
		string const placeholder = "$$" + it->name;
		output = subst(output, placeholder, opt);
	}

	return output;
}

} // namespace anon

} // namespace external

} // namespace lyx
