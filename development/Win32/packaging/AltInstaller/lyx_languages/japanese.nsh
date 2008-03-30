!ifndef _LYX_LANGUAGES_JAPANESE_NSH_
!define _LYX_LANGUAGES_JAPANESE_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_JAPANESE}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "このウィザードが、あなたがLyXをインストールする間のご案内をします。\r\n\
					  \r\n\
					  LyXをインストールするには、管理者権限が必要です。\r\n\
					  \r\n\
					  セットアップを開始する前に、他のすべてのアプリケーションを終了することをお勧めします。そうすることによって、このコンピューターを再起動することなく、関連するシステムファイルを更新することができるようになります。\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "LyX文書"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "すべてのユーザー用にインストールしますか？"
LangString SecFileAssocTitle "${LYX_LANG}" "ファイル関連付け"
LangString SecDesktopTitle "${LYX_LANG}" "デスクトップ・アイコン"

LangString SecCoreDescription "${LYX_LANG}" "LyXのファイル。"
LangString SecInstGSviewDescription "${LYX_LANG}" "ポストスクリプトおよびPDF文書を閲覧するためのプログラム。"
LangString SecInstJabRefDescription "${LYX_LANG}" "参考文献管理およびBibTeXファイルエディタ。"
LangString SecAllUsersDescription "${LYX_LANG}" "LyXをすべてのユーザー用に装備するか、現在のユーザー向けだけに装備するか。"
LangString SecFileAssocDescription "${LYX_LANG}" "拡張子が.lyxのファイルは自動的にLyXで開かれる。"
LangString SecDesktopDescription "${LYX_LANG}" "デスクトップ上のLyXアイコン。"

LangString LangSelectHeader "${LYX_LANG}" "LyXのメニュー言語の選択"
LangString AvailableLang "${LYX_LANG}" " 選択可能な言語 "

LangString MissProgHeader "${LYX_LANG}" "必要なプログラムを確認する"
LangString MissProgCap "${LYX_LANG}" "以下の必要なプログラムが追加的にインストールされます"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX: LaTeXディストリビューションの一つ"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript: ポストスクリプトとPDFのインタープリタ"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick: 画像変換子"
LangString MissProgAspell "${LYX_LANG}" "Aspell: スペルチェッカー"
LangString MissProgMessage "${LYX_LANG}" '追加プログラムをインストールする必要はありません。'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'LaTeXディストリビューション'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'LyXが使うべきLaTeXディストリビューションを指定してください。'
LangString EnterLaTeXFolder "${LYX_LANG}" 'ここでは、"latex.exe"ファイルへのパスを指定することができます。そうすることによって、\
					   LyXが使用すべきLaTeXディストリビューションを指定することができます。\r\n\
					   もしLaTeXを使わなければ、LyXは文書を出力することができません！\r\n\
					   \r\n\
					   インストーラーは、システム上にLaTeXディストリビューションとして\
					   "$LaTeXName"があることを検出しました。下欄にはそのパスが表示されています。'
LangString EnterLaTeXFolderNone "${LYX_LANG}" '下欄で"latex.exe"ファイルへのパスを指定してください。そうすることによって、\
					       LyXがどのLaTeXディストリビューションを使用するべきか指定することができます。\r\n\
					       もしLaTeXを使わなければ、LyXは文書を出力することができません！\r\n\
					   \r\n\
					   インストーラーは、システム上にLaTeXディストリビューションを見つけることができませんでした。'
LangString PathName "${LYX_LANG}" '"latex.exe"ファイルへのパス'
LangString DontUseLaTeX "${LYX_LANG}" "LaTeXを使用しない"
LangString InvalidLaTeXFolder "${LYX_LANG}" '指定されたパスに"latex.exe"ファイルが見つかりません。'

LangString LatexInfo "${LYX_LANG}" 'これからLaTeXディストリビューション"MiKTeX"のインストーラーを起動します。$\r$\n\
			            プログラムをインストールするには、インストールが始まるまでインストーラーのウィンドウにある"Next"ボタンを押してください。$\r$\n\
			            $\r$\n\
			            ！！！すべてMiKTeXインストーラーのデフォルトオプションに従ってください。！！！'
LangString LatexError1 "${LYX_LANG}" 'LaTeXディストリビューションを発見することができませんでした！$\r$\n\
                      		      LyXは、"MiKTeX"のようなLaTeXディストリビューションなしでは使うことができません！$\r$\n\
                      		      したがってインストールは中止します。'
			    
LangString GSviewInfo "${LYX_LANG}" 'これからプログラム"GSview"のインストーラーを起動します。$\r$\n\
			             プログラムをインストールするには、インストーラーの最初のウィンドウで｢Setup｣ボタンを押し、$\r$\n\
			             言語を選択して、インストーラーの次のウィンドウで「Next」ボタンを押してください。$\r$\n\
				     GSviewインストーラーのデフォルトのオプションをすべて受け入れてかまいません。'
LangString GSviewError "${LYX_LANG}" 'プログラム"GSview"は正しくインストールされませんでした！$\r$\n\
		                      インストーラーは、とりあえずこのままインストールを続けます。$\r$\n\
		                      後でGSviewのインストールを再度試みてください。'
				      
LangString JabRefInfo "${LYX_LANG}" 'これからプログラム"JabRef"のインストーラーを起動します。$\r$\n\
				     JabRefインストーラーのデフォルトのオプションをすべて受け入れてかまいません。'
