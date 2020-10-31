// -*- C++ -*-
/**
 * \file GuiCompare.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICOMPARE_H
#define GUICOMPARE_H

#include "GuiDialog.h"

#include "ui_CompareUi.h"

namespace lyx {

class Compare;

namespace frontend {


class GuiCompare : public GuiDialog, public Ui::CompareUi
{
	Q_OBJECT

public:
	///
	GuiCompare(GuiView & lv);
	~GuiCompare();

	void closeEvent(QCloseEvent *) override;

private Q_SLOTS:
	///
	void slotOK();
	///
	void slotCancel();
	///
	void slotButtonBox(QAbstractButton *);
	///
	void changeAdaptor();
	///
	void selectNewFile();
	///
	void selectOldFile();

	///
	void error();
	///
	void finished(bool aborted);
	///
	void progress(int);
	///
	void progressMax(int) const;
	///
	void setStatusMessage(QString const &);

private:
	///
	void updateContents() override;
	///
	bool isValid() override;
	///
	bool initialiseParams(std::string const &) override;
	///
	bool isBufferDependent() const override { return false; }
	///
	void clearParams() override {}
	///
	void dispatchParams() override {}
	///
	void apply() override {}


	/// enable or disable all controls and rename the Close/Cancel button
	void enableControls(bool enable);

	/// browse for a file
	QString browse(QString const & in_name) const;
	/// retrieve the buffer from the specified filename
	Buffer const * bufferFromFileName(std::string const & file) const;

	/// create the compare object and run the comparison
	int run();

private:
	/// the object that will do the comparison
	Compare * compare_;

	/// the buffer that will contain the result
	Buffer * dest_buffer_;
	/// the buffer that will contain the result
	Buffer const * old_buffer_;
	/// the buffer that will contain the result
	Buffer const * new_buffer_;
};



} // namespace frontend
} // namespace lyx

#endif // GUICOMPARE_H
