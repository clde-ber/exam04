#!/bin/sh

./a.out /bin/cat "|" /bin/cat "|" /bin/cat "|" ls > my_output
./a.out /bin/ls "|" /bin/grep microshell ";" /bin/echo i love my microshell > my_output
./a.out /bin/cat file.txt "|" grep he "|" wc -l > my_output
./a.out echo hello there ";" find . -name file.txt ";" cat file.txt > my_output

/bin/cat | /bin/cat | /bin/cat | ls > shell_output
/bin/ls | /bin/grep microshell ; /bin/echo i love my microshell > shell_output
/bin/cat file.txt | grep he | wc -l > shell_output
echo hello there ";" find . -name file.txt ";" cat file.txt > shell_output