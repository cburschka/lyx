// -*- C++ -*-
/**
 * \file ExternalSupport.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef EXTERNALSUPPORT_H
#define EXTERNALSUPPORT_H

#include <iosfwd>
#include <string>

class Buffer;
class ExportData;
class InsetExternalParams;

namespace lyx {
namespace external {

class Template;

/// A shorthand, helper function
Template const * getTemplatePtr(InsetExternalParams const & params);


/// Invoke the external editor.
void editExternal(InsetExternalParams const & params,
		  Buffer const & buffer);


/** Substitute meta-variables in string \p s, making use of \p params and
    \p buffer.
    If \p external_in_tmpdir is true, all files are assumed to be in the
    master buffers temp path, and the mangled filename is used.
    Otherwise, the output filename (absolute or relative to the parent
    document, as written in the .lyx file) is used.
*/
std::string const doSubstitution(InsetExternalParams const & params,
				 Buffer const & buffer,
                                 std::string const & s,
                                 bool external_in_tmpdir = false);


/** Write the output for a specific file format
    and generate any external data files.
    If \param external_in_tmpdir == true, then the generated file is
    place in the buffer's temporary directory.
*/
int writeExternal(InsetExternalParams const &,
		  std::string const & format,
		  Buffer const &,
		  std::ostream &,
		  ExportData &,
		  bool external_in_tmpdir = false);

} // namespace external
} // namespace lyx

#endif // NOT EXTERNALSUPPORT_H
