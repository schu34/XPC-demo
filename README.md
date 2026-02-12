# XPC C API Demo

This project demonstrates inter-process communication using Apple's XPC (Cross-Process Communication) C API on macOS.

## Overview

XPC is a structured, asynchronous IPC library native to macOS. This project contains **two complete demo implementations**, each showing a different approach to using XPC.

## Project Structure

```
xpc/
├── demos/
│   ├── single-process/          # Simple demo - client and service in one process
│   │   ├── demo_simple.c        # Complete implementation
│   │   ├── Makefile             # Build configuration
│   │   └── README.md            # Detailed documentation
│   │
│   └── service-based/           # Production pattern - separate processes
│       ├── xpc_app.c            # Client application
│       ├── xpc_service.c        # XPC service
│       ├── AppInfo.plist        # App bundle config
│       ├── ServiceInfo.plist    # Service bundle config
│       ├── build_bundle.sh      # Bundle build script
│       ├── Makefile             # Build configuration
│       └── README.md            # Detailed documentation
│
├── Makefile                     # Top-level build - builds all demos
├── README.md                    # This file
└── QUICKSTART.md               # Quick start guide
```

## Two Approaches to XPC

### 1. Single-Process Demo ([demos/single-process/](demos/single-process/))

**Purpose**: Learn XPC APIs without inter-process complexity

- Client and service run in the same process
- Uses anonymous XPC connections
- Perfect for understanding XPC message flow
- Simple to build and debug

```bash
cd demos/single-process
make run
```

**When to use**: Learning, testing, prototyping

### 2. Service-Based Demo ([demos/service-based/](demos/service-based/)) ⭐ **RECOMMENDED**

**Purpose**: Production pattern with real inter-process communication

- Client and service in separate processes
- Uses proper `.xpc` bundle structure
- Service launches on-demand via launchd
- This is how real macOS apps implement XPC

```bash
cd demos/service-based
make run
```

**When to use**: Production applications, any real-world XPC usage

## Quick Start

### Build Everything

```bash
make all
```

### Run the Single-Process Demo

```bash
make run-single-process
```

Output shows XPC communication within one process - great for learning the APIs.

### Run the Service-Based Demo (Recommended)

```bash
make run-service-based
```

Output shows:
- Two separate processes (client and service)
- Service launched by macOS's launchd
- True inter-process communication

## Key XPC Concepts Demonstrated

Both demos demonstrate:

### 1. Message Patterns
- **Ping/Pong**: Simple request-response
- **Echo**: String data round-trip
- **Add**: Arithmetic operations with parameters
- **Info**: Service metadata and process information

### 2. XPC Object Types
- Dictionaries (property-list style)
- Strings, integers, and other data types
- Reply objects linked to original messages

### 3. Communication Modes
- **Synchronous**: `xpc_connection_send_message_with_reply_sync()` - blocks until reply
- **Asynchronous**: `xpc_connection_send_message_with_reply()` - callback-based

### 4. Connection Types
- **Anonymous connections** (single-process demo): Created with `xpc_connection_create(NULL, NULL)`
- **Named connections** (service-based demo): Created with `xpc_connection_create("service.name", NULL)`

### 5. Service Lifecycle
- **Manual setup** (single-process): Direct connection creation
- **xpc_main()** (service-based): Standard XPC service initialization with automatic lifecycle

## Comparison

| Feature | Single-Process | Service-Based |
|---------|---------------|---------------|
| **Processes** | 1 | 2 (client + service) |
| **Setup Complexity** | Simple | Moderate |
| **Connection Type** | Anonymous + endpoint | Named (by service ID) |
| **Service Launch** | Manual | Automatic (launchd) |
| **Security Isolation** | None | Process isolation |
| **Production Ready** | No | Yes ✓ |
| **Best For** | Learning | Real applications |

## Learning Path

1. **Start with single-process** ([demos/single-process/](demos/single-process/))
   - Understand XPC message structure
   - Learn connection APIs
   - See request-reply patterns
   - Debug easily in one process

2. **Move to service-based** ([demos/service-based/](demos/service-based/))
   - See how production apps use XPC
   - Understand bundle structure
   - Learn service lifecycle
   - Implement proper process separation

## What Each Demo Shows

### Single-Process Demo Features
- Creating anonymous XPC connections
- Using XPC endpoints for connection sharing
- Sending structured messages (dictionaries)
- Synchronous request-reply patterns
- Message handling and responses

### Service-Based Demo Features
- Proper XPC service bundle structure
- Service discovery by name
- `xpc_main()` service initialization
- Automatic service launch via launchd
- Inter-process communication
- Service lifecycle management
- Production-ready patterns

## XPC API Reference

### Core Functions Used

**Connection Management:**
- `xpc_connection_create("service.name", queue)` - Connect to named service
- `xpc_connection_create(NULL, NULL)` - Create anonymous listener
- `xpc_connection_create_from_endpoint()` - Create from endpoint
- `xpc_connection_set_event_handler()` - Set event/message handler
- `xpc_connection_resume()` - Activate connection
- `xpc_connection_cancel()` - Cancel connection

**Service Initialization:**
- `xpc_main()` - Initialize XPC service runtime (service-based pattern)

**Messaging:**
- `xpc_connection_send_message()` - Send message (fire-and-forget)
- `xpc_connection_send_message_with_reply_sync()` - Send and wait for reply
- `xpc_connection_send_message_with_reply()` - Send with async callback

**Dictionary Operations:**
- `xpc_dictionary_create()` - Create message dictionary
- `xpc_dictionary_create_reply()` - Create reply to a message
- `xpc_dictionary_set_string()` / `xpc_dictionary_get_string()` - String values
- `xpc_dictionary_set_int64()` / `xpc_dictionary_get_int64()` - Integer values

**Endpoints:**
- `xpc_endpoint_create()` - Create endpoint from connection

**Utilities:**
- `xpc_connection_get_pid()` - Get peer process ID
- `xpc_get_type()` - Get XPC object type

## Error Handling

Both demos handle common XPC errors:
- `XPC_ERROR_CONNECTION_INVALID` - Connection closed or invalid
- `XPC_ERROR_CONNECTION_INTERRUPTED` - Connection temporarily interrupted
- `XPC_ERROR_TERMINATION_IMMINENT` - Service about to terminate

## Building from Top Level

```bash
# Build all demos
make all

# Build specific demo
make single-process
make service-based

# Run specific demo
make run-single-process
make run-service-based

# Clean everything
make clean

# Show help
make help
```

## Further Reading

- `man xpc` - Overview of XPC framework
- `man xpc_connection_create` - Connection creation and management
- `man xpc_object` - XPC object types and operations
- `man xpc_dictionary_create` - Dictionary operations
- `man xpc_main` - XPC service runtime (for bundled services)

For detailed documentation on each approach:
- [Single-Process Demo README](demos/single-process/README.md)
- [Service-Based Demo README](demos/service-based/README.md)

## License

This is demonstration code for educational purposes.
