#!/bin/zsh

cd `dirname $0`/../build

rm -f unused1.txt unused.txt
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
cppcheck --project=compile_commands.json --enable=unusedFunction --language=c++  2> unused1.txt
grep -v Event unused1.txt > unused.txt
rm unused1.txt
echo "Results written to `pwd`/unused.txt"
