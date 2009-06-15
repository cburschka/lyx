#Make a package from keystest scripts.
#It is supposed to be run inside development/keystest.

tar -c start_tests.sh killt* README stock_text report.sh autolyx lyx_make.sh maketar.sh test.py | gzip -9 > lyxtestc.tar.gz
