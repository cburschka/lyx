!insertmacro LANGFILE_EXT "Japanese"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "このウィザードが、あなたがLyXをインストールする間のご案内をします。$\r$\n\
					  $\r$\n\
					  $_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX (MiKTeX may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "LyX文書"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "すべてのユーザー用にインストールしますか？"
${LangFileString} SecFileAssocTitle "ファイル関連付け"
${LangFileString} SecDesktopTitle "デスクトップ・アイコン"

${LangFileString} SecCoreDescription "LyXのファイル。"
${LangFileString} SecInstJabRefDescription "参考文献管理およびBibTeXファイルエディタ。"
${LangFileString} SecAllUsersDescription "LyXをすべてのユーザー用に装備するか、現在のユーザー向けだけに装備するか。"
${LangFileString} SecFileAssocDescription "拡張子が.lyxのファイルは自動的にLyXで開かれる。"
${LangFileString} SecDesktopDescription "デスクトップ上のLyXアイコン。"

${LangFileString} EnterLaTeXHeader1 'LaTeXディストリビューション'
${LangFileString} EnterLaTeXHeader2 'LyXが使うべきLaTeXディストリビューションを指定してください。'
${LangFileString} EnterLaTeXFolder 'ここでは、$\"latex.exe$\"ファイルへのパスを指定することができます。そうすることによって、\
					   LyXが使用すべきLaTeXディストリビューションを指定することができます。$\r$\n\
					   もしLaTeXを使わなければ、LyXは文書を出力することができません！$\r$\n\
					   $\r$\n\
					   インストーラーは、システム上にLaTeXディストリビューションとして\
					   $\"$LaTeXName$\"があることを検出しました。下欄にはそのパスが表示されています。'
${LangFileString} EnterLaTeXFolderNone '下欄で$\"latex.exe$\"ファイルへのパスを指定してください。そうすることによって、\
					       LyXがどのLaTeXディストリビューションを使用するべきか指定することができます。$\r$\n\
					       もしLaTeXを使わなければ、LyXは文書を出力することができません！$\r$\n\
					   $\r$\n\
					   インストーラーは、システム上にLaTeXディストリビューションを見つけることができませんでした。'
${LangFileString} PathName '$\"latex.exe$\"ファイルへのパス'
${LangFileString} DontUseLaTeX "LaTeXを使用しない"
${LangFileString} InvalidLaTeXFolder '指定されたパスに$\"latex.exe$\"ファイルが見つかりません。'

${LangFileString} LatexInfo 'これからLaTeXディストリビューション$\"MiKTeX$\"のインストーラーを起動します。$\r$\n\
			            プログラムをインストールするには、インストールが始まるまでインストーラーのウィンドウにある$\"Next$\"ボタンを押してください。$\r$\n\
			            $\r$\n\
			            ！！！すべてMiKTeXインストーラーのデフォルトオプションに従ってください。！！！'
${LangFileString} LatexError1 'LaTeXディストリビューションを発見することができませんでした！$\r$\n\
                      		      LyXは、$\"MiKTeX$\"のようなLaTeXディストリビューションなしでは使うことができません！$\r$\n\
                      		      したがってインストールは中止します。'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'これからプログラム$\"JabRef$\"のインストーラーを起動します。$\r$\n\
				     JabRefインストーラーのデフォルトのオプションをすべて受け入れてかまいません。'
${LangFileString} JabRefError 'プログラム$\"JabRef$\"は正しくインストールされませんでした！$\r$\n\
		                      インストーラーは、とりあえずこのままインストールを続けます。$\r$\n\
		                      後でJabRefのインストールを再度試みてください。'

${LangFileString} LatexConfigInfo "以下のLyXの設定には少々時間がかかります。"

${LangFileString} MiKTeXPathInfo "すべてのユーザーが後で必要に応じてMiKTeXを$\r$\n\
					 カスタマイズできるように、MiKTeXのインストール先フォルダ$\r$\n\
					 $MiKTeXPath $\r$\n\
					 とその下位フォルダの書込み権限を、すべてのユーザーに与える必要があります。"
${LangFileString} MiKTeXInfo 'LyXとともに、LaTeXディストリビューション$\"MiKTeX$\"が用いられます。$\r$\n\
				     LyXを初めて使う前に、$\"MiKTeX更新ウィザード$\"プログラムを使って、$\r$\n\
				     利用可能なMiKTeX更新分をインストールことを推奨します。$\r$\n\
				     MiKTeXの更新分があるかどうか、いま確認しますか？'

${LangFileString} ModifyingConfigureFailed "configureスクリプト中で'path_prefix'を設定することができませんでした"
${LangFileString} RunConfigureFailed "configureスクリプトを実行することができませんでした"
${LangFileString} NotAdmin "LyXをインストールするには、管理者権限を持っていなくてはなりません！"
${LangFileString} InstallRunning "インストーラーは既に動いています！"
${LangFileString} StillInstalled "LyXは既にインストールされています！先にLyXをアンインストールしてください。"

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
					       MiKTeX was installed with administrator privileges but you are installing LyX without them."
${LangFileString} MetafileNotAvailable "The LyX support for images in the format EMF or WMF will not be$\r$\n\
					available because that requires to install a software printer to$\r$\n\
					Windows which is only possible with administrator privileges."

${LangFileString} FinishPageMessage "おめでとうございます！LyXが正しく装備されました。$\r$\n\
					    $\r$\n\
					    （初回のLyXの起動には時間がかかります。）"
${LangFileString} FinishPageRun "LyXを起動する"

${LangFileString} UnNotInRegistryLabel "レジストリにLyXが見当たりません。$\r$\n\
					       デスクトップとスタートメニューのショートカットは削除されません。"
${LangFileString} UnInstallRunning "まずLyXを閉じてください！"
${LangFileString} UnNotAdminLabel "LyXをアンインストールするには、管理者権限を持っていなくてはなりません！"
${LangFileString} UnReallyRemoveLabel "本当に、LyXとすべての附属コンポーネントを削除してしまう積もりですか？"
${LangFileString} UnLyXPreferencesTitle 'LyXのユーザー設定'

${LangFileString} SecUnMiKTeXDescription "LaTeXディストリビューションMiKTeXを脱装します。"
${LangFileString} SecUnJabRefDescription "文献マネージャーJabRefを脱装します。"
${LangFileString} SecUnPreferencesDescription 'ユーザー共通のLyXの設定フォルダ$\r$\n\
					   $\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
					   を削除します。'
${LangFileString} SecUnProgramFilesDescription "LyXとすべての附属コンポーネントを脱装します。"

