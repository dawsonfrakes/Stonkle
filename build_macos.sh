#!/bin/sh

set -e

cc -Wall -Wextra -nostdlib -o Stonkle main_macos.mm -framework AppKit -Wl,-e,_start
