/*
LyX Installer Language File
Language: Arabic
Author: Hatim Alahmady
*/

!insertmacro LANGFILE_EXT "Arabic"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(تركيب للمستخدم الحالي)"

${LangFileString} TEXT_WELCOME "هذا المساعد سوف يرشدك خلال تركيب $(^NameDA), $\r$\n\
				$\r$\n\
				$_CLICK"

#${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "تحديث قاعدة بيانات MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "ضبط ليك ($LaTeXInstalled ربما ينزل الحزم المفقودة, وهذا سيستغرق بعض الوقت) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "بناء سكربتات بايثون..."

${LangFileString} TEXT_FINISH_DESKTOP "إنشاء اختصار سطح المكتب"
${LangFileString} TEXT_FINISH_WEBSITE "زيارة lyx.org لمشاهدة آخر الاخبار, الدعم والأفكار"

#${LangFileString} FileTypeTitle "مستند - ليك"

#${LangFileString} SecInstJabRefTitle "JabRef"
#${LangFileString} SecAllUsersTitle "تركيب لكل المستخدمين؟"
${LangFileString} SecFileAssocTitle "اقتران الملف"
${LangFileString} SecDesktopTitle "رمز سطح المكتب"

${LangFileString} SecCoreDescription "ملفات ليك."
#${LangFileString} SecInstJabRefDescription "إدارة ثبت المراجع ومحرر ملفات bibtex."
#${LangFileString} SecAllUsersDescription "تركيب ليك لهذا المستخدم أم لجميع المستخدمين."
${LangFileString} SecFileAssocDescription "الملفات بلاحقة .lyx سوف تفتح تلفائيا ببرنامج ليك."
${LangFileString} SecDesktopDescription "رمز ليم على سطح المكتب."
${LangFileString} SecDictionaries "قواميس"
${LangFileString} SecDictionariesDescription "قواميس المدقق الإملائي التي يمكن تنزيلها وتركيبها."
${LangFileString} SecThesaurus "مكنز"
${LangFileString} SecThesaurusDescription "قواميس المفردات التي يمكن تنزيلها وتركيبها."

${LangFileString} EnterLaTeXHeader1 'توزيعة - لتيك'
${LangFileString} EnterLaTeXHeader2 'تعيين توزيعة لتيك التي يجب على ليك استخدامها.'
${LangFileString} EnterLaTeXFolder 'تستطيع هنا تعيين مسار الملف $\"latex.exe$\" وبعد ذلك تعيين \
					توزيعة لتيك التي يجب استخدامها مع ليك.\r\n\
					إذا لم تستخدم لتيك, ليك لن يستطيع استخراج المستندات!\r\n\
					\r\n\
					برنامج التركيب يفحص توزيعة لتيك \
					$\"$LaTeXName$\" في نظامك. ويعرض مسارها في الاسفل.'
${LangFileString} EnterLaTeXFolderNone 'عين في الاسفل مسار الملف $\"latex.exe$\". وبعد ذلك عين \
					توزيعة لتيك التي ستستخدم مع ليك.\r\n\
					إذا لم تستخدم لتيك, ليك لن يستطيع استخراج المستندات!\r\n\
					\r\n\
					برنامج التركيب لم يعثر على توزيعة لتيك في نظامك.'
${LangFileString} PathName 'مسار الملف $\"latex.exe$\"'
${LangFileString} DontUseLaTeX "لا تستخجم لتيك"
${LangFileString} InvalidLaTeXFolder 'الملف $\"latex.exe$\" ليس في المسار المحدد.'

${LangFileString} LatexInfo 'برنامج تركيب توزيعة لتيك $\"MiKTeX$\" سوف يبدأ الآن.$\r$\n\
				في تركيب البرنامج فضلا انقر زر $\"Next$\"  في نافذة التركيب عندما تبدأ.$\r$\n\
				$\r$\n\
				!!! فضلا إختر كل الخيارات الافتراضية في MiKTeX-installer !!!'
${LangFileString} LatexError1 'لم يتم العثور على توزيعة لتيك!$\r$\n\
				لا يمكن استخدام ليك بدون وجود توزيعة لتيك مثل $\"MiKTeX$\"!$\r$\n\
				وسوف تلغى عملية التركيب.'

