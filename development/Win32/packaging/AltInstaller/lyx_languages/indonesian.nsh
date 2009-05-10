!ifndef _LYX_LANGUAGES_INDONESIAN_NSH_
!define _LYX_LANGUAGES_INDONESIAN_NSH_

!ifdef LYX_LANG
  !undef LYX_LANG
!endif
!define LYX_LANG ${LANG_INDONESIAN}

LicenseLangString LyXLicenseData ${LYX_LANG} "${PRODUCT_LICENSE_FILE}"

LangString WelcomePageText "${LYX_LANG}" "Program ini akan memandu anda dalam melakukan instalasi LyX.\r\n\
					  \r\n\
					  Anda harus bertindak dan mempunyai hak sebagai administrator untuk instalasi LyX.\r\n\
					  \r\n\
					  Sangat disarankan anda menutup semua aplikasi sebelum memulai. Hal ini akan memungkinkan memperbarui berkas sistem yang relevan tanpa melakukan booting ulang komputer anda.\r\n\
					  \r\n\
					  $_CLICK"

LangString FileTypeTitle "${LYX_LANG}" "Dokumen-LyX"

LangString SecInstGSviewTitle "${LYX_LANG}" "GSview"
LangString SecInstJabRefTitle "${LYX_LANG}" "JabRef"
LangString SecAllUsersTitle "${LYX_LANG}" "Instal untuk semua pengguna?"
LangString SecFileAssocTitle "${LYX_LANG}" "Berkas yang terkait"
LangString SecDesktopTitle "${LYX_LANG}" "Ikon Desktop"

LangString SecCoreDescription "${LYX_LANG}" "Berkas-berkas LyX."
LangString SecInstGSviewDescription "${LYX_LANG}" "Program untuk melihat dokumen Postscript dan PDF."
LangString SecInstJabRefDescription "${LYX_LANG}" "Program manajemen acuan bibliografi dan penyunting berkas BibTeX."
LangString SecAllUsersDescription "${LYX_LANG}" "Instal LyX untuk semua pengguna atau hanya untuk pengguna ini saja."
LangString SecFileAssocDescription "${LYX_LANG}" "Berkas dengan ekstensi .lyx akan otomatis dibuka menggunakan LyX."
LangString SecDesktopDescription "${LYX_LANG}" "Ikon LyX muncul di desktop."

LangString LangSelectHeader "${LYX_LANG}" "Pilihan bahasa untuk menu LyX"
LangString AvailableLang "${LYX_LANG}" " Bahasa yang tersedia "

LangString MissProgHeader "${LYX_LANG}" "Pengecekan program yang diperlukan"
LangString MissProgCap "${LYX_LANG}" "Berikut ini adalah program yang diperlukan dan akan diinstal sebagai tambahan"
LangString MissProgLatex "${LYX_LANG}" "MiKTeX, salah satu program distribusi LaTeX"
LangString MissProgGhostscript "${LYX_LANG}" "Ghostscript, suatu penafsir PostScript and PDF"
LangString MissProgImageMagick "${LYX_LANG}" "ImageMagick, salah satu pengkonversi gambar"
LangString MissProgAspell "${LYX_LANG}" "Aspell, salah satu program pemeriksa ejaan"
LangString MissProgMessage "${LYX_LANG}" 'Tidak ada program tambahan yang perlu diinstal.'

LangString EnterLaTeXHeader1 "${LYX_LANG}" 'Distribusi LaTeX'
LangString EnterLaTeXHeader2 "${LYX_LANG}" 'Pengaturan distribusi LaTeX yang akan digunakan dalam LyX.'
LangString EnterLaTeXFolder "${LYX_LANG}" 'Anda dapat mengatur sendiri lokasi tempat berkas "latex.exe" berada dan mengatur lokasi \
					   tempat distribusi LaTeX yang akan digunakan oleh LyX.\r\n\
					   Apabila anda tidak menggunakan LaTeX, LyX tidak akan menampilkan output dokumen!\r\n\
					   \r\n\
					   Program instalasi ini mendeteksi adanya distribusi LaTeX \
					   "$LaTeXName" dalam sistem anda. Dibawah ini adalah lokasi keberadaannya.'
LangString EnterLaTeXFolderNone "${LYX_LANG}" 'Aturlah lokasi tempat berkas "latex.exe" berada. Atur dan nyatakanlah lokasi \
					       tempat distribusi LaTeX yang akan digunakan oleh LyX.\r\n\
					       Apabila anda tidak menggunakan LaTeX, LyX tidak akan menampilkan output dokumen!\r\n\
					       \r\n\
					       Program instalasi ini tidak menemukan adanya distribusi LaTeX di sistem anda.'
