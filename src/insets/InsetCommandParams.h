// -*- C++ -*-
/**
 * \file InsetCommandParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Georg Baum
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOMMANDPARAMS_H
#define INSETCOMMANDPARAMS_H

#include "InsetCode.h"

#include "OutputParams.h"

#include "support/docstring.h"

#include <string>
#include <vector>
#include <map>


namespace lyx {

class Lexer;
class Buffer;

class ParamInfo {
public:
	///
	ParamInfo() {}
	/// Types of parameters
	enum ParamType {
		LATEX_OPTIONAL,    /// normal optional argument
		LATEX_REQUIRED,    /// normal required argument
		LYX_INTERNAL       /// a parameter used internally by LyX
	};
	/// Special handling on output
	enum ParamHandling {
		HANDLING_NONE,    /// no special handling
		HANDLING_ESCAPE,  /// escape special characters
		HANDLING_LATEXIFY /// transform special characters to LaTeX macros
	};
	///
	class ParamData {
	// No parameter may be named "preview", because that is a required
	// flag for all commands.
	public:
		///
		ParamData(std::string const &, ParamType, ParamHandling = HANDLING_NONE);
		///
		std::string name() const { return name_; }
		///
		ParamType type() const { return type_; }
		///
		ParamHandling handling() const { return handling_; }
		/// whether this is an optional LaTeX argument
		bool isOptional() const;
		///
		bool operator==(ParamData const &) const;
		/// 
		bool operator!=(ParamData const & rhs) const
			{ return !(*this == rhs); }
	private:
		///
		std::string name_;
		///
		ParamType type_;
		/// do we need special handling on latex output?
		ParamHandling handling_;
	};

	/// adds a new parameter
	void add(std::string const & name, ParamType type,
		 ParamHandling = HANDLING_NONE);
	///
	bool empty() const { return info_.empty(); }
	///
	size_t size() const { return info_.size(); }
	///
	typedef std::vector<ParamData>::const_iterator const_iterator;
	///
	const_iterator const begin() const { return info_.begin(); }
	///
	const_iterator const end() const { return info_.end(); }
	/// \return true if name corresponds to a parameter of some sort.
	/// \return false if the parameter does not exist at all of it it 
	bool hasParam(std::string const & name) const;
	///
	ParamData const & operator[](std::string const & name) const;
	///
	bool operator==(ParamInfo const &) const;
private:
	///
	std::vector<ParamData> info_;
};


class InsetCommandParams {
public:
	/// Construct parameters for inset of type \p code.
	explicit InsetCommandParams(InsetCode code);
	/// Construct parameters for inset of type \p code with
	/// command name \p cmdName.
	explicit InsetCommandParams(InsetCode code,
			std::string const & cmdName);
	///
	std::string insetType() const;
	///
	InsetCode code() const { return insetCode_; }
	/// Parse the command
	void read(Lexer &);
	///
	void Read(Lexer &, Buffer const *);
	///
	void write(std::ostream &) const;
	///
	void Write(std::ostream & os, Buffer const * buf) const;
	/// Build the complete LaTeX command
	docstring getCommand(OutputParams const &) const;
	/// Return the command name
	std::string const & getCmdName() const { return cmdName_; }
	/// Set the name to \p n. This must be a known name. All parameters
	/// are cleared except those that exist also in the new command.
	/// What matters here is the parameter name, not position.
	void setCmdName(std::string const & n);
	/// FIXME Would be better removed, but is used in BufferView.cpp in 
	/// ways that make removal hard.
	docstring getFirstNonOptParam() const;
	/// get parameter \p name
	/// LyX will assert if name is not a valid parameter.
	docstring const & operator[](std::string const & name) const;
	/// set parameter \p name
	/// LyX will assert if name is not a valid parameter.
	docstring & operator[](std::string const & name);
	///
	bool preview() const { return preview_; }
	///
	void preview(bool p) { preview_ = p; }
	/// Clear the values of all parameters
	void clear();
	///
	static bool isCompatibleCommand(InsetCode code, std::string const & s);
	/// 
	ParamInfo const & info() const { return info_; }
	///
	docstring prepareCommand(OutputParams const & runparams,
	        docstring const & command, ParamInfo::ParamHandling handling) const;
private:
	std::string getDefaultCmd(InsetCode code);
	/// checks whether we need to write an empty optional parameter
	/// \return true if a non-empty optional parameter follows ci
	bool writeEmptyOptional(ParamInfo::const_iterator ci) const;

	/// Description of all command properties
	ParamInfo info_;
	/// what kind of inset we're the parameters for
	InsetCode insetCode_;
	/// The name of this command as it appears in .lyx and .tex files
	std::string cmdName_;
	///
	// if we need to allow more than one value for a parameter, this
	// could be made a multimap. it may be that the only thing that
	// would then need changing is operator[].
	typedef std::map<std::string, docstring> ParamMap;
	/// The parameters, by name.
	ParamMap params_;
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
