mkdir -p lib_build
cd lib_build
g++ -std=c++17 -Wall -fPIC -I ../include -c $(find ../src -name '*.cc' -mmin -5) $1
g++ -std=c++17 -fPIC -shared $(find -name '*.o') -o libecs.so $1