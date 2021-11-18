g++-11 -Wall $(find src -type d | awk '$0="-I ./"$0') -c $(find src -name '*.cpp' -mmin -5) $1
g++-11 $(find -name '*.o') -o App.elf $1