!insertmacro LANGFILE_EXT "Indonesian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Installed for Current User)"

${LangFileString} TEXT_WELCOME "Program ini akan memandu anda dalam melakukan instalasi LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Updating MiKTeX filename database..."
${LangFileString} TEXT_CONFIGURE_LYX "Configuring LyX ($LaTeXInstalled may download missing packages, this can take some time) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Compiling Python scripts..."

${LangFileString} TEXT_FINISH_DESKTOP "Create desktop shortcut"
${LangFileString} TEXT_FINISH_WEBSITE "Visit lyx.org for the latest news, support and tips"

${LangFileString} FileTypeTitle "Dokumen-LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Instal untuk semua pengguna?"
${LangFileString} SecFileAssocTitle "Berkas yang terkait"
${LangFileString} SecDesktopTitle "Ikon Desktop"

${LangFileString} SecCoreDescription "Berkas-berkas LyX."
${LangFileString} SecInstJabRefDescription "Program manajemen acuan bibliografi dan penyunting berkas BibTeX."
${LangFileString} SecAllUsersDescription "Instal LyX untuk semua pengguna atau hanya untuk pengguna ini saja."
${LangFileString} SecFileAssocDescription "Berkas dengan ekstensi .lyx akan otomatis dibuka menggunakan LyX."
${LangFileString} SecDesktopDescription "Ikon LyX muncul di desktop."
${LangFileString} SecDictionariesDescription "Spell-checker dictionaries that can be downloaded and installed."
${LangFileString} SecThesaurusDescription "Thesaurus dictionaries that can be downloaded and installed."

${LangFileString} EnterLaTeXHeader1 'Distribusi LaTeX'
${LangFileString} EnterLaTeXHeader2 'Pengaturan distribusi LaTeX yang akan digunakan dalam LyX.'
${LangFileString} EnterLaTeXFolder 'Anda dapat mengatur sendiri lokasi tempat berkas $\"latex.exe$\" berada dan mengatur lokasi \
					tempat distribusi LaTeX yang akan digunakan oleh LyX.\r\n\
					Apabila anda tidak menggunakan LaTeX, LyX tidak akan menampilkan output dokumen!\r\n\
					\r\n\
					Program instalasi ini mendeteksi adanya distribusi LaTeX \
					$\"$LaTeXName$\" dalam sistem anda. Dibawah ini adalah lokasi keberadaannya.'
${LangFileString} EnterLaTeXFolderNone 'Aturlah lokasi tempat berkas $\"latex.exe$\" berada. Atur dan nyatakanlah lokasi \
					tempat distribusi LaTeX yang akan digunakan oleh LyX.\r\n\
					Apabila anda tidak menggunakan LaTeX, LyX tidak akan menampilkan output dokumen!\r\n\
					\r\n\
					Program instalasi ini tidak menemukan adanya distribusi LaTeX di sistem anda.'
${LangFileString} PathName 'Lokasi tempat berkas $\"latex.exe$\" berada'
${LangFileString} DontUseLaTeX "Tidak menggunakan LaTeX"
${LangFileString} InvalidLaTeXFolder 'Berkas $\"latex.exe$\" tidak berada di lokasi tempat yang dinyatakan.'

${LangFileString} LatexInfo 'Sekarang instalasi program distribusi LaTeX $\"MiKTeX$\" akan dijalankan.$\r$\n\
				Untuk meneruskan tekanlah tombol $\"Lanjut$\" yang ada di jendela dan tunggu sampai instalasi dimulai.$\r$\n\
				$\r$\n\
				!!! Gunakan semua pilihan default pada program instalasi MiKTeX !!!'
${LangFileString} LatexError1 'Distribusi LaTeX tidak ditemukan!$\r$\n\
 				LyX tidak bisa digunakan tanpa menggunakan salah satu distribusi LaTeX seperti $\"MiKTeX$\"!$\r$\n\
				Oleh karena itu proses instalasi dibatalkan.'

${LangFileString} HunspellFailed 'Download of dictionary for language $\"$R3$\" failed.'
${LangFileString} ThesaurusFailed 'Download of thesaurus for language $\"$R3$\" failed.'

