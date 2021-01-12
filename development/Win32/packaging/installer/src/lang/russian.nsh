/*
LyX Installer Language File
Language: Russian
Authors: Yuriy Skalko
*/

!insertmacro LANGFILE_EXT "Russian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Установлено для текущего пользователя)"

${LangFileString} TEXT_WELCOME "Этот мастер проведет вас через процесс установки $(^NameDA). $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Обновление базы имен файлов MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Конфигурирование LyX ($R9 может скачать недостающие пакеты, это может занять некоторое время) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Компиляция скриптов Python..."

${LangFileString} TEXT_FINISH_DESKTOP "Создать ярлык на рабочем столе"
${LangFileString} TEXT_FINISH_WEBSITE "Перейти на lyx.org за новостями, поддержкой и советами"

#${LangFileString} FileTypeTitle "LyX-Document"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "Установить для всех пользователей?"
${LangFileString} SecFileAssocTitle "Ассоциации файлов"
${LangFileString} SecDesktopTitle "Значок на рабочем столе"

${LangFileString} SecCoreDescription "Файлы LyX."
#${LangFileString} SecInstJabRefDescription "Менеджер библиографических ссылок и редактор для файлов BibTeX."
#${LangFileString} SecAllUsersDescription "Установить LyX для всех пользователей или только для текущего пользователя."
${LangFileString} SecFileAssocDescription "Файлы с расширением .lyx будут автоматически открываться в LyX."
${LangFileString} SecDesktopDescription "Значок LyX на рабочем столе."
${LangFileString} SecDictionaries "Словари"
${LangFileString} SecDictionariesDescription "Словари для проверки орфографии, которые можно скачать и установить."
${LangFileString} SecThesaurus "Тезаурус"
${LangFileString} SecThesaurusDescription "Словари тезауруса, которые можно скачать и установить."

${LangFileString} EnterLaTeXHeader1 'Дистрибутив LaTeX'
${LangFileString} EnterLaTeXHeader2 'Укажите дистрибутив LaTeX, который будет использоваться LyX.'
${LangFileString} EnterLaTeXFolder 'При необходимости здесь можно задать путь к файлу $\"latex.exe$\" и тем самым \
					указать используемый дистрибутив LaTeX.$\r$\n\
					Если вы не используете LaTeX, то LyX не сможет выводить документы!$\r$\n\
					$\r$\n\
					Установщик обнаружил дистрибутив LaTeX \
					$\"$R9$\" в вашей системе. Ниже показан его путь.'
${LangFileString} EnterLaTeXFolderNone 'Задайте ниже путь к файлу $\"latex.exe$\". Этим вы укажете, какой \
					дистрибутив LaTeX будет использоваться LyX.$\r$\n\
					Если вы не используете LaTeX, то LyX не сможет выводить документы!$\r$\n\
					$\r$\n\
					Установщику не удалось найти дистрибутив LaTeX в вашей системе.'
${LangFileString} PathName 'Путь к файлу $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "Не использовать LaTeX"
${LangFileString} InvalidLaTeXFolder 'Файл $\"latex.exe$\" отсутствует по этому пути.'

${LangFileString} LatexInfo 'Теперь будет запущен установщик дистрибутива LaTeX $\"MiKTeX$\".$\r$\n\
				Для его установки нажимайте кнопку $\"Далее$\" в окнах установщика до начала установки.$\r$\n\
				$\r$\n\
				!!! Пожалуйста, используйте все опции по умолчанию для установщика MiKTeX !!!'
${LangFileString} LatexError1 'Дистрибутив LaTeX не найден!$\r$\n\
				LyX не может быть использован без дистрибутива LaTeX, такого как $\"MiKTeX$\"!$\r$\n\
				Поэтому установка будет прервана.'

${LangFileString} HunspellFailed 'Не удалось загрузить словарь для языка $\"$R9$\".'
${LangFileString} ThesaurusFailed 'Не удалось загрузить тезаурус для языка $\"$R9$\".'

#${LangFileString} JabRefInfo 'Теперь будет запущен установщик программы $\"JabRef$\".$\r$\n\
#				Вы можете использовать все параметры по умолчанию установщика JabRef.'
#${LangFileString} JabRefError 'Программа $\"JabRef$\" не может быть успешно установлена!$\r$\n\
#				Программа установки все равно продолжит работу.$\r$\n\
#				Попробуйте установить JabRef еще раз позже.'

