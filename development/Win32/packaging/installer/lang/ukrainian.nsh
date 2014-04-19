/*
LyX Installer Language File
Language: Ukrainian
Author: Yuri Chornoivan
*/

!insertmacro LANGFILE_EXT "Ukrainian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Встановлено для поточного користувача)"

${LangFileString} TEXT_WELCOME "За допомогою цього майстра ви зможете встановити LyX у вашу систему.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Оновлення бази даних назв файлів MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Налаштовування LyX (може виникнути потреба у отриманні $LaTeXInstalled додаткових пакунків, зачекайте)..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Обробка скриптів Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Створити значок на стільниці"
${LangFileString} TEXT_FINISH_WEBSITE "Відвідати lyx.org, щоб ознайомитися з новинами, довідковими матеріалами та підказками"

${LangFileString} FileTypeTitle "Документ LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Встановити для всіх користувачів?"
${LangFileString} SecFileAssocTitle "Прив’язка файлів"
${LangFileString} SecDesktopTitle "Піктограма стільниці"

${LangFileString} SecCoreDescription "Файли LyX."
${LangFileString} SecInstJabRefDescription "Засіб для роботи з бібліографічними посиланнями і редактор файлів BibTeX."
${LangFileString} SecAllUsersDescription "Визначає, чи слід встановити LyX для всіх користувачів, чи лише для поточного користувача."
${LangFileString} SecFileAssocDescription "Файли з суфіксом .lyx автоматично відкриватимуться за допомогою LyX."
${LangFileString} SecDesktopDescription "Піктограма LyX на стільниці."
${LangFileString} SecDictionariesDescription "Словники для перевірки правопису, які можна отримати і встановити."
${LangFileString} SecThesaurusDescription "Словники синонімів, які можна отримати і встановити."

${LangFileString} EnterLaTeXHeader1 'Дистрибутив LaTeX'
${LangFileString} EnterLaTeXHeader2 'Вкажіть дистрибутив LaTeX, який має використовувати LyX.'
${LangFileString} EnterLaTeXFolder 'Крім того, ви можете встановити тут розташування файла $\"latex.exe$\" і таким чином \
					встановити дистрибутив LaTeX, який має бути використано LyX.\r\n\
					Якщо ви не використовуватимете LaTeX, LyX не зможе створювати документів!\r\n\
					\r\n\
					Засобу встановлення програми вдалося виявити дистрибутив LaTeX \
					$\"$LaTeXName$\" у вашій системі. Нижче показано його розташування.'
${LangFileString} EnterLaTeXFolderNone 'Нижче ви маєте встановити розташування файла $\"latex.exe$\". Таким чином, ви встановите \
					дистрибутив LaTeX, який слід використовувати у LyX.\r\n\
					Якщо ви не використовуватимете LaTeX, LyX не зможе створювати документів!\r\n\
					\r\n\
					Засобу встановлення не вдалося виявити дистрибутива LaTeX у вашій системі.'
${LangFileString} PathName 'Розташування файла $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Не використовувати LaTeX"
${LangFileString} InvalidLaTeXFolder 'У вказаній теці немає файла $\"latex.exe$\".'

${LangFileString} LatexInfo 'Тепер буде запущено засіб встановлення дистрибутива LaTeX з назвою $\"MiKTeX$\".$\r$\n\
				Щоб встановити програму, натискайте кнопку $\"Next$\" у вікні засобу для встановлення, аж доки не розпочнеться процес встановлення.$\r$\n\
				$\r$\n\
				!!! Будь ласка, використовуйте лише типові параметри засобу для встановлення MiKTeX !!!'
${LangFileString} LatexError1 'Дистрибутивів LaTeX не виявлено!$\r$\n\
				Ви не зможете користуватися LyX без дистрибутива LaTeX на зразок $\"MiKTeX$\"!$\r$\n\
				З цієї причини встановлення буде припинено.'
      
