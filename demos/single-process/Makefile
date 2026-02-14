# Makefile for Single-Process XPC Demo
# This demo shows XPC communication within a single process

CC = clang
CFLAGS = -Wall -Wextra -g -fblocks

TARGET = demo_simple

.PHONY: all clean run

all: $(TARGET)

# Build single-process demo
demo_simple: demo_simple.c
	$(CC) $(CFLAGS) -o demo_simple demo_simple.c

# Run the demo
run: demo_simple
	./demo_simple

# Clean up
clean:
	rm -f $(TARGET) *.o *.dSYM

# Help
help:
	@echo "Single-Process XPC Demo"
	@echo ""
	@echo "This demo shows XPC communication within a single process."
	@echo "Both client and service run in the same process, which is useful"
	@echo "for understanding XPC APIs without the complexity of separate processes."
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the demo"
	@echo "  run     - Build and run the demo"
	@echo "  clean   - Remove built files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make run    # Build and run the demo"
	@echo "  make clean  # Clean up"
