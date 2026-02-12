# Top-level Makefile for XPC Demos
# This project demonstrates XPC (inter-process communication) on macOS

.PHONY: all clean help single-process service-based

# Default: build all demos
all: single-process service-based

# Build single-process demo
single-process:
	@echo "Building single-process demo..."
	@$(MAKE) -C demos/single-process

# Build service-based demo
service-based:
	@echo "Building service-based demo..."
	@$(MAKE) -C demos/service-based

# Run single-process demo
run-single-process:
	@echo "Running single-process demo..."
	@$(MAKE) -C demos/single-process run

# Run service-based demo
run-service-based:
	@echo "Running service-based demo..."
	@$(MAKE) -C demos/service-based run

# Clean all demos
clean:
	@echo "Cleaning all demos..."
	@$(MAKE) -C demos/single-process clean
	@$(MAKE) -C demos/service-based clean
	rm -f demo.c demo_simple.c service.c client.c main.c
	rm -f demo demo_simple service client main
	rm -f xpc_app.c xpc_service.c
	rm -f AppInfo.plist ServiceInfo.plist build_bundle.sh
	rm -rf build *.o *.dSYM

# Help
help:
	@echo "XPC Demos - macOS Inter-Process Communication"
	@echo ""
	@echo "This project contains two XPC demo approaches:"
	@echo ""
	@echo "1. Single-Process Demo (demos/single-process/)"
	@echo "   - Client and service in the same process"
	@echo "   - Great for learning XPC APIs"
	@echo "   - Simple to build and run"
	@echo ""
	@echo "2. Service-Based Demo (demos/service-based/)"
	@echo "   - PRODUCTION PATTERN with proper .xpc bundle"
	@echo "   - Client and service in separate processes"
	@echo "   - Uses macOS service discovery"
	@echo "   - How real apps implement XPC"
	@echo ""
	@echo "Targets:"
	@echo "  all                  - Build all demos"
	@echo "  single-process       - Build single-process demo"
	@echo "  service-based        - Build service-based demo"
	@echo "  run-single-process   - Run single-process demo"
	@echo "  run-service-based    - Run service-based demo (RECOMMENDED)"
	@echo "  clean                - Clean all demos"
	@echo "  help                 - Show this help"
	@echo ""
	@echo "Quick Start:"
	@echo "  make run-single-process   # Start with the simple demo"
	@echo "  make run-service-based    # Run the production pattern"
	@echo ""
	@echo "For more details, see:"
	@echo "  - demos/single-process/README.md"
	@echo "  - demos/service-based/README.md"
	@echo "  - README.md (project overview)"
