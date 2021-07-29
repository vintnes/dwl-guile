#!/bin/sh
# $1 = dwl-guile patch tag to diff patches to

git fetch --tags
rm -rf patches
mkdir -p patches

for patch in xwayland
do
    git diff $1 patch/$patch \
        ':(exclude)README.md' \
        ':(exclude)patches' \
        ':(exclude)scripts/create-patches.sh' \
        ':(exclude).gitignore' > patches/$patch.patch
done

# Create dwl-guile patch based on dwl v0.2
git diff v0.2 > patches/dwl-guile.patch
