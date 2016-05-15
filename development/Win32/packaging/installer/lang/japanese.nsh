/*
LyX Installer Language File
Language: Japanese
Author: Koji Yokata
*/

!insertmacro LANGFILE_EXT "Japanese"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(現ユーザー用に導入を行う)"

${LangFileString} TEXT_WELCOME "このウィザードが、あなたのLyX導入作業中のご案内をします。$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "MiKTeXのファイル名データベースを更新しています..."
${LangFileString} TEXT_CONFIGURE_LYX "LyXを構成しています ($LaTeXInstalledが不足分のパッケージをダウンロードするかもしれないので、時間のかかることがあります) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Pythonスクリプトをコンパイルしています..."

${LangFileString} TEXT_FINISH_DESKTOP "デスクトップにショートカットを作成する"
${LangFileString} TEXT_FINISH_WEBSITE "lyx.orgを開いて最新ニュースやサポート、ヒントなどを入手する"

${LangFileString} FileTypeTitle "LyX文書"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "すべてのユーザー用に導入を行いますか？"
${LangFileString} SecFileAssocTitle "ファイル関連付け"
${LangFileString} SecDesktopTitle "デスクトップ・アイコン"

${LangFileString} SecCoreDescription "LyXのファイル。"
${LangFileString} SecInstJabRefDescription "参考文献管理およびBibTeXファイルエディタ。"
${LangFileString} SecAllUsersDescription "LyXをすべてのユーザー用に導入するか、現在のユーザー向けだけに導入するか。"
${LangFileString} SecFileAssocDescription "拡張子が.lyxのファイルは自動的にLyXで開かれる。"
${LangFileString} SecDesktopDescription "デスクトップ上のLyXアイコン"
${LangFileString} SecDictionariesDescription "ダウンロード及び導入が可能なスペルチェック用辞書"
${LangFileString} SecThesaurusDescription "ダウンロード及び導入が可能な類義語辞典用辞書"

${LangFileString} EnterLaTeXHeader1 'LaTeX頒布版'
${LangFileString} EnterLaTeXHeader2 'LyXが使うべきLaTeX頒布版を指定してください。'
${LangFileString} EnterLaTeXFolder 'ここでは、$\"latex.exe$\"ファイルへのパスを指定することができます。そうすることによって、\
					LyXが使用すべきLaTeX頒布版を指定することができます。\r\n\
					もしLaTeXを使わなければ、LyXは文書を出力することができません！\r\n\
					\r\n\
					導入プログラムは、システム上にLaTeX頒布版として\
					$\"$LaTeXName$\"があることを検出しました。下欄にはそのパスが表示されています。'
${LangFileString} EnterLaTeXFolderNone '下欄で$\"latex.exe$\"ファイルへのパスを指定してください。そうすることによって、\
					LyXがどのLaTeX頒布版を使用するべきか指定することができます。\r\n\
					もしLaTeXを使わなければ、LyXは文書を出力することができません！\r\n\
					\r\n\
					導入プログラムは、システム上にLaTeX頒布版を見つけることができませんでした。'
${LangFileString} PathName '$\"latex.exe$\"ファイルへのパス'
${LangFileString} DontUseLaTeX "LaTeXを使用しない"
${LangFileString} InvalidLaTeXFolder '指定されたパスに$\"latex.exe$\"ファイルが見つかりません。'

${LangFileString} LatexInfo 'これからLaTeX頒布版$\"MiKTeX$\"の導入プログラムを起動します。$\r$\n\
				プログラムを導入するには、導入作業が始まるまで導入プログラムのウィンドウにある$\"Next$\"ボタンを押してください。$\r$\n\
				$\r$\n\
				！！！すべてMiKTeX導入プログラムの既定オプションに従ってください。！！！'
${LangFileString} LatexError1 'LaTeX頒布版を発見することができませんでした！$\r$\n\
				LyXは、$\"MiKTeX$\"のようなLaTeX頒布版なしでは使うことができません！$\r$\n\
				したがって導入を中止します。'

${LangFileString} HunspellFailed '言語$\"$R3$\"用辞書のダウンロードに失敗しました。'
${LangFileString} ThesaurusFailed '言語$\"$R3$\"用類義語辞典のダウンロードに失敗しました。'

