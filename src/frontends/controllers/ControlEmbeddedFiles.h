// -*- C++ -*-
/**
 * \file ControlEmbeddedFiles.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLEMBEDDEDFILES_H
#define CONTROLEMBEDDEDFILES_H

#include "Dialog.h"
#include "EmbeddedFiles.h"

namespace lyx {

namespace frontend {

class ControlEmbeddedFiles : public Dialog::Controller {
public:
	///
	ControlEmbeddedFiles(Dialog &);
	///
	virtual ~ControlEmbeddedFiles() {}
	///
	EmbeddedFiles const & embeddedFiles() const { return *embedded_files; }
	EmbeddedFiles & embeddedFiles() { return *embedded_files; }
	///
	virtual bool initialiseParams(std::string const &);
	/// obtain embedded files from buffer
	void updateEmbeddedFiles();
	///
	virtual void clearParams() {};
	///
	virtual bool isBufferDependent() const { return true; }
	///
	bool canApply() const { return true; }
	///
	virtual bool canApplyToReadOnly() const { return true; }
	///
	void setMessage(std::string const & msg) { message_ = msg; }
	///
	void dispatchParams();
	///
	void goTo(EmbeddedFile const & item);
	///
	void view(EmbeddedFile const & item);

protected:
	// directly handle buffer embedded files
	EmbeddedFiles * embedded_files;
	//
	std::string message_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLEMBEDDEDFILES_H
