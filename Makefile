TARGET = vra

CC = g++
CFLAGS = -O0 -std=c++11
OBJECTS = main.o VariableRangeAnalyser.o ssaGraph.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)
main.o: main.cpp VariableRangeAnalyser.h
	$(CC) $(CFLAGS) main.cpp -c -o main.o
VariableRangeAnalyser.o: VariableRangeAnalyser.cpp VariableRangeAnalyser.h ssaGraph.h
	$(CC) $(CFLAGS) VariableRangeAnalyser.cpp -c -o VariableRangeAnalyser.o
ssaGraph.o: ssaGraph.cpp ssaGraph.h
	$(CC) $(CFLAGS) ssaGraph.cpp -c -o ssaGraph.o

.PHONY: clean
clean: 
	rm $(TARGET) $(OBJECTS)