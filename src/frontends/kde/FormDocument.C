/**
 * \file FormDocument.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "docdlg.h"

#include "Dialogs.h"
#include "FormDocument.h"

#include "CutAndPaste.h" 
#include "buffer.h"
#include "Liason.h"

#include "QtLyXView.h"
#include "debug.h"

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

FormDocument::FormDocument(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	d->showLayoutDocument.connect(slot(this, &FormDocument::show));
}


FormDocument::~FormDocument()
{
	delete dialog_;
}


void FormDocument::update(bool)
{
	if (!lv_->view()->available())
		return;

	Buffer *buf = lv_->buffer();

	if (readonly!=buf->isReadonly()) {
		readonly = buf->isReadonly();
		dialog_->setReadOnly(readonly);
	}

	dialog_->setFromParams(buf->params);
}


void FormDocument::apply()
{
	if (readonly)
		return;
	
	if (!lv_->view()->available())
		return;

	BufferParams & params = lv_->buffer()->params;

	if (dialog_->updateParams(params))
		lv_->view()->redoCurrentBuffer();

	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Document layout set"));
}
 

void FormDocument::show()
{
	if (!dialog_)
		dialog_ = new DocDialog(this, 0, _("LyX: Document Options"), false);

	if (!dialog_->isVisible())
		h_ = d_->hideBufferDependent.connect(slot(this, &FormDocument::hide));


	dialog_->raise();
	dialog_->setActiveWindow();
	update();
 
	dialog_->show();
}


bool FormDocument::changeClass(BufferParams & params, int new_class)
{
	if (textclasslist.Load(new_class)) {
		// successfully loaded
		setMinibuffer(lv_, _("Converting document to new document class..."));

		CutAndPaste cap;
		int ret = cap.SwitchLayoutsBetweenClasses(
			params.textclass, new_class,
			lv_->buffer()->paragraph);
		if (ret) {
			/* FIXME: error message */
		}

		params.textclass = new_class;
	} else
		return false;

	return true;
}

 
void FormDocument::close()
{
	h_.disconnect();
}


void FormDocument::hide()
{
	dialog_->hide();
	close();
}
