#ifndef FORMREFDIALOG_H
#define FORMREFDIALOG_H
#include "FormRefDialogBase.h"

class FormRef;

class FormRefDialog : public FormReferenceDialogBase
{ 
    Q_OBJECT

public:
    FormRefDialog( FormRef* form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormRefDialog();

public slots:
    void apply_adaptor();
    void goto_adaptor();
    void highlight_adaptor(const QString&);
    void close_adaptor();
    void select_adaptor(const QString&);
    void sort_adaptor(bool);
    void update_adaptor();

protected:
    void closeEvent(QCloseEvent *e);

private:
    FormRef* form;
};

#endif // FORMREFDIALOG_H