${LangFileString} JabRefInfo 'これからプログラム$\"JabRef$\"の導入プログラムを起動します。$\r$\n\
				JabRef導入プログラムの既定オプションをすべて受け入れてかまいません。'
${LangFileString} JabRefError 'プログラム$\"JabRef$\"は正しく導入できませんでした！$\r$\n\
				導入プログラムは、とりあえずこのまま導入作業を続けます。$\r$\n\
				後でJabRefの導入を再度試みてください。'

${LangFileString} LatexConfigInfo "以下のLyXの設定には少々時間がかかります。"

${LangFileString} MiKTeXPathInfo "すべてのユーザーが後で必要に応じてMiKTeXを$\r$\n\
					カスタマイズできるように、MiKTeXの導入先フォルダ$\r$\n\
					$MiKTeXPath $\r$\n\
					とその下位フォルダの書込み権限を、すべてのユーザーに与える必要があります。"
${LangFileString} MiKTeXInfo 'LyXとともに、LaTeX頒布版$\"MiKTeX$\"が用いられます。$\r$\n\
				     LyXを初めて使う前に、$\"MiKTeX更新ウィザード$\"プログラムを使って、$\r$\n\
				     利用可能なMiKTeX更新分を導入することを推奨します。$\r$\n\
				     MiKTeXの更新分があるかどうか、いま確認しますか？'

${LangFileString} ModifyingConfigureFailed "configureスクリプト中で'path_prefix'を設定することができませんでした"
${LangFileString} RunConfigureFailed "configureスクリプトを実行することができませんでした"
${LangFileString} NotAdmin "LyXを導入するには、管理者権限を持っていなくてはなりません！"
${LangFileString} InstallRunning "導入プログラムは既に動作中です！"
${LangFileString} AlreadyInstalled "LyX${APP_SERIES_KEY2}は既に導入済みです！$\r$\n\
				導入済みのバージョンがテスト版であったり、導入済みLyXで問題がある場合には、$\r$\n\
				上書き導入作業は推奨されません。これらの場合には、LyXを最初から再導入する$\r$\n\
				ことが推奨されます。$\r$\n\
				これらを承知の上で、既存のLyXを上書きしますか？"
${LangFileString} NewerInstalled "あなたは、既に導入済みのLyXよりも古い版を導入しようとしています。$\r$\n\
				  本当にそうしたいのであれば、既存の LyX $OldVersionNumber をまず導入解除してください。"

${LangFileString} MultipleIndexesNotAvailable "MiKTeXが管理者権限で導入されているのに、LyXをMiKTeXなしで導入しようとしているため、$\r$\n\
						同一文書内での複数の索引の使用はサポートされません。"
${LangFileString} MetafileNotAvailable "LyXにおいて画像形式EMFおよびWMFを使う方法は、$\r$\n\
					Windowsにソフトウェアプリンタを管理者権限で導入する必要があるため、$\r$\n\
					サポートできません。"

${LangFileString} FinishPageMessage "おめでとうございます！LyXが正しく導入されました。$\r$\n\
					$\r$\n\
					初回のLyXの起動には時間がかかります。）"
${LangFileString} FinishPageRun "LyXを起動する"

${LangFileString} UnNotInRegistryLabel "レジストリにLyXが見当たりません。$\r$\n\
					デスクトップとスタートメニューのショートカットは削除されません。"
${LangFileString} UnInstallRunning "まずLyXを閉じてください！"
${LangFileString} UnNotAdminLabel "LyXの導入解除を行うには、管理者権限を持っていなくてはなりません！"
${LangFileString} UnReallyRemoveLabel "本当に、LyXとすべての附属コンポーネントを削除してしまう積もりですか？"
${LangFileString} UnLyXPreferencesTitle 'LyXのユーザー設定'

${LangFileString} SecUnMiKTeXDescription "LaTeX頒布版MiKTeXの導入解除を行います。"
${LangFileString} SecUnJabRefDescription "文献管理プログラムJabRefの導入解除を行います。"
${LangFileString} SecUnPreferencesDescription 'ユーザー共通のLyXの設定フォルダ$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						を削除します。'
${LangFileString} SecUnProgramFilesDescription "LyXとすべての附属コンポーネントの導入解除を行います。"