LangString PathName "${LYX_LANG}" 'Lokasi tempat berkas "latex.exe" berada'
LangString DontUseLaTeX "${LYX_LANG}" "Tidak menggunakan LaTeX"
LangString InvalidLaTeXFolder "${LYX_LANG}" 'Berkas "latex.exe" tidak berada di lokasi tempat yang dinyatakan.'

LangString LatexInfo "${LYX_LANG}" 'Sekarang instalasi program distribusi LaTeX "MiKTeX" akan dijalankan.$\r$\n\
			            Untuk meneruskan tekanlah tombol "Lanjut" yang ada di jendela dan tunggu sampai instalasi dimulai.$\r$\n\
				    $\r$\n\
				    !!! Gunakan semua pilihan default pada program instalasi MiKTeX !!!'
LangString LatexError1 "${LYX_LANG}" 'Distribusi LaTeX tidak ditemukan!$\r$\n\
                      		      LyX tidak bisa digunakan tanpa menggunakan salah satu distribusi LaTeX seperti "MiKTeX"!$\r$\n\
				      Oleh karena itu proses instalasi dibatalkan.'
			    
LangString GSviewInfo "${LYX_LANG}" 'Sekarang instalasi program "GSview" akan dijalankan.$\r$\n\
			             Untuk memulainya tekanlah tombol "Mulai" pada jendela instalasi yang pertama,$\r$\n\
				     pilihlah bahasa yang anda inginkan kemudian tekan tombol "Lanjut" pada jendela berikutnya.$\r$\n\
				     Anda boleh memilih semua pilihan default yang ada di program instalasi GSview.'
LangString GSviewError "${LYX_LANG}" 'Program "GSview" tidak berhasil diinstal secara keseluruhan!$\r$\n\
		                      Namun proses instalasi dapat diteruskan.$\r$\n\
				      Anda dapat mengulangi instalasi GSview nanti.'
				      
LangString JabRefInfo "${LYX_LANG}" 'Sekarang instalasi program "JabRef" akan dijalankan.$\r$\n\
				     Anda boleh memilih semua pilihan default yang ada di program instalasi JabRef.'
LangString JabRefError "${LYX_LANG}" 'Program "JabRef" tidak berhasil diinstal secara keseluruhan!$\r$\n\
		                      Namun proses instalasi dapat diteruskan.$\r$\n\
				      Anda dapat mengulang instalasi JabRef nanti.'
				      
LangString LatexConfigInfo "${LYX_LANG}" "Proses konfigurasi LyX selanjutnya akan memerlukan waktu beberapa saat."

LangString AspellInfo "${LYX_LANG}" 'Sekarang kamus untuk pemeriksa ejaan "Aspell" akan dimuat turun kemudian diinstal.$\r$\n\
				     Setiap lisensi kamus yang berbeda akan ditampilkan sebelum instalasi dimulai.'
LangString AspellDownloadFailed "${LYX_LANG}" "Kamus pemeriksa ejaan Aspell tidak bisa dimuat turun!"
LangString AspellInstallFailed "${LYX_LANG}" "Kamus pemeriksa ejaan Aspell tidak bisa diinstal!"
LangString AspellPartAnd "${LYX_LANG}" " dan "
LangString AspellPartSep "${LYX_LANG}" ", "
LangString AspellAll "${LYX_LANG}" 'Kamus bahasa sudah diinstal'
LangString AspellNone "${LYX_LANG}" 'Kamus untuk pemeriksa ejaan "Aspell" belum diinstal.$\r$\n\
				     Berbagai kamus dapat dimuat turun dari$\r$\n\
				     ${AspellLocation}$\r$\n\
				     Apakah anda menginginkan memuat turun berbagai kamus sekarang?'
LangString AspellPartStart "${LYX_LANG}" "Sudah berhasil diinstal "
LangString AspellPart1 "${LYX_LANG}" "kamus bahasa inggris"
LangString AspellPart2 "${LYX_LANG}" "Kamus untuk bahasa $LangName"
LangString AspellPart3 "${LYX_LANG}" "a$\r$\n\
				      kamus dari bahasa $LangNameSys"
