#ifndef FORMURLDIALOG_H
#define FORMURLDIALOG_H
#include "FormUrlDialogBase.h"

class FormUrl;

class FormUrlDialog : public FormUrlDialogBase
{ 
    Q_OBJECT

public:
    FormUrlDialog( FormUrl* form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormUrlDialog();

public slots:
    void apply_adaptor();
    void close_adaptor();

protected:
    void closeEvent( QCloseEvent* );

private:
    FormUrl* form;
};

#endif // FORMURLDIALOG_H
