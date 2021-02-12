vim $1
m4 $1 > "Assignment6.s"
gcc -g -o $2 "Assignment6.s"
rm "Assignment6.s"
gdb --args $2 input.bin
