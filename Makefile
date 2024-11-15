# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lssl -lcrypto

# Targets and sources
TARGET = imapcl
SRC = hello.cpp parseCMD.cpp imap.cpp imaps.cpp
OBJ = $(SRC:.cpp=.o)
DEPS = parseCMD.h imap.h imaps.h

# Default target: build everything
all: $(TARGET)

# Link object files into the final target (imapcl)
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule for compiling .cpp to .o files
%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(TARGET)
