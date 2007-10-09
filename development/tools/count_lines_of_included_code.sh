
for i in \
	boost/bind.hpp \
	boost/array.hpp \
	boost/assert.hpp \
	boost/crc.hpp \
	boost/cregex.hpp \
	boost/current_function.hpp \
	boost/function.hpp \
	boost/iterator/indirect_iterator.hpp \
	boost/scoped_array.hpp \
	boost/scoped_ptr.hpp \
	boost/shared_ptr.hpp \
	boost/signal.hpp \
	boost/signals/connection.hpp \
	boost/signals/trackable.hpp \
	boost/tokenizer.hpp \
	boost/tuple/tuple.hpp \
	boost/utility.hpp \
	boost/version.hpp \
	boost/signals/trackable.hpp \
	string \
	vector \
	map \
	list \
	deque \
	QObject \
	QString \
	QList \
	QVector \
	QMap \
	QHash \
	QRegExp
do
	echo "#include <$i>"  > 1.cpp
	inc='-I/suse/usr/src/lyx/trunk/boost -I/usr/include/qt4/QtCore -I/usr/include/qt4'
	l=`g++ $inc -DQT_NO_STL -E 1.cpp | wc -l`
	printf "%-40s: %d\n" $i $l
done

