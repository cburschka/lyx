// -*- C++ -*-
/**
 * \file filetools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_FILETOOL_H
#define LYX_FILETOOL_H

#include <vector>
#include <utility>
#include <string>

namespace lyx {
namespace support {

/// remove directory and all contents, returns 0 on success
int destroyDir(std::string const & tmpdir);

/// Creates the per buffer temporary directory
std::string const createBufferTmpDir();

/// Creates directory. Returns true on success
bool createDirectory(std::string const & name, int permissions);

/** Creates the global LyX temp dir.
  \p deflt can be an existing directory name. In this case a new directory
  inside \p deflt is created. If \p deflt does not exist yet, \p deflt is
  created and used as the temporary directory.
  \return the tmp dir name or string() if something went wrong.
 */
std::string const createLyXTmpDir(std::string const & deflt);

/** Find file by searching several directories.
  Uses a string of paths separated by ";"s to find a file to open.
    Can't cope with pathnames with a ';' in them. Returns full path to file.
    If path entry begins with $$LyX/, use system_lyxdir.
    If path entry begins with $$User/, use user_lyxdir.
    Example: "$$User/doc;$$LyX/doc".
*/
std::string const FileOpenSearch(std::string const & path, std::string const & name,
		       std::string const & ext = std::string());

/** Returns the real name of file name in directory path, with optional
  extension ext.
  The file is searched in the given path (unless it is an absolute
  file name), first directly, and then with extension .ext (if given).
  */
std::string const FileSearch(std::string const & path, std::string const & name,
		  std::string const & ext = std::string());

/// Returns a vector of all files in directory dir having extension ext.
std::vector<std::string> const DirList(std::string const & dir,
				  std::string const & ext = std::string());

/** Is directory read only?
  returns
    true: dir writeable
    false: not writeable
*/
bool IsDirWriteable (std::string const & path);

/** Is a file readable ?
  Returns true if the file `path' is readable.
 */
bool IsFileReadable (std::string const & path);

/** Is file read only?
  returns
    1: read-write
    0: read_only
   -1: error (doesn't exist, no access, anything else)
  */
int IsFileWriteable (std::string const & path);

///
bool IsLyXFilename(std::string const & filename);

///
bool IsSGMLFilename(std::string const & filename);

/** Returns the path of a library data file.
  Search the file name.ext in the subdirectory dir of
  \begin{enumerate}
    \item user_lyxdir
    \item build_lyxdir (if not empty)
    \item system_lyxdir
  \end{enumerate}
    The third parameter `ext' is optional.
*/
std::string const LibFileSearch(std::string const & dir, std::string const & name,
		     std::string const & ext = std::string());

/** Same as LibFileSearch(), but tries first to find an
  internationalized version of the file by prepending $LANG_ to the
  name
  */
std::string const
i18nLibFileSearch(std::string const & dir, std::string const & name,
		  std::string const & ext = std::string());

/** Takes a command such as "sh $$s/convertDefault.sh file.in file.out"
 *  and replaces "$$s/" with the path to the "most important" of LyX's
 *  script directories containing this script. If the script is not found,
 *  "$$s/" is removed. Executing the command will still fail, but the
 *  error message will make some sort of sense ;-)
 */
std::string const LibScriptSearch(std::string const & command);

///
std::string const GetEnv(std::string const & envname);

/// A helper function.
std::string const GetEnvPath(std::string const & name);

/// Substitutes active latex characters with underscores in filename
std::string const MakeLatexName(std::string const & file);

/// Put the name in quotes suitable for the current shell
std::string const QuoteName(std::string const & file);

/// Add a filename to a path. Any path from filename is stripped first.
std::string const AddName(std::string const & path, std::string const & fname);

/// Append sub-directory(ies) to path in an intelligent way
std::string const AddPath(std::string const & path, std::string const & path2);

