/**
 * \file ExternalSupport.C
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
#include "insetexternal.h"

#include "buffer.h"
#include "converter.h"
#include "debug.h"
#include "exporter.h"
#include "format.h"
#include "mover.h"

#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/lyxlib.h"
#include "support/path.h"
#include "support/path_defines.h"

#include "support/std_ostream.h"

namespace support = lyx::support;

using std::endl;

using std::ostream;
using std::string;
using std::vector;


namespace lyx {
namespace external {

Template const * getTemplatePtr(InsetExternalParams const & params)
{
	TemplateManager const & etm = TemplateManager::get();
	return etm.getTemplateByName(params.templatename());
}


void editExternal(InsetExternalParams const & params, Buffer const & buffer)
{
	string const file_with_path = params.filename.absFilename();
	formats.edit(buffer, file_with_path,
	             support::getFormatFromContents(file_with_path));
}


string const doSubstitution(InsetExternalParams const & params,
			    Buffer const & buffer, string const & s,
			    bool external_in_tmpdir)
{
	Buffer const * m_buffer = buffer.getMasterBuffer();
	string const parentpath = external_in_tmpdir ?
		m_buffer->temppath() :
		buffer.filePath();
	string const filename = external_in_tmpdir ?
		params.filename.mangledFilename() :
		params.filename.outputFilename(parentpath);
	string result;
	string const basename = support::ChangeExtension(
			support::OnlyFilename(filename), string());
	string const absname = support::MakeAbsPath(filename, parentpath);
	string const filepath = support::OnlyPath(filename);
	string const abspath = support::OnlyPath(absname);
	string const masterpath = external_in_tmpdir ?
		m_buffer->temppath() :
		m_buffer->filePath();
	string relToMasterPath = support::OnlyPath(
			support::MakeRelPath(absname, masterpath));
	if (relToMasterPath == "./")
		relToMasterPath.clear();
	string relToParentPath = support::OnlyPath(
			support::MakeRelPath(absname, parentpath));
	if (relToParentPath == "./")
		relToParentPath.clear();

	result = support::subst(s, "$$FName", filename);
	result = support::subst(result, "$$Basename", basename);
	result = support::subst(result, "$$Extension",
			'.' + support::GetExtension(filename));
	result = support::subst(result, "$$FPath", filepath);
	result = support::subst(result, "$$AbsPath", abspath);
	result = support::subst(result, "$$RelPathMaster", relToMasterPath);
	result = support::subst(result, "$$RelPathParent", relToParentPath);
	if (support::AbsolutePath(filename)) {
		result = support::subst(result, "$$AbsOrRelPathMaster",
		                        abspath);
		result = support::subst(result, "$$AbsOrRelPathParent",
		                        abspath);
	} else {
		result = support::subst(result, "$$AbsOrRelPathMaster",
		                        relToMasterPath);
		result = support::subst(result, "$$AbsOrRelPathParent",
		                        relToParentPath);
	}
	result = support::subst(result, "$$Tempname", params.tempname());
	result = support::subst(result, "$$Sysdir", support::system_lyxdir());

	// Handle the $$Contents(filename) syntax
	if (support::contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;

		string const filepath = support::IsFileReadable(file) ?
			buffer.filePath() : m_buffer->temppath();
		support::Path p(filepath);

		if (support::IsFileReadable(file))
			contents = support::GetFileContents(file);

		result = support::subst(result,
					("$$Contents(\"" + file + "\")").c_str(),
					contents);
	}

	return result;
}


namespace {

/** update the file represented by the template.
    If \param external_in_tmpdir == true, then the generated file is
    place in the buffer's temporary directory.
*/
void updateExternal(InsetExternalParams const & params,
		    string const & format,
		    Buffer const & buffer,
                    ExportData & exportdata,
		    bool external_in_tmpdir)
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

	string abs_from_file = params.filename.absFilename();

	if (from_format == "*") {
		if (abs_from_file.empty())
			return; // NOT_NEEDED

		// Try and ascertain the file format from its contents.
		from_format = support::getFormatFromContents(abs_from_file);
		if (from_format.empty())
			return; // FAILURE

	}

	string const to_format = outputFormat.updateFormat;
	if (to_format.empty())
		return; // NOT_NEEDED

	if (!converters.isReachable(from_format, to_format)) {
		lyxerr[Debug::EXTERNAL]
			<< "external::updateExternal. "
			<< "Unable to convert from "
			<< from_format << " to " << to_format << endl;
		return; // FAILURE
	}

	// The master buffer. This is useful when there are multiple levels
	// of include files
	Buffer const * m_buffer = buffer.getMasterBuffer();

	// We copy the source file to the temp dir and do the conversion
	// there if necessary
	string const temp_file =
		support::MakeAbsPath(params.filename.mangledFilename(),
				     m_buffer->temppath());
	if (!abs_from_file.empty()) {
		unsigned long const from_checksum = support::sum(abs_from_file);
		unsigned long const temp_checksum = support::sum(temp_file);

		if (from_checksum != temp_checksum) {
			Mover const & mover = movers(from_format);
			if (!mover.copy(abs_from_file, temp_file)) {
				lyxerr[Debug::EXTERNAL]
					<< "external::updateExternal. "
					<< "Unable to copy "
					<< abs_from_file << " to " << temp_file << endl;
				return; // FAILURE
			}
		}
	}

	// the generated file (always in the temp dir)
	string const to_file = doSubstitution(params, buffer,
					      outputFormat.updateResult,
	                                      true);
	string const abs_to_file =
		support::MakeAbsPath(to_file, m_buffer->temppath());

	// Record the referenced files for the exporter.
	// The exporter will copy them to the export dir.
	typedef Template::Format::FileMap FileMap;
	FileMap::const_iterator rit  = outputFormat.referencedFiles.begin();
	FileMap::const_iterator rend = outputFormat.referencedFiles.end();
	for (; rit != rend; ++rit) {
		vector<string>::const_iterator fit  = rit->second.begin();
		vector<string>::const_iterator fend = rit->second.end();
		for (; fit != fend; ++fit) {
			string const source = support::MakeAbsPath(
					doSubstitution(params, buffer, *fit,
					               true),
					m_buffer->temppath());
			string const file = doSubstitution(params, buffer,
			                                   *fit,
			                                   external_in_tmpdir);
			// if file is a relative name, it is interpreted
			// relative to the master document.
			exportdata.addExternalFile(rit->first, source, file);
		}
	}

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if from_file is newer than to_file
	if (support::compare_timestamps(temp_file, abs_to_file) < 0)
		return; // SUCCESS
	string const to_file_base =
		support::ChangeExtension(to_file, string());
	/* bool const success = */
		converters.convert(&buffer, temp_file, to_file_base,
				   from_format, to_format);
	// return success
}


