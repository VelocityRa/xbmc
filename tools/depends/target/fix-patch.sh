#!/bin/bash

sed -i '/^diff /g' $1
sed -i '/^index /g' $1
sed -i 's/^--- a\/tools\/depends\/target\/[a-zA-Z0-9]*\/aarch64-none-elf-release\//--- /g' $1
sed -i 's/^+++ b\/tools\/depends\/target\/[a-zA-Z0-9]*\/aarch64-none-elf-release\//+++ /g' $1