LangString AspellPart4 "${LYX_LANG}" ' untuk pemeriksa ejaan "Aspell".$\r$\n\
				      Berbagai kamus dapat dimuat turun dari$\r$\n\
				      ${AspellLocation}$\r$\n\
				      Apakah anda menginginkan memuat turun berbagai kamus yang lain sekarang?'

LangString MiKTeXPathInfo "${LYX_LANG}" "Agar memungkinkan semua pengguna dapat mengatur MiKTeX sesuai keinginannya$\r$\n\
					 anda perlu memberikan hak merubah untuk semua pengguna pada lokasi instalasi MiKTeX yaitu folder$\r$\n\
					 $MiKTeXPath $\r$\n\
					 serta di semua subfoldernya."
LangString MiKTeXInfo "${LYX_LANG}" 'Distribusi LaTeX "MiKTeX" akan digunakan dengan LyX.$\r$\n\
				     Sangat disarankan memperbarui program yang ada di MiKTeX menggunakan "MiKTeX Update Wizard"$\r$\n\
				     sebelum anda menggunakan LyX untuk yang pertama kali.$\r$\n\
				     Apakah anda akan memeriksa perlunya memperbarui MikTeX?'

LangString UpdateNotAllowed "${LYX_LANG}" "Program update ini hanya hanya dapat memperbarui ${PRODUCT_VERSION_OLD}!"
LangString ModifyingConfigureFailed "${LYX_LANG}" "Tidak bisa menyatakan 'path_prefix' pada skrip configure"
LangString CreateCmdFilesFailed "${LYX_LANG}" "Tidak bisa membuat berkas lyx.bat"
LangString RunConfigureFailed "${LYX_LANG}" "Tidak bisa menjalankan skrip configure"
LangString NotAdmin "${LYX_LANG}" "Anda harus berlaku dan mempunyai hak sebagai administrator untuk instalasi LyX!"
LangString InstallRunning "${LYX_LANG}" "Program instalasi sedang berjalan!"
LangString StillInstalled "${LYX_LANG}" "LyX sudah pernah diinstal! Perlu menghapus LyX yang ada terlebih dahulu."

LangString FinishPageMessage "${LYX_LANG}" "Selamat! LyX berhasil diinstal dengan sukses.\r\n\
					    \r\n\
					    (Menjalankan LyX untuk pertama kali memerlukan waktu beberapa detik.)"
LangString FinishPageRun "${LYX_LANG}" "Menjalankan LyX"

LangString UnNotInRegistryLabel "${LYX_LANG}" "Tidak bisa menemukan LyX di catatan registry.$\r$\n\
					       Shortcuts pada desktop dan yang ada di Menu Start tidak dihapus."
LangString UnInstallRunning "${LYX_LANG}" "Anda harus menutup LyX terlebih dahulu!"
LangString UnNotAdminLabel "${LYX_LANG}" "Anda harus berlaku dan mempunyai hak sebagai administrator untuk menghapus LyX!"
LangString UnReallyRemoveLabel "${LYX_LANG}" "Apakah anda yakin akan menghapus LyX secara menyeluruh termasuk semua komponen yang ada?"
LangString UnLyXPreferencesTitle "${LYX_LANG}" 'Preferensi pengguna LyX'
LangString UnGSview "${LYX_LANG}" 'Silahkan tekan tombol "Hapus" pada jendela berikutnya untuk memulai penghapusan$\r$\n\
				   Postscript dan  penampil PDF "GSview".'
LangString UnRemoveSuccessLabel "${LYX_LANG}" "LyX telah dihapus dengan sukses dari komputer anda."

LangString SecUnAspellDescription "${LYX_LANG}" 'Penghapusan pemeriksa ejaan Aspell serta kamus yang ada.'
LangString SecUnMiKTeXDescription "${LYX_LANG}" "Penghapusan distribusi LaTeX MiKTeX."
LangString SecUnJabRefDescription "${LYX_LANG}" "Penghapusan program manajemen bibliografi JabRef."
LangString SecUnPreferencesDescription "${LYX_LANG}" 'Menghapus berkas konfigurasi LyX pada folder$\r$\n\
					   "$AppPre\username\$AppSuff\${PRODUCT_SUBFOLDER}"$\r$\n\
					   untuk semua pengguna.'
LangString SecUnProgramFilesDescription "${LYX_LANG}" "Penghapusan LyX serta semua komponen yang ada."

!undef LYX_LANG

!endif # _LYX_LANGUAGES_INDONESIAN_NSH_
