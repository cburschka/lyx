/**
 * \file Exporter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Exporter.h"

#include "Mover.h"

#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Package.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;

/// ask the user what to do if a file already exists
static int checkOverwrite(FileName const & filename)
{
	if (!filename.exists())
		return 0;
	docstring text = bformat(_("The file %1$s already exists.\n\n"
							 "Do you want to overwrite that file?"),
						makeDisplayPath(filename.absFilename()));
	return Alert::prompt(_("Overwrite file?"),
					 text, 0, 2,
					 _("&Overwrite"), _("Overwrite &all"),
					 _("&Cancel export"));
}


/** copy file \p sourceFile to \p destFile. If \p force is false, the user
 *  will be asked before existing files are overwritten.
 *  \return
 *  - SUCCESS if this file got copied
 *  - FORCE   if subsequent calls should not ask for confirmation before
 *            overwriting files anymore.
 *  - CANCEL  if the export should be cancelled
 */
CopyStatus copyFile(string const & format,
		    FileName const & sourceFile, FileName const & destFile,
		    string const & latexFile, bool force)
{
	CopyStatus ret = force ? FORCE : SUCCESS;

	// Only copy files that are in our tmp dir, all other files would
	// overwrite themselves. This check could be changed to
	// boost::filesystem::equivalent(sourceFile, destFile) if export to
	// other directories than the document directory is desired.
	if (!prefixIs(onlyPath(sourceFile.absFilename()), package().temp_dir().absFilename()))
		return ret;

	if (!force) {
		switch(checkOverwrite(destFile)) {
		case 0:
			ret = SUCCESS;
			break;
		case 1:
			ret = FORCE;
			break;
		default:
			return CANCEL;
		}
	}

	Mover const & mover = getMover(format);
	if (!mover.copy(sourceFile, destFile, latexFile))
		Alert::error(_("Couldn't copy file"),
			     bformat(_("Copying %1$s to %2$s failed."),
				     makeDisplayPath(sourceFile.absFilename()),
				     makeDisplayPath(destFile.absFilename())));

	return ret;
}


ExportedFile::ExportedFile(FileName const & s, string const & e)
	: sourceName(s), exportName(e)
{}


bool operator==(ExportedFile const & f1, ExportedFile const & f2)
{
	return f1.sourceName == f2.sourceName &&
	       f1.exportName == f2.exportName;

}


void ExportData::addExternalFile(string const & format,
				 FileName const & sourceName,
				 string const & exportName)
{
	// Make sure that we have every file only once, otherwise copyFile()
	// would ask several times if it should overwrite a file.
	vector<ExportedFile> & files = externalfiles[format];
	ExportedFile file(sourceName, exportName);
	if (find(files.begin(), files.end(), file) == files.end())
		files.push_back(file);
}


void ExportData::addExternalFile(string const & format,
				 FileName const & sourceName)
{
	addExternalFile(format, sourceName, onlyFilename(sourceName.absFilename()));
}


vector<ExportedFile> const
ExportData::externalFiles(string const & format) const
{
	FileMap::const_iterator cit = externalfiles.find(format);
	if (cit != externalfiles.end())
		return cit->second;
	return vector<ExportedFile>();
}


} // namespace lyx
