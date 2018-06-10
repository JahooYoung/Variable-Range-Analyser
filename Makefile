TARGET = vra

CC = g++
CFLAGS = -O0 -std=c++11 -Wall
OBJECTS = main.o VariableRangeAnalyser.o SsaGraph.o ConstraintGraph.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)
main.o: main.cpp VariableRangeAnalyser.h
	$(CC) $(CFLAGS) main.cpp -c -o main.o
VariableRangeAnalyser.o: VariableRangeAnalyser.cpp VariableRangeAnalyser.h SsaGraph.h ConstraintGraph.h
	$(CC) $(CFLAGS) VariableRangeAnalyser.cpp -c -o VariableRangeAnalyser.o
SsaGraph.o: SsaGraph.cpp SsaGraph.h
	$(CC) $(CFLAGS) SsaGraph.cpp -c -o SsaGraph.o
ConstraintGraph.o: ConstraintGraph.cpp ConstraintGraph.h
	$(CC) $(CFLAGS) ConstraintGraph.cpp -c -o ConstraintGraph.o

.PHONY: clean
clean: 
	rm $(TARGET) $(OBJECTS)