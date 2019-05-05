#!/bin/bash
# Extracts a sorted list of last translators from the po files

grep -h "Last-Translator" *.po | \
perl -pe 's/"Last-Translator: .*<([^>]+)>\\n"/\1/' | \
grep -v Last-Translator | \
sort
