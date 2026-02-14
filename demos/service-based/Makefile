# Makefile for Service-Based XPC Demo
# This demo shows the production pattern: XPC service in a bundle

CC = clang
CFLAGS = -Wall -Wextra -g -fblocks

.PHONY: all clean run bundle help

all: bundle

# Build the XPC service bundle (production pattern)
bundle:
	./build_bundle.sh

# Run the XPC service bundle demo
run: bundle
	./build/XPCDemo.app/Contents/MacOS/XPCDemo

# Clean up
clean:
	rm -rf build *.dSYM

# Help
help:
	@echo "Service-Based XPC Demo"
	@echo ""
	@echo "This demo shows the PRODUCTION PATTERN for XPC services."
	@echo "The XPC service is packaged as a proper .xpc bundle inside an"
	@echo "application bundle. This is how XPC services work in real macOS apps."
	@echo ""
	@echo "Structure:"
	@echo "  XPCDemo.app/"
	@echo "    Contents/"
	@echo "      MacOS/"
	@echo "        XPCDemo              (main app - xpc_app.c)"
	@echo "      XPCServices/"
	@echo "        com.example.DemoService.xpc/"
	@echo "          Contents/"
	@echo "            MacOS/"
	@echo "              com.example.DemoService  (service - xpc_service.c)"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the service bundle (default)"
	@echo "  bundle  - Build the service bundle"
	@echo "  run     - Build and run the demo"
	@echo "  clean   - Remove built files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make run    # Build and run the demo"
	@echo "  make clean  # Clean up"
