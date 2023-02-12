# Makefile for telegram_check_updates
# Date: 05.02.2023

CXXFLAGS = -g -O2 -I$(HOME)/cpp_sources/stopfile -I$(HOME)/cpp_sources/logfile -Wall -Wextra -std=c++17 
	
telegram_check_updates:	telegram_check_updates.o ~/cpp_sources/stopfile/stopfile.o ~/cpp_sources/logfile/logfile.o
	g++ $(CXXFLAGS) ~/cpp_sources/stopfile/stopfile.o ~/cpp_sources/logfile/logfile.o telegram_check_updates.cpp -o telegram_check_updates -lstdc++fs -lcurl -ljsoncpp
	
stopfile.o: ~/cpp_sources/stopfile/stopfile.cpp ~/cpp_sources/stopfile/stopfile.hpp
	g++ $(CXXFLAGS) -c ~/cpp_sources/stopfile/stopfile.cpp -o ~/cpp_sources/stopfile/stopfile.o -lstdc++fs

logfile.o: ~/cpp_sources/logfile/logfile.cpp ~/cpp_sources/logfile/logfile.hpp
	g++ $(CXXFLAGS) -c ~/cpp_sources/logfile/logfile.cpp -o ~/cpp_sources/logfile/logfile.o -lstdc++fs
