# Makefile for XPC Demo

CC = clang

# -fblocks is needed for using blocks (eg `^(char foo){printf(foo)}`) in C, which is common in XPC code
CFLAGS = -Wall -Wextra -g -fblocks
LDFLAGS = -framework Foundation

# Targets
TARGETS = service client demo demo_simple
MAIN = main

.PHONY: all clean run

all: $(TARGETS)

# Build service executable
service: service.c
	$(CC) $(CFLAGS) -o service service.c $(LDFLAGS)

# Build client executable
client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

# Build demo (combined service + client in one binary with fork)
demo: demo.c
	$(CC) $(CFLAGS) -o demo demo.c $(LDFLAGS)

# Build simple demo (service + client in same process)
demo_simple: demo_simple.c
	$(CC) $(CFLAGS) -o demo_simple demo_simple.c $(LDFLAGS)

# Build original main
main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

# Run the simple demo (default)
run: demo_simple
	./demo_simple

# Run the multi-process demo (experimental - has issues)
run-multi: demo
	./demo

# Clean up
clean:
	rm -f $(TARGETS) $(MAIN) *.o
	rm -rf *.dSYM

# Help
help:
	@echo "XPC Demo Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all executables"
	@echo "  demo_simple  - Build the simple demo (RECOMMENDED)"
	@echo "  demo         - Build the multi-process demo (experimental)"
	@echo "  service      - Build the service program"
	@echo "  client       - Build the client program"
	@echo "  main         - Build the original main program"
	@echo "  run          - Build and run the simple demo"
	@echo "  run-multi    - Build and run the multi-process demo"
	@echo "  clean        - Remove all built files"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make            # Build all programs"
	@echo "  make run        # Run the simple demo (recommended)"
	@echo "  make run-multi  # Run the multi-process demo"
	@echo "  make clean      # Clean up"