LangString JabRefError "${LYX_LANG}" 'プログラム"JabRef"は正しくインストールされませんでした！$\r$\n\
		                      インストーラーは、とりあえずこのままインストールを続けます。$\r$\n\
		                      後でJabRefのインストールを再度試みてください。'
				      
LangString LatexConfigInfo "${LYX_LANG}" "以下のLyXの設定には少々時間がかかります。"

LangString AspellInfo "${LYX_LANG}" 'これからスペルチェッカー"Aspell"の辞書をダウンロードしてインストールします。$\r$\n\
				     各辞書は異なるライセンスを持っているので、インストールの前に各々表示します。'
LangString AspellDownloadFailed "${LYX_LANG}" "スペルチェッカーAspellの辞書をダウンロードすることができませんでした！"
LangString AspellInstallFailed "${LYX_LANG}" "スペルチェッカーAspellの辞書をインストールすることができませんでした！"
LangString AspellPartAnd "${LYX_LANG}" "及び"
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" '以下の言語の辞書は既にインストールされています:'
LangString AspellNone "${LYX_LANG}" 'スペルチェッカー"Aspell"用の辞書がインストールされていません。辞書は$\r$\n\
				     ${AspellLocation}$\r$\n\
				     からダウンロードすることができます。$\r$\n\
				     辞書をいまダウンロードしますか？'
LangString AspellPartStart "${LYX_LANG}" 'スペルチェッカー"Aspell"用の"'
LangString AspellPart1 "${LYX_LANG}" "英語辞書"
LangString AspellPart2 "${LYX_LANG}" "言語「$LangName」の辞書"
LangString AspellPart3 "${LYX_LANG}" "$\r$\n\
				      言語「$LangNameSys」の辞書"
LangString AspellPart4 "${LYX_LANG}" 'が正しくインストールされています。追加の辞書を$\r$\n\
				     ${AspellLocation}$\r$\n\
				     からダウンロードすることができます。$\r$\n\
				     他の辞書をいまダウンロードしますか？'

LangString MiKTeXPathInfo "${LYX_LANG}" "すべてのユーザーが後で必要に応じてMiKTeXを$\r$\n\
					 カスタマイズできるように、MiKTeXのインストール先フォルダ$\r$\n\
					 $MiKTeXPath $\r$\n\
					 とその下位フォルダの書込み権限を、すべてのユーザーに与える必要があります。"
LangString MiKTeXInfo "${LYX_LANG}" 'LyXとともに、LaTeXディストリビューション"MiKTeX"が用いられます。$\r$\n\
				     LyXを初めて使う前に、"MiKTeX更新ウィザード"プログラムを使って、$\r$\n\
				     利用可能なMiKTeX更新分をインストールことを推奨します。$\r$\n\
				     MiKTeXの更新分があるかどうか、いま確認しますか？'

LangString UpdateNotAllowed "${LYX_LANG}" "このアップデートパッケージは${PRODUCT_VERSION_OLD}だけをアップデートすることができます！"
LangString ModifyingConfigureFailed "${LYX_LANG}" "configureスクリプト中で'path_prefix'を設定することができませんでした"
LangString CreateCmdFilesFailed "${LYX_LANG}" "lyx.batを生成することができませんでした"
LangString RunConfigureFailed "${LYX_LANG}" "configureスクリプトを実行することができませんでした"
LangString NotAdmin "${LYX_LANG}" "LyXをインストールするには、管理者権限を持っていなくてはなりません！"
LangString InstallRunning "${LYX_LANG}" "インストーラーは既に動いています！"
LangString StillInstalled "${LYX_LANG}" "LyXは既にインストールされています！先にLyXをアンインストールしてください。"

LangString FinishPageMessage "${LYX_LANG}" "おめでとうございます！LyXが正しく装備されました。\r\n\
					    \r\n\
					    （初回のLyXの起動には時間がかかります。）"
LangString FinishPageRun "${LYX_LANG}" "LyXを起動する"

LangString UnNotInRegistryLabel "${LYX_LANG}" "レジストリにLyXが見当たりません。$\r$\n\
					       デスクトップとスタートメニューのショートカットは削除されません。"
LangString UnInstallRunning "${LYX_LANG}" "まずLyXを閉じてください！"
LangString UnNotAdminLabel "${LYX_LANG}" "LyXをアンインストールするには、管理者権限を持っていなくてはなりません！"
LangString UnReallyRemoveLabel "${LYX_LANG}" "本当に、LyXとすべての附属コンポーネントを削除してしまう積もりですか？"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'LyXのユーザー設定'
LangString UnGSview "${LYX_LANG}" 'Postscript・PDFビューア"GSview"を脱装するには、$\r$\n\
				   次のウィンドウで"Uninstall"ボタンを押してください。'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyXはお使いのコンピューターから取り除かれました。"

LangString SecUnAspellDescription "${LYX_LANG}" 'スペルチェッカーAspellと附属辞書を脱装します。'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "LaTeXディストリビューションMiKTeXを脱装します。"
LangString SecUnJabRefDescription "${LYX_LANG}" "文献マネージャーJabRefを脱装します。"
LangString SecUnPreferencesDescription "${LYX_LANG}" 'ユーザー共通のLyXの設定フォルダ$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   を削除します。'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "LyXとすべての附属コンポーネントを脱装します。"

!undef LYX_LANG

!endif # _LYX_LANGUAGES_JAPANESE_NSH_
