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
#include <QTreeWidget>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

namespace {

QString const guiString(QString in)
{
	// recode specially encoded chars in file names (URL encoding and underbar)
	return QString(QByteArray::fromPercentEncoding(in.toUtf8())).replace('_', ' ');
}

} // namespace anon


QMap<QString, QString> GuiLyXFiles::getFiles()
{
	QMap<QString, QString> result;
	// We look for lyx files in the subdirectory dir of
	//   1) user_lyxdir
	//   2) build_lyxdir (if not empty)
	//   3) system_lyxdir
	// in this order. Files with a given sub-hierarchy will
	// only be listed once.
	// We also consider i18n subdirectories and store them separately.
	QStringList dirs;
	QStringList relpaths;

	// The three locations to look at.
	string const user = addPath(package().user_support().absFileName(), fromqstr(type_));
	string const build = addPath(package().build_support().absFileName(), fromqstr(type_));
	string const system = addPath(package().system_support().absFileName(), fromqstr(type_));

	available_languages_.insert(toqstr("en"), qt_("English"));

	QString const type = fileTypeCO->itemData(fileTypeCO->currentIndex()).toString();

	// Search in the base paths
	if (type == "all" || type == "user")
		dirs << toqstr(user);
	if (type == "all" || type == "system")
		dirs << toqstr(build)
		     << toqstr(system);

	for (int i = 0; i < dirs.size(); ++i) {
		QString const dir = dirs.at(i);
		QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString fn(QFile(it.next()).fileName());
			if (!fn.endsWith(getSuffix()))
				continue;
			QString relpath = toqstr(makeRelPath(qstring_to_ucs4(fn),
							     qstring_to_ucs4(dir)));
			// <cat>/
			int s = relpath.indexOf('/', 0);
			QString cat = qt_("General");
			QString localization = "en";
			if (s != -1) {
				// <cat>/<subcat>/
				cat = relpath.left(s);
				if (all_languages_.contains(cat)
				    && !all_languages_.contains(dir.right(dir.lastIndexOf('/')))) {
					QMap<QString, QString>::const_iterator li = all_languages_.find(cat);
					// Skip i18n dir, but add language to the combo
					if (!available_languages_.contains(li.key()))
						available_languages_.insert(li.key(), li.value());
					localization = cat;
					int sc = relpath.indexOf('/', s + 1);
					cat = (sc == -1) ? qt_("General") : relpath.mid(s + 1, sc - s - 1);
					s = sc;
				}
				if (s != -1) {
					int sc = relpath.indexOf('/', s + 1);
					QString const subcat = (sc == -1) ?
								QString() : relpath.mid(s + 1, sc - s - 1);
					if (!subcat.isEmpty())
						cat += '/' + subcat;
				}
			}
			if (!relpaths.contains(relpath)) {
				relpaths.append(relpath);
				if (localization != "en")
					// strip off lang/
					relpath = relpath.mid(relpath.indexOf('/') + 1);
				result.insert(relpath, cat);
									
				QMap<QString, QString> lm;
				if (localizations_.contains(relpath))
					lm = localizations_.find(relpath).value();
				lm.insert(localization, fn);
				localizations_.insert(relpath, lm);
			}
		}
	}
	// Find and store GUI language
	for (auto const & l : guilangs_) {
		// First try with the full name
		// `en' files are not in a subdirectory
		if (available_languages_.contains(toqstr(l))) {
			guilang_ = toqstr(l);
			break;
		}
		// Then the name without country code
		string const shortl = token(l, '_', 0);
		if (available_languages_.contains(toqstr(shortl))) {
			guilang_ = toqstr(shortl);
			break;
		}
	}
	// pre-fill the language combo (it will be updated once an item 
	// has been clicked)
	languageCO->clear();
	QMap<QString, QString>::const_iterator i =available_languages_.constBegin();
	while (i != available_languages_.constEnd()) {
		languageCO->addItem(i.value(), i.key());
		++i;
	}
	setLanguage();
	languageLA->setText(qt_("Preferred &Language:"));
	return result;
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
		QString const code = toqstr(lang->code());
		if (!all_languages_.contains(code))
			all_languages_.insert(code, qt_(lang->display()));
		// Also store code without country code
		QString const shortcode = code.left(code.indexOf('_'));
		if (shortcode != code && !all_languages_.contains(shortcode))
			all_languages_.insert(shortcode, qt_(lang->display()));
	}
	// Get GUI language
	string lang = getGuiMessages().language();
	string const language = getEnv("LANGUAGE");
	if (!language.empty())
		lang += ":" + language;
	guilangs_ =  getVectorFromString(lang, ":");

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

	QIcon user_icon(getPixmap("images/", "lyxfiles-user", "svgz,png"));
	QIcon system_icon(getPixmap("images/", "lyxfiles-system", "svgz,png"));
	fileTypeCO->addItem(qt_("User and System Files"), toqstr("all"));
	fileTypeCO->addItem(user_icon, qt_("User Files Only"), toqstr("user"));
	fileTypeCO->addItem(system_icon, qt_("System Files Only"), toqstr("system"));

	setFocusProxy(filter_);
}


