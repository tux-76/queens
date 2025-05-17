CXX = g++
CXXFLAGS = -Wall -g
TARGET = queens
SRC = queens.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)