${LangFileString} HunspellFailed 'فشل تنزيل قاموس اللغة $\"$R3$\" .'
${LangFileString} ThesaurusFailed 'فشل تنزيل قاموس مفردات اللغة $\"$R3$\" .'

#${LangFileString} JabRefInfo 'برنامج تركيب البرنامج $\"JabRef$\" سيبدأ.$\r$\n\
#				يمكنك إختيار جميع الخيارات الافتراضية في JabRef-installer.'
#${LangFileString} JabRefError 'البرنامج $\"JabRef$\" لم ينجح تركيبه!$\r$\n\
#				إستمرار التركيب على أي حال.$\r$\n\
#				حاول تركيب JabRef لاحقا.'

#${LangFileString} LatexConfigInfo "ضبط ليك سيستغرق وفت."

#${LangFileString} MiKTeXPathInfo "من أجل تخصيص MiKTeX لكل مستخدم يجب$\r$\n\
#					يجب إعطاء صلاحية الكتابة لكل المستخدمين في مجلد تركيب MiKTeX$\'s  $\r$\n\
#					$MiKTeXPath $\r$\n\
#					والمجلدات الفرعية."
${LangFileString} MiKTeXInfo 'توزيعة لتيك $\"MiKTeX$\" توزيعة ليك سوف تستخدم أيضا مع ليك.$\r$\n\
				يفضل تركيب تحديث مايك تيكس باستخدام $\"MiKTeX Update Wizard$\"$\r$\n\
				قبل استخدام ليك في أول مرة.$\r$\n\
				هل ترغب بفحص MiKTeX updates?'

${LangFileString} ModifyingConfigureFailed "لم تعين 'path_prefix' في سكريبت الضبط"
#${LangFileString} RunConfigureFailed "لم ينفذ سكريبت الضبط"
${LangFileString} InstallRunning "المركب يعمل حاليا!"
${LangFileString} AlreadyInstalled "ليك ${APP_SERIES_KEY2} تم تركيبه بالفعل!$\r$\n\
				التركيب على النسخة الحالية غير مفضل إذا كانت النسخة الحالية$\r$\n\
				تجريبية أو بها مشاكل.$\r$\n\
				في هذه الحالة من الأفضل إعادة التركيب.$\r$\n\
				هل تريد بالرغم من ذلك تركيب ليك على النسخة الحالية؟"
${LangFileString} NewerInstalled "تحاول تركيب نسخة ليك أقدم من الموجودة حاليا.$\r$\n\
				  إذا كنت تريدها بالتأكيد, عليك حذف النسخة الحالية $OldVersionNumber أولا."

${LangFileString} MultipleIndexesNotAvailable "دعم الفهارس المتعددة في المستند لن تكون متوفرة لأن $\r$\n\
						MiKTeX تم تركيبه بصلاحيات المدير بينما ليك تم تركيبه بدونها."

#${LangFileString} FinishPageMessage "مبروك! تم تركيب ليك بنجاح.$\r$\n\
#					$\r$\n\
#					(البدء الأول لليك ربما يستغرق ثوان.)"
${LangFileString} FinishPageRun "بدء ليك"

${LangFileString} UnNotInRegistryLabel "لم يتم العثور على ليك في سجل النظام.$\r$\n\
					إختصارات سطح المكتب وقائمة البدء لم يتم حذفها."
${LangFileString} UnInstallRunning "يجب إغلاق ليك أولا!"
${LangFileString} UnNotAdminLabel "يجب أن يكون لديك صلاحيات المدير لكي تحذف ليك!"
${LangFileString} UnReallyRemoveLabel "هل ترغب بإزالة ليك مع كل مكوناته؟"
${LangFileString} UnLyXPreferencesTitle 'تفضيلات مستخدم ليك'

${LangFileString} SecUnMiKTeXDescription "إزالة توزيعة لتيك MiKTeX."
#${LangFileString} SecUnJabRefDescription "إزالة مدير ثبت المراجع JabRef."
${LangFileString} SecUnPreferencesDescription 'حذف LyX$\'s ضبط$\r$\n\
						(مجلد $\"$AppPre\username\$\r$\n\
						$AppSuff\$\r$\n\
						${APP_DIR_USERDATA}$\")$\r$\n\
						لك او لكل المستخدمين (إذا كنت المدير).'
${LangFileString} SecUnProgramFilesDescription "إزالة ليك مع كل مكوناته."
