/****************************************************************************
** Form interface generated from reading ui file 'printdlg.ui'
**
** Created: Sun Feb 4 23:02:16 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef PRINTDLG_H
#define PRINTDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class PrintDlg : public QDialog
{ 
    Q_OBJECT

public:
    PrintDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PrintDlg();

    QButtonGroup* ButtonGroup1;
    QRadioButton* toPrinter;
    QRadioButton* toFile;
    QLineEdit* printerName;
    QLineEdit* fileName;
    QPushButton* browsePB;
    QButtonGroup* ButtonGroup3;
    QRadioButton* allPages;
    QRadioButton* oddPages;
    QRadioButton* evenPages;
    QLabel* fromPageL;
    QLabel* toPageL;
    QLineEdit* toPage;
    QCheckBox* reverse;
    QLineEdit* fromPage;
    QRadioButton* customPages;
    QGroupBox* GroupBox3;
    QSpinBox* copies;
    QCheckBox* collate;
    QPushButton* printPB;
    QPushButton* cancelPB;

protected slots:
    virtual void cancel_adaptor();
    virtual void enable_pagerange(int);
    virtual void browse_file();
    virtual void print();
    virtual void set_collate(int);

protected:
    QVBoxLayout* PrintDlgLayout;
    QGridLayout* ButtonGroup1Layout;
    QGridLayout* ButtonGroup3Layout;
    QHBoxLayout* GroupBox3Layout;
    QHBoxLayout* Layout1;
};

#endif // PRINTDLG_H