${LangFileString} JabRefInfo 'Sekarang instalasi program $\"JabRef$\" akan dijalankan.$\r$\n\
				Anda boleh memilih semua pilihan default yang ada di program instalasi JabRef.'
${LangFileString} JabRefError 'Program $\"JabRef$\" tidak berhasil diinstal secara keseluruhan!$\r$\n\
				Namun proses instalasi dapat diteruskan.$\r$\n\
				Anda dapat mengulang instalasi JabRef nanti.'

${LangFileString} LatexConfigInfo "Proses konfigurasi LyX selanjutnya akan memerlukan waktu beberapa saat."

${LangFileString} MiKTeXPathInfo "Agar memungkinkan semua pengguna dapat mengatur MiKTeX sesuai keinginannya$\r$\n\
					anda perlu memberikan hak merubah untuk semua pengguna pada lokasi instalasi MiKTeX yaitu folder$\r$\n\
					$MiKTeXPath $\r$\n\
					serta di semua subfoldernya."
${LangFileString} MiKTeXInfo 'Distribusi LaTeX $\"MiKTeX$\" akan digunakan dengan LyX.$\r$\n\
				Sangat disarankan memperbarui program yang ada di MiKTeX menggunakan $\"MiKTeX Update Wizard$\"$\r$\n\
				sebelum anda menggunakan LyX untuk yang pertama kali.$\r$\n\
				Apakah anda akan memeriksa perlunya memperbarui MikTeX?'

${LangFileString} ModifyingConfigureFailed "Tidak bisa menyatakan 'path_prefix' pada skrip configure"
${LangFileString} RunConfigureFailed "Tidak bisa menjalankan skrip configure"
${LangFileString} NotAdmin "Anda harus berlaku dan mempunyai hak sebagai administrator untuk instalasi LyX!"
${LangFileString} InstallRunning "Program instalasi sedang berjalan!"
${LangFileString} StillInstalled "LyX ${APP_SERIES_KEY2} sudah pernah diinstal! Perlu menghapus LyX yang ada terlebih dahulu."
${LangFileString} NewerInstalled "You are trying to install an older version of LyX than what you have installed.$\r$\n\
				  If you really want this, you must uninstall the existing LyX $OldVersionNumber before."

${LangFileString} MultipleIndexesNotAvailable "The support for using several indexes in a document will not be available because$\r$\n\
						MiKTeX was installed with administrator privileges but you are installing LyX without them."
${LangFileString} MetafileNotAvailable "The LyX support for images in the format EMF or WMF will not be$\r$\n\
					available because that requires to install a software printer to$\r$\n\
					Windows which is only possible with administrator privileges."

${LangFileString} FinishPageMessage "Selamat! LyX berhasil diinstal dengan sukses.$\r$\n\
					$\r$\n\
					(Menjalankan LyX untuk pertama kali memerlukan waktu beberapa detik.)"
${LangFileString} FinishPageRun "Menjalankan LyX"

${LangFileString} UnNotInRegistryLabel "Tidak bisa menemukan LyX di catatan registry.$\r$\n\
					Shortcuts pada desktop dan yang ada di Menu Start tidak dihapus."
${LangFileString} UnInstallRunning "Anda harus menutup LyX terlebih dahulu!"
${LangFileString} UnNotAdminLabel "Anda harus berlaku dan mempunyai hak sebagai administrator untuk menghapus LyX!"
${LangFileString} UnReallyRemoveLabel "Apakah anda yakin akan menghapus LyX secara menyeluruh termasuk semua komponen yang ada?"
${LangFileString} UnLyXPreferencesTitle 'Preferensi pengguna LyX'

${LangFileString} SecUnMiKTeXDescription "Penghapusan distribusi LaTeX MiKTeX."
${LangFileString} SecUnJabRefDescription "Penghapusan program manajemen bibliografi JabRef."
${LangFileString} SecUnPreferencesDescription 'Menghapus berkas konfigurasi LyX pada folder$\r$\n\
						$\"$AppPre\username\$AppSuff\${APP_DIR_USERDATA}$\"$\r$\n\
						untuk semua pengguna.'
${LangFileString} SecUnProgramFilesDescription "Penghapusan LyX serta semua komponen yang ada."

