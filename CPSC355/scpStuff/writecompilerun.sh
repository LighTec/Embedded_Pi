vim $1
m4 $1 > "Assignment1.s"
gcc -g -o $2 "Assignment1.s"
rm "Assignment1.s"
./$2 input.bin