/** Change extension of oldname to extension.
 If oldname does not have an extension, it is appended.
 If the extension is empty, any extension is removed from the name.
 */
std::string const
ChangeExtension(std::string const & oldname, std::string const & extension);

/// Return the extension of the file (not including the .)
std::string const GetExtension(std::string const & name);

/// Return the type of the file as an extension from contents
std::string const getExtFromContents(std::string const & name);

/// check for zipped file
bool zippedFile(std::string const & name);

/** \return the name that LyX will give to the unzipped file \p zipped_file
  if the second argument of unzipFile() is empty.
 */
std::string const unzippedFileName(std::string const & zipped_file);

/** Unzip \p zipped_file.
  The unzipped file is named \p unzipped_file if \p unzipped_file is not
  empty, and unzippedFileName(\p zipped_file) otherwise.
  Will overwrite an already existing unzipped file without warning.
 */
std::string const unzipFile(std::string const & zipped_file,
                            std::string const & unzipped_file = std::string());

/// Returns true is path is absolute
bool AbsolutePath(std::string const & path);

/// Create absolute path. If impossible, don't do anything
std::string const ExpandPath(std::string const & path);

/** Convert relative path into absolute path based on a basepath.
  If relpath is absolute, just use that.
  If basepath doesn't exist use CWD.
  */
std::string const MakeAbsPath(std::string const & RelPath = std::string(),
			 std::string const & BasePath = std::string());

/** Creates a nice compact path for displaying. The parameter
  threshold, if given, specifies the maximal length of the path.
  */
std::string const
MakeDisplayPath(std::string const & path, unsigned int threshold = 1000);

/** Makes relative path out of absolute path.
  If it is deeper than basepath,
  it's easy. If basepath and abspath share something (they are all deeper
  than some directory), it'll be rendered using ..'s. If they are completely
  different, then the absolute path will be used as relative path
  WARNING: the absolute path and base path must really be absolute paths!!!
  */
std::string const
MakeRelPath(std::string const & abspath, std::string const & basepath);

/// Strip filename from path name
std::string const OnlyPath(std::string const & fname);

/** Normalize a path. Constracts path/../path
 *  Also converts paths like /foo//bar ==> /foo/bar
 */
std::string const NormalizePath(std::string const & path);

/// Strips path from filename
std::string const OnlyFilename(std::string const & fname);

/// Get the contents of a file as a huge std::string
std::string const GetFileContents(std::string const & fname);

/** Check and Replace Environmentvariables ${NAME} in Path.
    Replaces all occurences of these, if they are found in the
    environment.
    Variables are defined by Var := '${' [a-zA-Z_][a-zA-Z_0-9]* '}'
*/
std::string const ReplaceEnvironmentPath(std::string const & path);

/* Set \c link to the path \c file points to as a symbolic link.
   If \c resolve is true, then \c link is an absolute path
   Returns true if successful */
bool LyXReadLink(std::string const & file, std::string & link, bool resolve = false);

/// Uses kpsewhich to find tex files
std::string const findtexfile(std::string const & fil, std::string const & format);

/// remove the autosave-file and give a Message if it can't be done
void removeAutosaveFile(std::string const & filename);

/// read the BoundingBox entry from a ps/eps/pdf-file
std::string const readBB_from_PSFile(std::string const & file);

/** \param file1, file2 the two files to be compared. Must have absolute paths.
 *  \returns 1 if \c file1 has a more recent timestamp than \c file2,
 *           0 if their timestamps are the same,
 *          -1 if \c file2 has a more recent timestamp than \c file1.
 *  If one of the files does not exist, the return value indicates the file
 *  which does exist. Eg, if \c file1 exists but \c file2 does not, return 1.
 */
int compare_timestamps(std::string const & file1, std::string const & file2);

typedef std::pair<int, std::string> cmd_ret;

cmd_ret const RunCommand(std::string const & cmd);

} // namespace support
} // namespace lyx

#endif
