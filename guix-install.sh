#!/bin/sh
guix environment dwl --ad-hoc pkg-config wlroots -- make clean dwl CC=gcc
