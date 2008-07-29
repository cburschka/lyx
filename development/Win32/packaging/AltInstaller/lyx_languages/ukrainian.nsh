!ifndef _LYX_LANGUAGES_UKRAINIAN_NSH_
!define _LYX_LANGUAGES_UKRAINIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_UKRAINIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "За допомогою цього майстра ви зможете встановити LyX у вашу систему.\r\n\
					  \r\n\
					  Для того, щоб встановити LyX вам потрібні права адміністратора.\r\n\
					  \r\n\
					  Ми рекомендуємо вам закрити вікна всіх інших програм перш ніж розпочати встановлення. Це зробить можливим оновлення всіх потрібних системних файлів без перезавантаження вашого комп’ютера.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Документ LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Встановити для всіх користувачів?"
LangString SecFileAssocTitle "${LYX_LANG}" "Прив’язка файлів"
LangString SecDesktopTitle "${LYX_LANG}" "Піктограма стільниці"

LangString SecCoreDescription "${LYX_LANG}" "Файли LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Програма для перегляду документів у форматах Postscript і PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Засіб для роботи з бібліографічними посиланнями і редактор файлів BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Визначає, чи слід встановити LyX для всіх користувачів, чи лише для поточного користувача."
LangString SecFileAssocDescription "${LYX_LANG}" "Файли з суфіксом .lyx автоматично відкриватимуться за допомогою LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Піктограма LyX на стільниці."

LangString LangSelectHeader "${LYX_LANG}" "Вибір мови меню LyX"
LangString AvailableLang "${LYX_LANG}" " Доступні мови "

LangString MissProgHeader "${LYX_LANG}" "Перевірка наявності необхідних програм"
LangString MissProgCap "${LYX_LANG}" "Додатково буде встановлено таке, необхідне для роботи програми програмне забезпечення"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, дистрибутив LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, інтерпретатор файлів PostScript і PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, засіб для перетворення форматів зображень"
LangString MissProgAspell "${LYX_LANG}" "Aspell, засіб для перевірки орфографії"
LangString MissProgMessage "${LYX_LANG}" 'Не потрібно встановлювати жодних додаткових програм.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Дистрибутив LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Вкажіть дистрибутив LaTeX, який має використовувати LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Крім того, ви можете встановити тут розташування файла "latex.exe" і таким чином \
					   встановити дистрибутив LaTeX, який має бути використано LyX.\r\n\
					   Якщо ви не використовуватимете LaTeX, LyX не зможе створювати документів!\r\n\
					   \r\n\
					   Засобу встановлення програми вдалося виявити дистрибутив LaTeX \
					   "$LaTeXName" у вашій системі. Нижче показано його розташування.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Нижче ви маєте встановити розташування файла "latex.exe". Таким чином, ви встановите \
					       дистрибутив LaTeX, який слід використовувати у LyX.\r\n\
					       Якщо ви не використовуватимете LaTeX, LyX не зможе створювати документів!\r\n\
					       \r\n\
					       Засобу встановлення не вдалося виявити дистрибутива LaTeX у вашій системі.'
LangString PathName "${LYX_LANG}" 'Розташування файла "latex.exe"'
LangString DontUseLaTeX "${LYX_LANG}" "Не використовувати LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'У вказаній теці немає файла "latex.exe".'

LangString LatexInfo "${LYX_LANG}" 'Тепер буде запущено засіб встановлення дистрибутива LaTeX з назвою "MiKTeX".$\r$\n\
			            Щоб встановити програму, натискайте кнопку "Next" у вікні засобу для встановлення, аж доки не розпочнеться процес встановлення.$\r$\n\
				    $\r$\n\
				    !!! Будь ласка, використовуйте лише типові параметри засобу для встановлення MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Дистрибутивів LaTeX не виявлено!$\r$\n\
                      		      Ви не зможете користуватися LyX без дистрибутива LaTeX на зразок "MiKTeX"!$\r$\n\
				      З цієї причини встановлення буде припинено.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Тепер буде запущено засіб для встановлення програми "GSview".$\r$\n\
			             Щоб встановити програму, натисніть кнопку "Setup" у першому вікні засобу для встановлення,$\r$\n\
				     виберіть мову, а потім натискайте кнопку "Next" у всіх наступних вікнах засобу для встановлення.$\r$\n\
				     Ви можете використовувати типові параметри засобу для встановлення GSview.'
LangString GSviewError "${LYX_LANG}" 'Не вдалося встановити програму "GSview"!$\r$\n\
		                      Процес встановлення LyX буде продовжено.$\r$\n\
				      Спробуйте встановити GSview пізніше самостійно.'
				      
