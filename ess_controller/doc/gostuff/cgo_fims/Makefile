liblibrary.so: library.cpp library_bridge.cpp
	g++ -o liblibrary.so library.cpp library_bridge.cpp \
	-std=c++11 -O3 -Wall -Wextra -fPIC -shared

#I can run the example program as follows:

#$ make
#clang++ -o liblibrary.so library.cpp library-bridge.cpp \
#    -std=c++17 -O3 -Wall -Wextra -fPIC -shared



#$ go run library.go
