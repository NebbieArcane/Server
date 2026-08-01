#!/bin/bash
# temp=$(git rev-list --tags --max-count=1)
echo git tag list
git tag --list
echo "git tag list 'r[0-9]*' --sort=-v:refname | head -n1"
(git tag --list 'r[0-9]*' --sort=-v:refname | head -n1)
echo git rev-parse abbrev-ref HEAD
git rev-parse --abbrev-ref HEAD
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
echo "branch: $branch"
echo "tag: $tag"
echo "version: $version"
# echo "temp: $temp"

# If no version is found, use git describe --always
if [ -z "$version" ]; then
	version=$(git describe --always)
	echo "version from describe: $version"
fi

# Real release → auto-motd. Feature/alpha builds must not touch motd Server slot.
# Signals: TAG=rX.Y[.Z], or HEAD exactly on such a tag.
is_release=0
motd_version=""
release_tag=""
exact_tag=$(git describe --exact-match --match 'r[0-9]*' HEAD 2>/dev/null || true)
if [ -n "$TAG" ] && [[ "$TAG" =~ ^r([0-9]+)\.([0-9]+)(\.[0-9]+)?$ ]]; then
	release_tag="$TAG"
elif [ -n "$exact_tag" ] && [[ "$exact_tag" =~ ^r([0-9]+)\.([0-9]+)(\.[0-9]+)?$ ]]; then
	release_tag="$exact_tag"
fi
if [ -n "$release_tag" ] && [[ "$release_tag" =~ ^r([0-9]+)\.([0-9]+)(\.[0-9]+)?$ ]]; then
	is_release=1
	motd_version="${BASH_REMATCH[1]}.${BASH_REMATCH[2]}${BASH_REMATCH[3]}"
fi

build=$(git log --pretty=format:"%f" -n1)
# Subject is %f (sanitized); body is %b for news extended text; author is %an.
build_body=$(git log -1 --pretty=%b)
build_author=$(git log -1 --pretty=%an)
# Optional short motd motto for releases (else binary uses BUILD humanized).
motd_headline="${MOTD_HEADLINE:-}"

# Escape a string for use inside a C "..." literal (keeps \n for multiline bodies).
c_escape() {
	local s=$1
	s=${s//\\/\\\\}
	s=${s//\"/\\\"}
	s=${s//$'\r'/}
	s=${s//$'\t'/\\t}
	s=${s//$'\n'/\\n}
	printf '%s' "$s"
}

build_esc=$(c_escape "$build")
build_body_esc=$(c_escape "$build_body")
build_author_esc=$(c_escape "$build_author")
motd_version_esc=$(c_escape "$motd_version")
motd_headline_esc=$(c_escape "$motd_headline")

echo "Tag: $branch $version $build author=$build_author is_release=$is_release motd_version=$motd_version"
#REVISION   = $(shell git rev-list $(LAST_TAG).. --count)
#ROOTDIR    = $(shell git rev-parse --show-toplevel)
outfile=${1:-release.hpp}
echo

if [ "$branch" = "HEAD" ] ; then
	branch="Release"
fi
echo "8<----------------------------"
echo "#define VERSION \"$version ($branch)\"" > "$outfile"
echo "#define BUILD \"$build_esc\"" >> "$outfile"
echo "#define BUILD_BODY \"$build_body_esc\"" >> "$outfile"
echo "#define BUILD_AUTHOR \"$build_author_esc\"" >> "$outfile"
echo "#define IS_RELEASE $is_release" >> "$outfile"
echo "#define MOTD_VERSION \"$motd_version_esc\"" >> "$outfile"
echo "#define MOTD_HEADLINE \"$motd_headline_esc\"" >> "$outfile"
cat "$outfile"
