#!/bin/bash
temp=$(git rev-list --tags --max-count=1)
branch=$(git rev-parse --abbrev-ref HEAD)
version=$(git describe --tags --long)
if [ -z "$version" ] ; then
	version=$(git describe)
fi
build=$(git log --pretty=format:"%f" -n1)

#REVISION   = $(shell git rev-list $(LAST_TAG).. --count)
#ROOTDIR    = $(shell git rev-parse --show-toplevel)
outfile=${1:-release.hpp}
echo 
if [ "$branch" = "HEAD" ] ; then
	branch="Release"
fi
echo "#define VERSION \"$version ($branch)\"" > "$outfile"
echo "#define BUILD \"$build\"" >> "$outfile"
