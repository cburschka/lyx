/****************************************************************************
** Form interface generated from reading ui file 'FormCopyrightDialogBase.ui'
**
** Created: Wed Jan 24 12:18:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORMCOPYRIGHTDIALOGBASE_H
#define FORMCOPYRIGHTDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;

class FormCopyrightDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    FormCopyrightDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormCopyrightDialogBase();

    QLabel* TextLabel5;
    QLabel* TextLabel5_2;
    QLabel* TextLabel5_2_2;
    QPushButton* okPB;

protected:
    QVBoxLayout* FormCopyrightDialogBaseLayout;
};

#endif // FORMCOPYRIGHTDIALOGBASE_H
