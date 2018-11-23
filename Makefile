CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pedantic -Wno-unused-parameter -ggdb -O0
HEADERS=-Iheaders
SOURCE=source/*.cpp tests/*.cpp
LIBRARIES=-llua
RESULT=tests/test

$(RESULT):
	$(CXX) $(CXXFLAGS) $(HEADERS) -o $(RESULT) $(SOURCE) $(LIBRARIES)
