/****************************************************************************
** Form interface generated from reading ui file 'paragraphdlg.ui'
**
** Created: Fri Mar 9 13:08:44 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef PARAGRAPHDLG_H
#define PARAGRAPHDLG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QWidget;

class ParagraphDlg : public QDialog
{ 
    Q_OBJECT

public:
    ParagraphDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ParagraphDlg();

    QTabWidget* TabsParagraph;
    QWidget* tab;
    QLabel* alignmentL;
    QComboBox* alignment;
    QCheckBox* noIndent;
    QGroupBox* spacingOptions;
    QComboBox* spacingBelowValueUnit;
    QComboBox* spacingBelowPlusUnit;
    QComboBox* spacingBelowMinusUnit;
    QComboBox* spacingAbovePlusUnit;
    QComboBox* spacingAboveValueUnit;
    QLineEdit* spacingBelowValue;
    QLineEdit* spacingBelowPlus;
    QComboBox* spacingAboveMinusUnit;
    QLineEdit* spacingAboveValue;
    QLineEdit* spacingAbovePlus;
    QLineEdit* spacingAboveMinus;
    QLineEdit* spacingBelowMinus;
    QLabel* spacingBelowUnitsL;
    QLabel* spacingAboveUnitsL;
    QLabel* spacingValueL;
    QLabel* spacingPlusL;
    QLabel* spacingMinusL;
    QComboBox* spacingAbove;
    QLabel* spacingAboveL;
    QLabel* spacingBelowL;
    QComboBox* spacingBelow;
    QCheckBox* spacingAboveKeep;
    QCheckBox* spacingBelowKeep;
    QGroupBox* listOptions;
    QLabel* labelWidthL;
    QLineEdit* labelWidth;
    QWidget* tab_2;
    QGroupBox* pagebreakOptions;
    QCheckBox* pagebreakAbove;
    QCheckBox* pagebreakBelow;
    QGroupBox* lineOptions;
    QCheckBox* lineAbove;
    QCheckBox* lineBelow;
    QWidget* tab_3;
    QLabel* extraUnitL;
    QComboBox* extraUnit;
    QLineEdit* extraWidth;
    QComboBox* extraType;
    QLabel* extraWidthL;
    QGroupBox* minipageOptions;
    QCheckBox* minipageStart;
    QCheckBox* minipageHfill;
    QLabel* minipageValignL;
    QComboBox* minipageValign;
    QPushButton* defaultsPB;
    QPushButton* applyPB;
    QPushButton* okPB;
    QPushButton* cancelPB;

protected slots:
    virtual void apply_adaptor();
    virtual void cancel_adaptor();
    virtual void enable_extraOptions(int);
    virtual void enable_minipageOptions(int);
    virtual void enable_spacingAbove(int);
    virtual void enable_spacingBelow(int);
    virtual void ok_adaptor();
    virtual void restore_adaptor();

protected:
    QVBoxLayout* ParagraphDlgLayout;
    QVBoxLayout* tabLayout;
    QHBoxLayout* Layout6;
    QGridLayout* spacingOptionsLayout;
    QHBoxLayout* Layout2_1;
    QHBoxLayout* Layout2_1_2;
    QHBoxLayout* listOptionsLayout;
    QGridLayout* tabLayout_2;
    QVBoxLayout* pagebreakOptionsLayout;
    QVBoxLayout* lineOptionsLayout;
    QGridLayout* tabLayout_3;
    QGridLayout* Layout7;
    QVBoxLayout* minipageOptionsLayout;
    QHBoxLayout* Layout15;
    QHBoxLayout* Layout1;
};

#endif // PARAGRAPHDLG_H
