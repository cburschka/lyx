// -*- C++ -*-
/**
 * \file InsetCommandParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOMMANDPARAMS_H
#define INSETCOMMANDPARAMS_H

#include "InsetCode.h"
#include "support/docstring.h"

#include <iosfwd>
#include <vector>


namespace lyx {

class Lexer;

class InsetCommandParams {
public:
	/// Construct parameters for inset of type \p code.
	explicit InsetCommandParams(InsetCode code);
	/// Construct parameters for inset of type \p code with
	/// command name \p cmdName.
	explicit InsetCommandParams(InsetCode code,
			std::string const & cmdName);
	///
	std::string insetType() const { return insetName(insetCode_); }
	///
	InsetCode code() const { return insetCode_; }
	///
	void read(Lexer &);
	/// Parse the command
	/// FIXME remove
	void scanCommand(std::string const &);
	///
	void write(std::ostream &) const;
	/// Build the complete LaTeX command
	docstring const getCommand() const;
	/// Return the command name
	std::string const & getCmdName() const { return cmdName_; }
	/// this is used by listings package.
	std::string const getOptions() const;
	/// FIXME remove
	std::string const getContents() const;
	/// Set the name to \p n. This must be a known name. All parameters
	/// are cleared except those that exist also in the new command.
	/// What matters here is the parameter name, not position.
	void setCmdName(std::string const & n);
	/// this is used by the listings package
	void setOptions(std::string const &);
	/// FIXME remove
	void setContents(std::string const &);
	/// get parameter \p name
	docstring const & operator[](std::string const & name) const;
	/// set parameter \p name
	docstring & operator[](std::string const & name);
	///
	bool preview() const { return preview_; }
	///
	void preview(bool p) { preview_ = p; }
	/// Clear the values of all parameters
	void clear();

private:
	/// FIXME remove
	std::string const getSecOptions() const;
	/// FIXME remove
	void setSecOptions(std::string const &);
	///
	struct CommandInfo {
		/// Number of parameters
		size_t n;
		/// Parameter names. paramnames[n] must be "".
		char const * const * paramnames;
		/// Tells whether a parameter is optional
		bool const * optional;
	};
	/// Get information for inset type \p code.
	/// Returns 0 if the inset is not known.
	static CommandInfo const * findInfo(InsetCode code);
	/// Get information for \p code and command \p cmdName.
	/// Returns 0 if the combination is not known.
	/// Don't call this without first making sure the command name is
	/// acceptable to the inset.
	static CommandInfo const * findInfo(InsetCode code,
	                                    std::string const & cmdName);
	///
	std::string getDefaultCmd(InsetCode);
	/// Description of all command properties
	CommandInfo const * info_;
	/// what kind of inset we're the parameters for
	InsetCode insetCode_;
	/// The name of this command as it appears in .lyx and .tex files
	std::string cmdName_;
	///
	typedef std::vector<docstring> ParamVector;
	/// The parameters (both optional and required ones). The order is
	/// the same that is required for LaTeX output. The size of params_
	/// is always info_->n.
	ParamVector params_;
	///
	bool preview_;
	///
	friend bool operator==(InsetCommandParams const &,
			InsetCommandParams const &);
};


///
bool operator==(InsetCommandParams const &, InsetCommandParams const &);

///
bool operator!=(InsetCommandParams const &, InsetCommandParams const &);


} // namespace lyx

#endif
