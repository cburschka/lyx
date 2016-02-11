/*
LyX Installer Language File
Language: Indonesian
Author: Waluyo Adi Siswanto
*/

!insertmacro LANGFILE_EXT "Indonesian"

${LangFileString} TEXT_INSTALL_CURRENTUSER "(Pasang untuk Pengguna Yang Ini)"

${LangFileString} TEXT_WELCOME "Program ini akan memandu anda dalam melakukan instalasi LyX.$\r$\n\
				$\r$\n\
				$_CLICK"

${LangFileString} TEXT_CONFIGURE_MIKTEXFNDB "Memperbarui nama basisdata MiKTeX..."
${LangFileString} TEXT_CONFIGURE_LYX "Konfigurasi LyX ($LaTeXInstalled mungkin mengunduh paket yang diperlukan, proses perlu waktu mohon tunggu) ..."
${LangFileString} TEXT_CONFIGURE_PYTHON "Proses kompilasi skrip Python ..."

${LangFileString} TEXT_FINISH_DESKTOP "Membuat pintasan ikon di destop"
${LangFileString} TEXT_FINISH_WEBSITE "Kunjungi lyx.org untuk berita terbaru serta dukungan"

${LangFileString} FileTypeTitle "Dokumen-LyX"

${LangFileString} SecInstJabRefTitle "JabRef"
${LangFileString} SecAllUsersTitle "Pasang untuk semua pengguna?"
${LangFileString} SecFileAssocTitle "Berkas yang terkait"
${LangFileString} SecDesktopTitle "Ikon Desktop"

${LangFileString} SecCoreDescription "Berkas-berkas LyX."
${LangFileString} SecInstJabRefDescription "Program manajemen acuan bibliografi dan penyunting berkas BibTeX."
${LangFileString} SecAllUsersDescription "Pasang LyX untuk semua pengguna atau hanya untuk pengguna ini saja."
${LangFileString} SecFileAssocDescription "Berkas dengan ekstensi .lyx akan otomatis dibuka menggunakan LyX."
${LangFileString} SecDesktopDescription "Ikon LyX muncul di desktop."
${LangFileString} SecDictionariesDescription "Kamus pemeriksa ejaan yang dapat diunduh dan dipasang."
${LangFileString} SecThesaurusDescription "Kamus padanan kata yang dapat diunduh dan dipasang."

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
				Sangat disarankan memperbarui program yang ada di MiKTeX menggunakan $\"Bantuan memperbarui MiKTeX$\"$\r$\n\
				sebelum anda menggunakan LyX untuk yang pertama kali.$\r$\n\
				Apakah anda akan memeriksa perlunya memperbarui MikTeX?'

${LangFileString} ModifyingConfigureFailed "Tidak bisa menyatakan 'path_prefix' pada skrip configure"
${LangFileString} RunConfigureFailed "Tidak bisa menjalankan skrip konfigurasi"
${LangFileString} NotAdmin "Anda harus berlaku dan mempunyai hak sebagai administrator untuk instalasi LyX!"
${LangFileString} InstallRunning "Program instalasi sedang berjalan!"
${LangFileString} AlreadyInstalled "LyX ${APP_SERIES_KEY2} sudah pernah diinstal!$\r$\n\
				Installing over existing installations is not recommended if the installed version$\r$\n\
				is a test release or if you have problems with your existing LyX installation.$\r$\n\
				In these cases better reinstall LyX.$\r$\n\
				Dou you nevertheles want to install LyX over the existing version?"
${LangFileString} NewerInstalled "Anda akan memasang versi LyX yang lama dari yang sudah terpasang.$\r$\n\
				  Jika memang dikehendaki, anda harus menghapus dulu LyX yang ada, LyX $OldVersionNumber."

${LangFileString} MultipleIndexesNotAvailable "Tidak ada dukungan menggunakan beberapa indeks dalam dokumen karena$\r$\n\
						MiKTeX dipasang oleh administrator tetapi anda akan memasang LyX bukan sebagai administrator."

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