QString const GuiLyXFiles::getSuffix()
{
	if (type_ == "bind" || type_ == "ui")
		return toqstr(".") + type_;
	else if (type_ == "kbd")
		return ".kmap";
	
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


void GuiLyXFiles::on_languageCO_activated(int i)
{
	savelang_ = languageCO->itemData(i).toString();
	if (!filesLW->currentItem())
		return;

	filesLW->currentItem()->setData(0, Qt::ToolTipRole, getRealPath());
	changed();
}


void GuiLyXFiles::on_filesLW_itemDoubleClicked(QTreeWidgetItem * item, int)
{
	if (!item->data(0, Qt::UserRole).toString().endsWith(getSuffix()))
		// not a file (probably a header)
		return;

	applyView();
	dispatchParams();
	close();
}

void GuiLyXFiles::on_filesLW_itemClicked(QTreeWidgetItem * item, int)
{
	QString const data = item->data(0, Qt::UserRole).toString();
	if (!data.endsWith(getSuffix()))
		// not a file (probably a header)
		return;

	languageCO->clear();
	QMap<QString, QString>::const_iterator i =available_languages_.constBegin();
	while (i != available_languages_.constEnd()) {
		if (localizations_.contains(data)
		    && localizations_.find(data).value().contains(i.key()))
			languageCO->addItem(i.value(), i.key());
		++i;
	}
	languageLA->setText(qt_("File &Language:"));
	languageCO->setToolTip(qt_("All available languages of the selected file are displayed here.\n"
				   "The selected language version will be opened."));
	setLanguage();
	QString const realpath = getRealPath();
	filesLW->currentItem()->setData(0, Qt::ToolTipRole, realpath);
	QIcon user_icon(getPixmap("images/", "lyxfiles-user", "svgz,png"));
	QIcon system_icon(getPixmap("images/", "lyxfiles-system", "svgz,png"));
	QIcon file_icon = (realpath.startsWith(toqstr(package().user_support().absFileName()))) ?
			user_icon : system_icon;
	item->setIcon(0, file_icon);
}


void GuiLyXFiles::setLanguage()
{
	// Enable language selection only if there is a selection.
	languageCO->setEnabled(languageCO->count() > 1);
	languageLA->setEnabled(languageCO->count() > 1);
	// first try last setting
	if (!savelang_.isEmpty()) {
		int index = languageCO->findData(savelang_);
		if (index != -1) {
			languageCO->setCurrentIndex(index);
			return;
		}
	}
	// next, try GUI lang
	if (!guilang_.isEmpty()) {
		int index = languageCO->findData(guilang_);
		if (index != -1) {
			languageCO->setCurrentIndex(index);
			return;
		}
	}
	// Finally, fall back to English (which should be always there)
	int index = languageCO->findData(toqstr("en"));
	if (index != -1) {
		languageCO->setCurrentIndex(index);
	}
}


void GuiLyXFiles::on_browsePB_pressed()
{
	QString path1 = toqstr(lyxrc.document_path);
	QString path2 = toqstr(lyxrc.example_path);
	QString title = qt_("Select example file");
	QString filter = qt_("LyX Documents (*.lyx)");
	QString b1 = qt_("D&ocuments");
	QString b2 = qt_("&Examples");

	if (type_ == "templates") {
		path2 = toqstr(lyxrc.template_path);
		title = qt_("Select template file");
		b1 = qt_("D&ocuments");
		b2 = qt_("&Templates");
	}
	else if (type_ != "examples") {
		path1 = toqstr(addName(package().user_support().absFileName(), fromqstr(type_)));
		path2 = toqstr(addName(package().system_support().absFileName(), fromqstr(type_)));
		b1 = qt_("&User files");
		b2 = qt_("&System files");
	}
	if (type_ == "ui") {
		title = qt_("Chose UI file");
		filter = qt_("LyX UI Files (*.ui)");
	}
	if (type_ == "bind") {
		title = qt_("Chose bind file");
		filter = qt_("LyX Bind Files (*.bind)");
	}
	if (type_ == "kbd") {
		title = qt_("Chose keyboard map");
		filter = qt_("LyX Keymap Files (*.kmap)");
	}

	FileDialog dlg(title);
	dlg.setButton1(b1, path1);
	dlg.setButton2(b2, path2);

	FileDialog::Result result = dlg.open(path2, QStringList(filter));

	if (result.first != FileDialog::Later && !result.second.isEmpty()) {
		file_ = toqstr(FileName(fromqstr(result.second)).absFileName());
		dispatchParams();
		close();
	}
}


void GuiLyXFiles::updateContents()
{
	languageCO->clear();
	QMap<QString, QString> files = getFiles();
	languageCO->model()->sort(0);

	filesLW->clear();
	QIcon user_icon(getPixmap("images/", "lyxfiles-user", "svgz,png"));
	QIcon system_icon(getPixmap("images/", "lyxfiles-system", "svgz,png"));
	QStringList cats;
	QMap<QString, QString>::const_iterator it = files.constBegin();
	QFont capfont;
	capfont.setBold(true);
	while (it != files.constEnd()) {
		QFileInfo const info = QFileInfo(it.key());
		QString const realpath = getRealPath(it.key());
		QString cat = it.value();
		QString subcat;
		QString catsave;
		if (cat.contains('/')) {
			catsave = cat;
			cat = catsave.left(catsave.indexOf('/'));
			subcat = toqstr(translateIfPossible(
					qstring_to_ucs4(guiString(catsave.mid(catsave.indexOf('/') + 1)))));
		}
		cat =  toqstr(translateIfPossible(qstring_to_ucs4(guiString(cat))));
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
			guiname = toqstr(translateIfPossible(qstring_to_ucs4(guiString(guiname))));
		QIcon file_icon = (realpath.startsWith(toqstr(package().user_support().absFileName()))) ?
				user_icon : system_icon;
		item->setIcon(0, file_icon);
		item->setData(0, Qt::UserRole, it.key());
		item->setData(0, Qt::DisplayRole, guiname);
		item->setData(0, Qt::ToolTipRole, realpath);
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
				subcatItem->setIcon(0, file_icon);
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

QString const GuiLyXFiles::getRealPath(QString relpath)
{
	if (relpath.isEmpty())
		relpath = filesLW->currentItem()->data(0, Qt::UserRole).toString();
	QString const language = languageCO->itemData(languageCO->currentIndex()).toString();
	if (localizations_.contains(relpath)) {
		if (localizations_.find(relpath).value().contains(language))
			return localizations_.find(relpath).value().find(language).value();
		else if (localizations_.find(relpath).value().contains(guilang_))
			return localizations_.find(relpath).value().find(guilang_).value();
		else if (localizations_.find(relpath).value().contains(toqstr("en")))
			return localizations_.find(relpath).value().find(toqstr("en")).value();
	}
	return QString();
}


void GuiLyXFiles::applyView()
{
	file_ = getRealPath();
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


void GuiLyXFiles::passParams(string const & data)
{
	initialiseParams(data);
	updateContents();
}


void GuiLyXFiles::selectItem(QString const item)
{
	/* Using an intermediary variable flags is needed up to at least
	 * Qt 5.5 because of a subtle namespace issue. See:
	 *   https://stackoverflow.com/questions/10755058/qflags-enum-type-conversion-fails-all-of-a-sudden
	 * for details.*/
	Qt::MatchFlags const flags(Qt::MatchExactly|Qt::MatchRecursive);
	QList<QTreeWidgetItem *> twi = filesLW->findItems(item, flags);
	if (!twi.isEmpty())
		twi.first()->setSelected(true);
}


void GuiLyXFiles::paramsToDialog()
{
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

	if (lfun == LFUN_NOACTION)
		// emit signal
		fileSelected(file_);
	else
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