${LangFileString} HunspellFailed 'Спроба отримання словника для мови $\"$R3$\" зазнала невдачі.'
${LangFileString} ThesaurusFailed 'Спроба отримання словника синонімів для мови $\"$R3$\" зазнала невдачі.'

${LangFileString} JabRefInfo 'Тепер буде запущено засіб для встановлення програми $\"JabRef$\".$\r$\n\
				У засобі для встановлення ви можете не змінювати типових параметрів JabRef.'
${LangFileString} JabRefError 'Не вдалося встановити програму $\"JabRef$\"!$\r$\n\
				Процес встановлення LyX буде продовжено.$\r$\n\
				Спробуйте встановити JabRef пізніше самостійно.'

${LangFileString} LatexConfigInfo "Налаштування LyX може тривати досить довго."

${LangFileString} MiKTeXPathInfo "Щоб кожен з користувачів міг пізніше налаштувати MiKTeX згідно до власних потреб,$\r$\n\
					слід встановити права на запис MiKTeX для всіх користувачів до теки, куди його встановлено:$\r$\n\
					$MiKTeXPath $\r$\n\
					і всіх підтек цієї теки."
${LangFileString} MiKTeXInfo 'У LyX використовуватиметься дистрибутив LaTeX $\"MiKTeX$\".$\r$\n\
				Ми рекомендуємо вам встановити всі доступні оновлення MiKTeX за допомогою програми $\"MiKTeX Update Wizard$\"$\r$\n\
				до того, як ви вперше скористаєтеся LyX.$\r$\n\
				Бажаєте перевірити наявність оновлень MiKTeX?'

${LangFileString} ModifyingConfigureFailed "Не вдалося встановити 'path_prefix' у скрипті налаштування"
${LangFileString} RunConfigureFailed "Не вдалося виконати скрипт налаштування"
${LangFileString} NotAdmin "Для встановлення LyX вам потрібні привілеї адміністратора!"
${LangFileString} InstallRunning "Засіб для встановлення вже працює!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} вже встановлено!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Ви намагаєтеся встановити версію LyX, яка є застарілою порівняно з вже встановленою.$\r$\n\
				  Якщо ви хочете встановити застарілу версію, вам слід спочатку вилучити вже встановлений LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Ви не зможете скористатися підтримкою декількох покажчиків у документі, оскільки $\r$\n\
						MiKTeX було встановлено від імені адміністратора, а LyX - від імені звичайного користувача."
${LangFileString} MetafileNotAvailable "Ви не зможете скористатися підтримкою роботи з зображеннями у форматах EMF і WMF,$\r$\n\
					оскільки для роботи з такими зображеннями потрібен програмний принтер для$\r$\n\
					Windows, встановити який можна лише від імені адміністратора системи."

${LangFileString} FinishPageMessage "Вітаємо! LyX було успішно встановлено.$\r$\n\
					$\r$\n\
					(Перший запуск LyX може тривати декілька секунд.)"
${LangFileString} FinishPageRun "Запустити LyX"

${LangFileString} UnNotInRegistryLabel "Не вдалося знайти записи LyX у регістрі.$\r$\n\
					Записи на стільниці і у меню запуску вилучено не буде."
${LangFileString} UnInstallRunning "Спочатку слід завершити роботу програми LyX!"
${LangFileString} UnNotAdminLabel "Для вилучення LyX вам слід мати привілеї адміністратора!"
${LangFileString} UnReallyRemoveLabel "Ви справді бажаєте повністю вилучити LyX і всі його компоненти?"
${LangFileString} UnLyXPreferencesTitle 'Параметри LyX, встановлені користувачем'

${LangFileString} SecUnMiKTeXDescription "Вилучає дистрибутив LaTeX - MiKTeX."
${LangFileString} SecUnJabRefDescription "Вилучає засіб для роботи з бібліографічними даними JabRef."
${LangFileString} SecUnPreferencesDescription 'Вилучає теку з налаштуваннями LyX$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						для всіх користувачів.'
${LangFileString} SecUnProgramFilesDescription "Вилучити LyX і всі його компоненти."

