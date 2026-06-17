#!/bin/bash
# temp=$(git rev-list --tags --max-count=1)
branch=$(git rev-parse --abbrev-ref HEAD)
tag=$(git tag --list '[r0-9]*' --sort=-v:refname | head -n1)

# Use the TAG env variable if defined, otherwise fall back to the latest tag
if [ -n "$TAG" ]; then
	version="$TAG"

else
	# If the tag matches the pattern vMAJOR.MINOR[.PATCH], increment the MINOR
	if [[ "$tag" =~ ^r([0-9]+)\.([0-9]+)(\.[0-9]+)?$ ]]; then
		major="${BASH_REMATCH[1]}"
		minor="${BASH_REMATCH[2]}"
		patch="${BASH_REMATCH[3]}"
		new_minor=$((minor + 1))
		if [ -n "$patch" ]; then
			version="v${major}.${new_minor}.0-alpha"
		else
			version="v${major}.${new_minor}.0-alpha"
		fi
	else
		version="$tag"
	fi
fi
# echo "temp: $temp"

# If no version is found, use git describe --always
if [ -z "$version" ]; then
	version=$(git describe --always)
fi

build=$(git log --pretty=format:"%f" -n1)
echo "Tag: $branch $version $build"
#REVISION   = $(shell git rev-list $(LAST_TAG).. --count)
#ROOTDIR    = $(shell git rev-parse --show-toplevel)
outfile=${1:-release.hpp}
echo

if [ "$branch" = "HEAD" ] ; then
	branch="Release"
fi
echo "8<----------------------------"
echo "#define VERSION \"$version ($branch)\"" > "$outfile"
echo "#define BUILD \"$build\"" >> "$outfile"
cat "$outfile"