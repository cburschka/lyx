#! /bin/sh

# This script makes a bunch of symbolic links from the current directory
# to the one containing the LyX documentation.

USAGE () {
	echo Usage: $0 dir_where_the_docs_are_found
	exit 1
}

if [ $# -ne 1 ]; then
	USAGE
fi

DIR=$1

if [ ! -d ${DIR} ]; then
	USAGE
fi

FILES=`ls ${DIR}/*.*`

N_LYXFILES=`echo "${FILES}" | grep ".lyx" | wc -l`

if [ ${N_LYXFILES} -eq 0 ]; then
	echo ${DIR} contains NO lyx files!
	exit 1
fi

EXAMPLE_FILE=`echo "${FILES}" | grep ".lyx" | sed -e '2,$d'`
PARSABLEDIR=`echo ${DIR} | sed 's/\//\\\\\//g'`
EXAMPLE_FILE=`echo ${EXAMPLE_FILE} | sed "s/${PARSABLEDIR}\///"`

echo ${DIR} contains ${N_LYXFILES} .lyx files, an example being ${EXAMPLE_FILE}
echo
echo "Would you like to generate links to the files in this dir? (Y/N) >N<"
read ANSWER

if [ "${ANSWER}" != "y" -a "${ANSWER}" != "Y" ]; then
	exit 0
fi

echo
echo "Would you like these file names in the .cvsignore file? (Y/N) >N<"
read ANSWER

CVSIGNORE=0
if [ "${ANSWER}" = "y" -o "${ANSWER}" = "Y" ]; then
	CVSIGNORE=1
fi

# Keep the original .cvsignore file safe
if [ ${CVSIGNORE} -eq 1 ]; then
	if [ -r .cvsignore ]; then
		if [ -r .cvsignore-safe ]; then
			cp .cvsignore-safe .cvsignore
		else
			cp .cvsignore .cvsignore-safe
		fi
	fi
	echo .cvsignore-safe > .cvsignore
	echo '*.lyx' >> .cvsignore
	echo '*.eps' >> .cvsignore
fi

for FILE in ${FILES}
do
	DESTFILE=`echo ${FILE} | sed "s/${PARSABLEDIR}\///"`
	ln -fs ${FILE} ${DESTFILE}
done
