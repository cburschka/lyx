/****************************************************************************
** Form interface generated from reading ui file 'searchdlg.ui'
**
** Created: Sun Mar 4 21:52:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;

class SearchDlg : public QDialog
{ 
    Q_OBJECT

public:
    SearchDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SearchDlg();

    QLabel* findStrLabel;
    QComboBox* find;
    QLabel* replaceLabel;
    QComboBox* replace;
    QCheckBox* caseSensitive;
    QCheckBox* matchWord;
    QPushButton* findPB;
    QPushButton* replacePB;
    QPushButton* replaceAllPB;
    QCheckBox* searchBack;
    QPushButton* cancelPB;

protected slots:
    virtual void Find();
    virtual void Replace();
    virtual void ReplaceAll();
    virtual void cancel_adaptor();

protected:
    QGridLayout* SearchDlgLayout;
};

#endif // SEARCHDLG_H
