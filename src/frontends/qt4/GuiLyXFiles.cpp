/**
 * \file GuiLyXFiles.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiLyXFiles.h"
#include "GuiApplication.h"
#include "qt_helpers.h"

#include "FileDialog.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LyXRC.h"

#include "support/environment.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Messages.h"
#include "support/qstring_helpers.h"
#include "support/Package.h"

#include <QDirIterator>
#include <QFileIconProvider>
#include <QTreeWidget>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


void GuiLyXFiles::getFiles(QMap<QString, QString> & in, QString const type)
{
	// We look for lyx files in the subdirectory dir of
	//   1) user_lyxdir
	//   2) build_lyxdir (if not empty)
	//   3) system_lyxdir
	// in this order. Files with a given sub-hierarchy will
	// only be listed once.
	// We also consider i18n subdirectories and prefer them.
	QStringList dirs;
	QStringList relpaths;

	// The three locations to look at.
	string const user = addPath(package().user_support().absFileName(), fromqstr(type));
	string const build = addPath(package().build_support().absFileName(), fromqstr(type));
	string const system = addPath(package().system_support().absFileName(), fromqstr(type));

	// First, query the current language subdir (except for English)
	QString const lang = languageCO->currentData().toString();
	if (!lang.startsWith("en")) {
		// First try with the full code
		dirs << toqstr(addPath(user, fromqstr(lang)));
		dirs << toqstr(addPath(build, fromqstr(lang)));
		dirs << toqstr(addPath(system, fromqstr(lang)));
		// Then the name without country code
		QString const shortl = lang.left(lang.indexOf('_'));
		if (shortl != lang) {
			dirs << toqstr(addPath(user, fromqstr(shortl)));
			dirs << toqstr(addPath(build, fromqstr(shortl)));
			dirs << toqstr(addPath(system, fromqstr(shortl)));
		}
	}

	// Next, search in the base path
	dirs << toqstr(user)
	     << toqstr(build)
	     << toqstr(system);

	for (int i = 0; i < dirs.size(); ++i) {
		QString const dir = dirs.at(i);
		QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString fn(QFile(it.next()).fileName());
			if (!fn.endsWith(getSuffix()))
				continue;
			QString const relpath = toqstr(makeRelPath(qstring_to_ucs4(fn),
								   qstring_to_ucs4(dir)));
			// <cat>/
			int s = relpath.indexOf('/', 0);
			QString cat = qt_("General");
			if (s != -1) {
				// <cat>/<subcat>/
				cat = relpath.left(s);
				int sc = relpath.indexOf('/', s + 1);
				QString const subcat = (sc == -1) ?
							QString() : relpath.mid(s + 1, sc - s - 1);
				if (langcodes_.contains(cat)
				    && !langcodes_.contains(dir.right(dir.lastIndexOf('/'))))
					// Skip i18n dir
					continue;
				if (!subcat.isEmpty())
					cat += '/' + subcat;
			}
			if (!relpaths.contains(relpath)) {
				relpaths.append(relpath);
				in.insert(fn, cat);
			}
		}
	}
}


GuiLyXFiles::GuiLyXFiles(GuiView & lv)
	: GuiDialog(lv, "lyxfiles", qt_("New File From Template"))
{
	setupUi(this);

	// Get all supported languages (by code) in order to exclude those
	// dirs later.
	QAbstractItemModel * language_model = guiApp->languageModel();
	language_model->sort(0);
	for (int i = 0; i != language_model->rowCount(); ++i) {
		QModelIndex index = language_model->index(i, 0);
		Language const * lang =
			languages.getLanguage(fromqstr(index.data(Qt::UserRole).toString()));
		if (!lang)
			continue;
		string const code = lang->code();
		languageCO->addItem(qt_(lang->display()), toqstr(code));
		langcodes_ << toqstr(code);
		// Also store code without country code
		string const shortcode = token(code, '_', 0);
		if (shortcode != code)
			langcodes_ << toqstr(shortcode);
	}
	// Preset to GUI language
	string lang = getGuiMessages().language();
	string const language = getEnv("LANGUAGE");
	if (!language.empty())
		lang += ":" + language;

	for (auto const & l : getVectorFromString(lang, ":")) {
		// First try with the full name
		// `en' files are not in a subdirectory
		int i = languageCO->findData(toqstr(l));
		if (i != -1) {
			languageCO->setCurrentIndex(i);
			break;
		}
		// Then the name without country code
		string const shortl = token(l, '_', 0);
		i = languageCO->findData(toqstr(l));
		if (i != -1) {
			languageCO->setCurrentIndex(i);
			break;
		}
	}

	// The filter bar
	filter_ = new FancyLineEdit(this);
	filter_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	filter_->setButtonVisible(FancyLineEdit::Right, true);
	filter_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	filter_->setAutoHideButton(FancyLineEdit::Right, true);
	filter_->setPlaceholderText(qt_("All available files"));
	filter_->setToolTip(qt_("Enter string to filter the list of available files"));
#if (QT_VERSION < 0x050000)
	connect(filter_, SIGNAL(downPressed()),
		filesLW, SLOT(setFocus()));
#else
	connect(filter_, &FancyLineEdit::downPressed,
		filesLW, [=](){ focusAndHighlight(filesLW); });
#endif

	filterBarL->addWidget(filter_, 0);
	findKeysLA->setBuddy(filter_);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

	connect(filesLW, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(changed_adaptor()));
	connect(filesLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(changed_adaptor()));
	connect(filter_, SIGNAL(textEdited(QString)),
		this, SLOT(filterLabels()));
	connect(filter_, SIGNAL(rightButtonClicked()),
		this, SLOT(resetFilter()));

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Open));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));

	//filesLW->setViewMode(QListView::ListMode);
	filesLW->setIconSize(QSize(22, 22));

	setFocusProxy(filter_);
}


QString const GuiLyXFiles::getSuffix()
{
	if (type_ == "bind" || type_ == "ui")
		return toqstr(".") + type_;
	
	return ".lyx";
}


bool GuiLyXFiles::translateName() const
{
	return (type_ == "templates" || type_ == "examples");
}


void GuiLyXFiles::changed_adaptor()
{
	changed();
}


void GuiLyXFiles::on_fileTypeCO_activated(int)
{
	updateContents();
}


void GuiLyXFiles::on_languageCO_activated(int)
{
	updateContents();
}


void GuiLyXFiles::on_filesLW_itemDoubleClicked(QTreeWidgetItem *, int)
{
	applyView();
	dispatchParams();
	close();
}


void GuiLyXFiles::on_browsePB_pressed()
{
	bool const examples = (type_ == "examples");
	FileDialog dlg(qt_("Select template file"));
	dlg.setButton1(qt_("D&ocuments"), toqstr(lyxrc.document_path));
	if (examples)
		dlg.setButton2(qt_("&Examples"), toqstr(lyxrc.example_path));
	else
		dlg.setButton2(qt_("&Templates"), toqstr(lyxrc.template_path));

	FileDialog::Result result = dlg.open(examples ? toqstr(lyxrc.example_path)
						      : toqstr(lyxrc.template_path),
				 QStringList(qt_("LyX Documents (*.lyx)")));

	if (result.first != FileDialog::Later && !result.second.isEmpty()) {
		file_ = toqstr(FileName(fromqstr(result.second)).absFileName());
		dispatchParams();
		close();
	}
}


void GuiLyXFiles::updateContents()
{
	QString type = fileTypeCO->itemData(fileTypeCO->currentIndex()).toString();
	QMap<QString, QString> files;
	getFiles(files, type);

	filesLW->clear();
	QFileIconProvider iconprovider;
	QStringList cats;
	QMap<QString, QString>::const_iterator it = files.constBegin();
	QFont capfont;
	capfont.setBold(true);
	while (it != files.constEnd()) {
		QFileInfo const info = QFileInfo(it.key());
		QString cat = it.value();
		QString subcat;
		QString catsave;
		if (cat.contains('/')) {
			catsave = cat;
			cat = catsave.left(catsave.indexOf('/'));
			subcat = toqstr(translateIfPossible(
					qstring_to_ucs4(catsave.mid(
						catsave.indexOf('/') + 1).replace('_', ' '))));
		}
		cat =  toqstr(translateIfPossible(qstring_to_ucs4(cat.replace('_', ' '))));
		QTreeWidgetItem * catItem = new QTreeWidgetItem();
		if (!cats.contains(cat)) {
			catItem->setText(0, cat);
			catItem->setFont(0, capfont);
			filesLW->insertTopLevelItem(0, catItem);
			catItem->setExpanded(true);
			cats << cat;
		} else
			catItem = filesLW->findItems(cat, Qt::MatchExactly).first();
		QTreeWidgetItem * item = new QTreeWidgetItem();
		QString const filename = info.fileName();
		QString guiname = filename.left(filename.lastIndexOf(getSuffix())).replace('_', ' ');
		if (translateName())
			guiname = toqstr(translateIfPossible(qstring_to_ucs4(guiname)));
		item->setIcon(0, iconprovider.icon(info));
		item->setData(0, Qt::UserRole, info.filePath());
		item->setData(0, Qt::DisplayRole, guiname);
		item->setData(0, Qt::ToolTipRole, info.filePath());
		if (subcat.isEmpty())
			catItem->addChild(item);
		else {
			QTreeWidgetItem * subcatItem = new QTreeWidgetItem();
			if (cats.contains(catsave)) {
				QList<QTreeWidgetItem *> pcats = filesLW->findItems(cat, Qt::MatchExactly);
				for (int iit = 0; iit < pcats.size(); ++iit) {
					for (int cit = 0; cit < pcats.at(iit)->childCount(); ++cit) {
						if (pcats.at(iit)->child(cit)->text(0) == subcat) {
							subcatItem = pcats.at(iit)->child(cit);
							break;
						}
					}
				}
			} else {
				subcatItem->setText(0, subcat);
				cats << catsave;
			}
			subcatItem->addChild(item);
			catItem->addChild(subcatItem);
		}
		++it;
	}
	filesLW->sortItems(0, Qt::AscendingOrder);
	// redo filter
	filterLabels();
}


void GuiLyXFiles::slotButtonBox(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Open:
		slotOK();
		break;
	case QDialogButtonBox::Cancel:
		slotClose();
		break;
	default:
		break;
	}
}


void GuiLyXFiles::filterLabels()
{
	Qt::CaseSensitivity cs = csFindCB->isChecked() ?
		Qt::CaseSensitive : Qt::CaseInsensitive;
	QTreeWidgetItemIterator it(filesLW);
	while (*it) {
		(*it)->setHidden(
			(*it)->childCount() == 0
			&& !(*it)->text(0).contains(filter_->text(), cs)
		);
		++it;
	}
}


void GuiLyXFiles::resetFilter()
{
	filter_->setText(QString());
	filterLabels();
}


void GuiLyXFiles::applyView()
{
	file_ = filesLW->currentItem()->data(0, Qt::UserRole).toString();
}


bool GuiLyXFiles::isValid()
{
	return filesLW->currentItem() && filesLW->currentItem()->isSelected();
}


bool GuiLyXFiles::initialiseParams(string const & type)
{
	type_ = type.empty() ? toqstr("templates") : toqstr(type);
	paramsToDialog();
	return true;
}


void GuiLyXFiles::paramsToDialog()
{
	fileTypeCO->clear();
	if (type_ == "examples" || type_ == "templates") {
		fileTypeCO->addItem(qt_("Templates"), toqstr("templates"));
		fileTypeCO->addItem(qt_("Examples"), toqstr("examples"));
	} else if (type_ == "ui")
		fileTypeCO->addItem(qt_("User Interface Files"), toqstr("ui"));
	else if (type_ == "bind")
		fileTypeCO->addItem(qt_("Key Binding Files"), toqstr("bind"));

	if (!type_.isEmpty()) {
		int i = fileTypeCO->findData(type_);
		if (i != -1)
			fileTypeCO->setCurrentIndex(i);
	}
	if (type_ == "examples")
		setTitle(qt_("Open Example File"));
	else if (type_ == "templates")
		setTitle(qt_("New File From Template"));
	else
		setTitle(qt_("Open File"));

	bc().setValid(isValid());
}


void GuiLyXFiles::dispatchParams()
{
	if (file_.isEmpty())
		return;

	string arg;
	if (type_ == "templates")
		arg = "newfile ";
	arg += fromqstr(file_);
	FuncCode const lfun = getLfun();

	dispatch(FuncRequest(lfun, arg));
}


FuncCode GuiLyXFiles::getLfun() const
{
	if (type_ == "examples")
		return LFUN_FILE_OPEN;
	else if (type_ == "templates")
		return LFUN_BUFFER_NEW_TEMPLATE;
	return LFUN_NOACTION;
}

Dialog * createGuiLyXFiles(GuiView & lv) { return new GuiLyXFiles(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiLyXFiles.cpp"
