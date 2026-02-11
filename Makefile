CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

SRC_DIR = src
BUILD_DIR = build
TARGET = rox

SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SRCS))

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: clean
