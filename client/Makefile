# Define the C++ compiler
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++23 -Isrc -Isrc/hdr -Isrc/backends -Isrc/imgui -I../server/build

SRCS_DIR := src
TARGET := client

# Find all cpp files in src directory
SRCS := $(shell find $(SRCS_DIR) -name "*.cpp" | sort)

# Proto directory and files
PROTO_DIR := ../server/build
PROTO_SRCS := $(PROTO_DIR)/e-space.pb.cc $(PROTO_DIR)/e-space.grpc.pb.cc

# Define the object files
OBJS := $(SRCS:.cpp=.o)
PROTO_OBJS := $(PROTO_DIR)/e-space.pb.o $(PROTO_DIR)/e-space.grpc.pb.o

# Package config flags
PKG_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs sdl3 2>/dev/null)

# Updated LDLIBS with gRPC and protobuf (use pkg-config to get all dependencies)
GRPC_LIBS := $(shell pkg-config --libs grpc++ grpc protobuf 2>/dev/null || echo "-lgrpc++ -lgrpc -lprotobuf -labsl_log_internal_message -labsl_log_internal_check_op -labsl_log_internal_nullguard -labsl_synchronization -labsl_base")
LDLIBS := -ldl -lpthread -lvulkan $(PKG_LIBS) $(GRPC_LIBS)

# Default target: builds the executable
all: check_proto $(TARGET)

# Check if proto files exist
check_proto:
	@if [ ! -f $(PROTO_DIR)/e-space.pb.cc ]; then \
		echo "Error: Proto files not found. Building server first..."; \
		cd ../server/build && cmake .. && make; \
	fi

# Rule to link the object files into the final executable
$(TARGET): $(OBJS) $(PROTO_OBJS)
	$(CXX) $(OBJS) $(PROTO_OBJS) -o $(TARGET) $(LDLIBS)

# Rule to compile the C++ source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PKG_CFLAGS) -c $< -o $@

# Rule to compile proto files
$(PROTO_DIR)/%.o: $(PROTO_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: removes generated files
.PHONY: clean check_proto
clean:
	rm -f $(OBJS) $(PROTO_OBJS) $(TARGET)

# Run target
run: $(TARGET)
	./$(TARGET)