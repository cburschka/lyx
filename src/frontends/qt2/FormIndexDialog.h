#ifndef FORMINDEXDIALOG_H
#define FORMINDEXDIALOG_H
#include "FormIndexDialogBase.h"

class FormIndex;

class FormIndexDialog : public FormIndexDialogBase
{ 
    Q_OBJECT

public:
    FormIndexDialog( FormIndex* form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormIndexDialog();

public slots:
	void apply_adaptor();
  void close_adaptor();

protected:
  void closeEvent(QCloseEvent *e);

private:
    FormIndex* form;
};

#endif // FORMINDEXDIALOG_H
