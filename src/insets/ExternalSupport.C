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
#include "insetexternal.h"

#include "buffer.h"
#include "converter.h"
#include "debug.h"
#include "ExternalTemplate.h"

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


namespace lyx {
namespace external {

Template const * getTemplatePtr(InsetExternalParams const & params)
{
	TemplateManager const & etm = TemplateManager::get();
	return etm.getTemplateByName(params.templatename());
}


void editExternal(InsetExternalParams const & params, Buffer const & buffer)
{
	Template const * const et_ptr = getTemplatePtr(params);
	if (!et_ptr)
		return;
	Template const & et = *et_ptr;

	if (et.editCommand.empty())
		return;

	string const command = doSubstitution(params, buffer, et.editCommand);

	support::Path p(buffer.filePath());
	support::Forkedcall call;
	if (lyxerr.debugging(Debug::EXTERNAL)) {
		lyxerr << "Executing '" << command << "' in '"
		       << buffer.filePath() << '\'' << endl;
	}
	call.startscript(support::Forkedcall::DontWait, command);
}


namespace {

string const doSubstitution(InsetExternalParams const & params,
			    Buffer const & buffer, string const & s,
			    string const & filename)
{
	string result;
	string const basename = support::ChangeExtension(filename, string());
	string const filepath = support::OnlyPath(filename);

	result = support::subst(s, "$$FName", filename);
	result = support::subst(result, "$$Basename", basename);
	result = support::subst(result, "$$FPath", filepath);
	result = support::subst(result, "$$Tempname", params.tempname());
	result = support::subst(result, "$$Sysdir", support::system_lyxdir());

	// Handle the $$Contents(filename) syntax
	if (support::contains(result, "$$Contents(\"")) {

		string::size_type const pos = result.find("$$Contents(\"");
		string::size_type const end = result.find("\")", pos);
		string const file = result.substr(pos + 12, end - (pos + 12));
		string contents;

		string const filepath = support::IsFileReadable(file) ?
			buffer.filePath() : buffer.temppath();
		support::Path p(filepath);

		if (support::IsFileReadable(file))
			contents = support::GetFileContents(file);

		result = support::subst(result,
					("$$Contents(\"" + file + "\")").c_str(),
					contents);
	}

	return result;
}

/** update the file represented by the template.
    If \param external_in_tmpdir == true, then the generated file is
    place in the buffer's temporary directory.
*/
void updateExternal(InsetExternalParams const & params,
		    string const & format,
		    Buffer const & buffer,
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

	string from_file = params.filename.absFilename();

	if (from_format == "*") {
		if (from_file.empty())
			return; // NOT_NEEDED

		// Try and ascertain the file format from its contents.
		from_format = support::getExtFromContents(from_file);
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

	if (external_in_tmpdir && !from_file.empty()) {
		// We are running stuff through LaTeX
		string const temp_file =
			support::MakeAbsPath(params.filename.mangledFilename(),
					     buffer.temppath());
		unsigned long const from_checksum = support::sum(from_file);
		unsigned long const temp_checksum = support::sum(temp_file);

		if (from_checksum != temp_checksum) {
			if (!support::copy(from_file, temp_file))
				return; // FAILURE
		}

		from_file = temp_file;
	}

	string const to_file = doSubstitution(params, buffer,
					      outputFormat.updateResult,
					      from_file);

	string const abs_to_file =
		support::MakeAbsPath(to_file, buffer.filePath());

	// Do we need to perform the conversion?
	// Yes if to_file does not exist or if from_file is newer than to_file
	if (support::compare_timestamps(from_file, abs_to_file) < 0)
		return; // SUCCESS

	string const to_file_base =
		support::ChangeExtension(to_file, string());
	/* bool const success = */
		converters.convert(&buffer, from_file, to_file_base,
				   from_format, to_format);
	// return success
}

} // namespace anon


int writeExternal(InsetExternalParams const & params,
		  string const & format,
		  Buffer const & buffer, ostream & os,
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

	updateExternal(params, format, buffer, external_in_tmpdir);

	string from_file = params.filename.outputFilename(buffer.filePath());
	if (external_in_tmpdir && !from_file.empty()) {
		// We are running stuff through LaTeX
		from_file =
			support::MakeAbsPath(params.filename.mangledFilename(),
					     buffer.temppath());
	}
	
	string const str = doSubstitution(params, buffer, cit->second.product,
					  from_file);
	os << str;
	return int(lyx::count(str.begin(), str.end(),'\n') + 1);
}


/// Substitute meta-variables in this string
string const doSubstitution(InsetExternalParams const & params,
			    Buffer const & buffer, string const & s)
{
	string const buffer_path = buffer.filePath();
	string const filename = params.filename.outputFilename(buffer_path);
	return doSubstitution(params, buffer, s, filename);
}

} // namespace external
} // namespace lyx
