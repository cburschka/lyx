#ifndef FORMTOCDIALOG_H
#define FORMTOCDIALOG_H
#include "FormTocDialogBase.h"

class FormToc;

class FormTocDialog : public FormTocDialogBase
{ 
    Q_OBJECT

public:
    FormTocDialog( FormToc* form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormTocDialog();

public slots:
    void activate_adaptor(int);
    void close_adaptor();
    void depth_adaptor(int);
    void select_adaptor(QListViewItem*);
    void update_adaptor();

protected:
    void closeEvent(QCloseEvent *e);

private:
    FormToc* form;
};

#endif // FORMTOCDIALOG_H