string const substituteCommands(InsetExternalParams const & params,
				string const & input, string const & format);

string const substituteOptions(InsetExternalParams const & params,
			       string const & input, string const & format);

} // namespace anon


int writeExternal(InsetExternalParams const & params,
		  string const & format,
		  Buffer const & buffer, ostream & os,
                  ExportData & exportdata,
		  bool external_in_tmpdir)
{
	Template const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return 0;
	Template const & et = *et_ptr;

	Template::Formats::const_iterator cit = et.formats.find(format);
	if (cit == et.formats.end()) {
		lyxerr[Debug::EXTERNAL]
			<< "External template format '" << format
			<< "' not specified in template "
			<< params.templatename() << endl;
		return 0;
	}

	updateExternal(params, format, buffer, exportdata, external_in_tmpdir);

	string str = doSubstitution(params, buffer, cit->second.product,
	                            external_in_tmpdir);
	str = substituteCommands(params, str, format);
	str = substituteOptions(params, str, format);
	os << str;
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
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
	typedef std::map<TransformID, TransformStore> Transformers;
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
		support::subst(input, ptr->front_placeholder(), ptr->front());
	return support::subst(result, ptr->back_placeholder(),  ptr->back());
}


template <>
string const substituteIt<TransformOption>(string const & input,
					   TransformID id,
					   string const & fname,
					   Template::Format const & format,
					   InsetExternalParams const & params)
{
	typedef std::map<TransformID, TransformStore> Transformers;
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

	return support::subst(input, ptr->placeholder(), ptr->option());
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
	if (format == "LinuxDoc")
		return sanitizeLinuxDocOption(opt);
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
		output = support::subst(output, placeholder, opt);
	}

	return output;
 }

} // namespace anon

} // namespace external
} // namespace lyx
