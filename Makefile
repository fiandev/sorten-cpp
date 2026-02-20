CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
BIN_DIR = binary

run:
        @FILE="$(word 2,$(MAKECMDGOALS))"; \
        if [ -z "$$FILE" ]; then \
                echo "Pake: make run <file>.cpp"; \
                exit 1; \
        fi; \
        mkdir -p $(BIN_DIR); \
        NAME=$${FILE##*/}; \
        NAME=$${NAME%.cpp}; \
        DIR=$$(dirname $$FILE); \
        if grep -q '#include "utils.h"' $$FILE 2>/dev/null && [ -f "$$DIR/utils.cpp" ]; then \
                $(CXX) $(CXXFLAGS) $$FILE $$DIR/utils.cpp -o $(BIN_DIR)/$$NAME && ./$(BIN_DIR)/$$NAME; \
        else \
                $(CXX) $(CXXFLAGS) $$FILE -o $(BIN_DIR)/$$NAME && ./$(BIN_DIR)/$$NAME; \
        fi

%:
        @:

.PHONY: run
.SILENT: %

