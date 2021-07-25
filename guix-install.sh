#!/bin/sh
guix environment dwl --ad-hoc pkg-config wlroots guile -- make clean dwl CC=gcc