#${LangFileString} LatexConfigInfo "Следующая конфигурация LyX займет некоторое время."

#${LangFileString} MiKTeXPathInfo "Для того, чтобы каждый пользователь имел возможность настроить MiKTeX для своих нужд$\r$\n\
#					необходимо установить права на запись для всех пользователей каталога установки MiKTeX$\r$\n\
#					$MiKTeXPath $\r$\n\
#					и его подкаталогов."
${LangFileString} MiKTeXInfo 'Дистрибутив LaTeX $\"MiKTeX$\" будет использоваться совместно с LyX.$\r$\n\
				Рекомендуется установить доступные обновления MiKTeX с помощью программы $\"MiKTeX Update Wizard$\"$\r$\n\
				перед первым использованием LyX.$\r$\n\
				Проверить наличие обновлений MiKTeX сейчас?'

${LangFileString} ModifyingConfigureFailed "Не удалось установить 'path_prefix' в сценарии настройки"
#${LangFileString} RunConfigureFailed "Не удалось выполнить сценарий настройки"
${LangFileString} InstallRunning "Установщик уже запущен!"
${LangFileString} AlreadyInstalled "LyX ${APP_VERSION} уже установлен!$\r$\n\
				Установка поверх существующих установок не рекомендуется, если установленная версия$\r$\n\
				является тестовым выпуском или у вас возникли проблемы с существующей установкой LyX.$\r$\n\
				В этих случаях лучше переустановить LyX.$\r$\n\
				Вы все равно хотите установить LyX поверх существующей версии?"
${LangFileString} NewerInstalled "Вы пытаетесь установить более старую версию LyX, чем уже установленная.$\r$\n\
				  Если вы действительно хотите этого, то сначала необходимо удалить существующий LyX $R9."

${LangFileString} MultipleIndexesNotAvailable "Поддержка использования нескольких индексов в документе будет недоступна, поскольку$\r$\n\
						MiKTeX был установлен с правами администратора, а LyX вы устанавливаете без них."

#${LangFileString} FinishPageMessage "Поздравляем! LyX был успешно установлен.$\r$\n\
#					$\r$\n\
#					(Первый запуск LyX может занять несколько секунд.)"
${LangFileString} FinishPageRun "Запустить LyX"

${LangFileString} UnNotInRegistryLabel "Не удалось найти LyX в реестре.$\r$\n\
					Ярлыки на рабочем столе и в меню Пуск не будут удалены."
${LangFileString} UnInstallRunning "Вы должны сначала закрыть LyX!"
${LangFileString} UnNotAdminLabel "Необходимо иметь права администратора для удаления LyX!"
${LangFileString} UnReallyRemoveLabel "Вы действительно хотите полностью удалить LyX и все его компоненты?"
${LangFileString} UnLyXPreferencesTitle 'Пользовательские настройки LyX'

${LangFileString} SecUnMiKTeXDescription "Удалить дистрибутив MiKTeX."
#${LangFileString} SecUnJabRefDescription "Удалить менеджер библиографических ссылок JabRef."
${LangFileString} SecUnPreferencesDescription 'Удалить настройки LyX$\r$\n\
						(каталог $\"$R9\username\$\r$\n\
						$R8\$\r$\n\
						LyX${VERSION_MAJOR}.${VERSION_MINOR}$\")$\r$\n\
						для вас или для всех пользователей (если вы администратор).'
${LangFileString} SecUnProgramFilesDescription "Удалить LyX и все его компоненты."

# MikTeX Update Stuff
${LangFileString} MiKTeXUpdateMsg "Из-за изменений в MiKTeX установка версий LyX позже 2.2.3 может привести к частичному обновлению, что делает MiKTeX непригодным.$\r$\n\
$\r$\n\
Если вы еще этого не сделали, мы рекомендуем вам обновить MiKTeX до последней версии до установки новой версии LyX.$\r$\n\
$\r$\n\
Вы можете либо продолжить установку, либо отменить ее, чтобы обновить MiKTeX.$\r$\n\
$\r$\n\
(Извините за автоматизированный перевод.)"

${LangFileString} MiKTeXUpdateHeader "ПРЕДУПРЕЖДЕНИЕ: Изменения в MiKTeX"
