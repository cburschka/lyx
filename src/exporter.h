// -*- C++ -*-
/**
 * \file exporter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXPORTER_H
#define EXPORTER_H

#include <map>
#include <string>
#include <vector>


class Buffer;
class Format;

class Exporter {
public:
	///
	static
	bool Export(Buffer * buffer, std::string const & format,
		    bool put_in_tempdir, std::string & result_file);
	///
	static
	bool Export(Buffer * buffer, std::string const & format,
		    bool put_in_tempdir);
	///
	static
	bool Preview(Buffer * buffer, std::string const & format);
	///
	static
	bool IsExportable(Buffer const & buffer, std::string const & format);
	///
	static
	std::vector<Format const *> const
	GetExportableFormats(Buffer const & buffer, bool only_viewable);
	///
};


struct ExportedFile {
	ExportedFile(std::string const &, std::string const &);
	/// absolute name of the source file
	std::string sourceName;
	/// final name that the exported file should get (absolute name or
	/// relative to the directory of the master document)
	std::string exportName;
};


bool operator==(ExportedFile const &, ExportedFile const &);


class ExportData {
public:
	/** add a referenced file for one format.
	 *  No inset should ever write any file outside the tempdir.
	 *  Instead, files that need to be exported have to be registered
	 *  with this method.
	 *  Then the exporter mechanism copies them to the right place, asks
	 *  for confirmation before overwriting existing files etc.
	 * \param format     format that references the given file
	 * \param sourceName source file name. Needs to be absolute
	 * \param exportName resulting file name. Can be either absolute
	 *                   or relative to the exported document.
	 */
	void addExternalFile(std::string const & format,
	                     std::string const & sourceName,
	                     std::string const & exportName);
	/** add a referenced file for one format.
	 *  The final name is the source file name without path.
	 * \param format     format that references the given file
	 * \param sourceName source file name. Needs to be absolute
	 */
	void addExternalFile(std::string const & format,
	                     std::string const & sourceName);
	/// get referenced files for \p format
	std::vector<ExportedFile> const
	externalFiles(std::string const & format) const;
private:
	typedef std::map<std::string, std::vector<ExportedFile> > FileMap;
	/** Files that are referenced by the export result in the
	 *  different formats.
	 */
	FileMap externalfiles;
};

#endif
