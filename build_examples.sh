mkdir -p example_build
cd example_build
cp ../lib_build/libecs.so .

for file in ../examples/*.cc
do
    g++ -std=c++17 -Wall -I ../include -c "$file" $1
    g++ -L . $(basename "$file" .cc).o -o $(basename "$file" .cc) -lecs -lpthread -Wl,-rpath='$ORIGIN'
done