LangString JabRefInfo "${LYX_LANG}" 'Тепер буде запущено засіб для встановлення програми "JabRef".$\r$\n\
				     У засобі для встановлення ви можете не змінювати типових параметрів JabRef.'
LangString JabRefError "${LYX_LANG}" 'Не вдалося встановити програму "JabRef"!$\r$\n\
		                      Процес встановлення LyX буде продовжено.$\r$\n\
				      Спробуйте встановити JabRef пізніше самостійно.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "Налаштування LyX може тривати досить довго."

LangString AspellInfo "${LYX_LANG}" 'Тепер буде звантажено і встановлено словники для засобу перевірки орфографії "Aspell".$\r$\n\
				     Кожен зі словників має власну ліцензію, яку буде показано перед початком процесу встановлення.'
LangString AspellDownloadFailed "${LYX_LANG}" "Не вдалося звантажити жодного зі словників засобу для перевірки орфографії Aspell!"
LangString AspellInstallFailed "${LYX_LANG}" "Не вдалося встановити жодного зі словників засобу для перевірки орфографії Aspell!"
LangString AspellPartAnd "${LYX_LANG}" " і "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Вже було встановлено словник для'
LangString AspellNone "${LYX_LANG}" 'Не було встановлено жодного зі словників засобу для перевірки орфографії "Aspell".$\r$\n\
				     Словники можна звантажити з сайта$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Бажаєте звантажити словники зараз?'
LangString AspellPartStart "${LYX_LANG}" "Було успішно встановлено "
LangString AspellPart1 "${LYX_LANG}" "словник для англійської"
LangString AspellPart2 "${LYX_LANG}" "словник для $LangName"
LangString AspellPart3 "${LYX_LANG}" "словник$\r$\n\
				      для $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' засобу перевірки орфографії "Aspell".$\r$\n\
				      Додаткові словники можна звантажити з сайта$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Бажаєте звантажити інші словники зараз?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Щоб кожен з користувачів міг пізніше налаштувати MiKTeX згідно до власних потреб,$\r$\n\
					 слід встановити права на запис MiKTeX для всіх користувачів до теки, куди його встановлено:$\r$\n\
					 $MiKTeXPath $\r$\n\
					 і всіх підтек цієї теки."
LangString MiKTeXInfo "${LYX_LANG}" 'У LyX використовуватиметься дистрибутив LaTeX "MiKTeX".$\r$\n\
				     Ми рекомендуємо вам встановити всі доступні оновлення MiKTeX за допомогою програми "MiKTeX Update Wizard"$\r$\n\
				     до того, як ви вперше скористаєтеся LyX.$\r$\n\
				     Бажаєте перевірити наявність оновлень MiKTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "Цей пакунок для оновлення призначено лише для ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Не вдалося встановити 'path_prefix' у скрипті налаштування"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Не вдалося створити lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Не вдалося виконати скрипт налаштування"
LangString NotAdmin "${LYX_LANG}" "Для встановлення LyX вам потрібні привілеї адміністратора!"
LangString InstallRunning "${LYX_LANG}" "Засіб для встановлення вже працює!"
LangString StillInstalled "${LYX_LANG}" "LyX вже встановлено! Спочатку вилучіть його LyX."

LangString FinishPageMessage "${LYX_LANG}" "Вітаємо! LyX було успішно встановлено.\r\n\
					    \r\n\
					    (Перший запуск LyX може тривати декілька секунд.)"
LangString FinishPageRun "${LYX_LANG}" "Запустити LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Не вдалося знайти записи LyX у регістрі.$\r$\n\
					       Записи на стільниці і у меню запуску вилучено не буде."
LangString UnInstallRunning "${LYX_LANG}" "Спочатку слід завершити роботу програми LyX!"
LangString UnNotAdminLabel "${LYX_LANG}" "Для вилучення LyX вам слід мати привілеї адміністратора!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Ви справді бажаєте повністю вилучити LyX і всі його компоненти?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Параметри LyX, встановлені користувачем'
LangString UnGSview "${LYX_LANG}" 'Будь ласка, натисніть кнопку "Uninstall" у наступному вікні, щоб вилучити$\r$\n\
				   переглядач Postscript і PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX було успішно вилучено з вашого комп’ютера."

LangString SecUnAspellDescription "${LYX_LANG}" 'Вилучає засіб для перевірки орфографії Aspell разом з усіма його словниками.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Вилучає дистрибутив LaTeX - MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Вилучає засіб для роботи з бібліографічними даними JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Вилучає теку з налаштуваннями LyX$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   для всіх користувачів.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Вилучити LyX і всі його компоненти."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_UKRAINIAN_NSH_
