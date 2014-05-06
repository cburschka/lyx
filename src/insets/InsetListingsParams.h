// -*- C++ -*-
/**
 * \file InsetListingsParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETLISTINGSPARAMS_H
#define INSETLISTINGSPARAMS_H

#include "InsetCollapsable.h"

#include <map>

namespace lyx {

class InsetListingsParams {
public:
	///
	InsetListingsParams();

	///
	InsetListingsParams(std::string const &, bool in=false,
		InsetCollapsable::CollapseStatus s = InsetCollapsable::Open);

	/// write parameters to an ostream
	void write(std::ostream &) const;

	/// read parameters from an ostream
	void read(Lexer &);

	/// valid parameter string
	std::string params(std::string const & sep=",") const;

	/// add key=value to params_. key_=value will be used if key=value already exists
	/// unless replace=true.
	void addParam(std::string const & key, std::string const & value, 
			bool replace = false);

	/// add a few parameters
	void addParams(std::string const & par);

	/// set params_ with par, throw an exception if par is valid
	void setParams(std::string const & par);

	/// generate a parameter string that can be safely save and restored
	/// by lyx' lexer
	std::string encodedString() const;

	/// newline (\n) separated parameters. comma can be removed.
	/// One possible complication is that , may appear in option value.
	std::string separatedParams(bool keepComma = false) const;

	/// get parameter from encoded string
	void fromEncodedString(std::string const & par);

	///
	bool isInline() const { return inline_; }

	///
	bool isFloat() const;

	///
	InsetCollapsable::CollapseStatus status() const { return status_; }

	///
	void setInline(bool i) { inline_ = i; }

	/// get value of option \c param
	std::string getParamValue(std::string const & param) const;

	///
	void clear() { params_.clear(); }
	
	/// validate parameter, return an error message
	docstring validate() const;

private:
	/// inline or normal listings
	bool inline_;

	/// Do we have a param with the given \c key?
	bool hasParam(std::string const & key) const;
	/// return the value for the given \c key, if avaible, else empty string
	std::string getValue(std::string const & key) const;

	/// key-value pairs of the parameters
	// Use a vector of pairs in order to maintain the order of insertion.
	typedef std::vector<std::pair<std::string, std::string> > keyValuePair;
	keyValuePair params_;

	/// collapsable status
	InsetCollapsable::CollapseStatus status_;
};


} // namespace lyx

#endif
