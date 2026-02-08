# XPC C API Demo

This project demonstrates inter-process communication using Apple's XPC (Cross-Process Communication) C API on macOS. It showcases the XPC C API with working examples.

## Overview

XPC is a structured, asynchronous IPC library native to macOS. This demo project includes:

- **demo_simple**: A working demo showing XPC API mechanics within a single process
- **service.c / client.c**: Standalone programs (for reference)
- **demo.c**: Multi-process attempt (experimental, demonstrates fork limitations with XPC)

## Project Structure

```
.
├── README.md           # This file
├── Makefile           # Build configuration
├── demo_simple.c      # Working XPC demo (RECOMMENDED)
├── demo.c             # Multi-process demo (experimental)
├── service.c          # Standalone service (reference)
├── client.c           # Standalone client (reference)
└── main.c             # Original exploration code
```

## Key XPC Concepts Demonstrated

### 1. Connection Types

- **Anonymous Connections**: Created with `xpc_connection_create(NULL, NULL)`
  - Not tied to a named service
  - Uses endpoints for inter-process communication
  - Perfect for dynamic, peer-to-peer communication

- **Endpoints**: Created with `xpc_endpoint_create(connection)`
  - Boxed connection that can be passed between processes
  - Allows creating connections via `xpc_connection_create_from_endpoint()`

### 2. Message Patterns

The demo implements several message patterns:

- **Ping/Pong**: Simple request-response
- **Echo**: String data round-trip
- **Add**: Arithmetic operation with integer parameters
- **Info**: Service metadata query

### 3. Message Structure

All messages are XPC dictionaries (property-list style):

```c
xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
xpc_dictionary_set_string(message, "type", "ping");
xpc_dictionary_set_int64(message, "value", 42);
```

### 4. Communication Modes

- **Synchronous**: `xpc_connection_send_message_with_reply_sync()`
  - Blocks until reply received
  - Returns reply object directly

- **Asynchronous**: `xpc_connection_send_message_with_reply()`
  - Non-blocking with callback handler
  - Uses dispatch queues

### 5. Event Handling

Event handlers receive both messages and errors:

```c
xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);
    if (type == XPC_TYPE_ERROR) {
        // Handle errors (connection invalid, interrupted, etc.)
    } else if (type == XPC_TYPE_DICTIONARY) {
        // Handle messages
    }
});
```

## Building

```bash
# Build all programs
make

# Or build specific targets
make demo_simple  # Recommended
make demo
make service
make client
```

## Running

### Quick Start - Recommended

Run the working demo:

```bash
make run
# or
./demo_simple
```

This demonstrates the XPC C API including:
- Creating anonymous XPC connections
- Creating and using endpoints
- Sending structured messages (dictionaries)
- Request-reply patterns
- Different message types (ping, echo, add, info)

### Expected Output

```
===========================================
XPC C API Demo - Two Process Communication
===========================================

This demo shows the XPC C API in action.
Since anonymous XPC connections are complex to demo across
actual separate processes, this version demonstrates the API
mechanics using a service and client within the same process.

For real inter-process XPC communication, you would typically:
1. Use xpc_main() in an XPC service bundle (.xpc)
2. Use xpc_connection_create("service.name", queue) from client
3. Or use xpc_connection_create_mach_service() with launchd

Starting demonstration...

[Service] Creating anonymous XPC listener...
[Service] Endpoint created (PID: 37103)
[Service] Setting up event handler...
[Service] Listener active and waiting for connections...

[Client] Creating connection from endpoint (PID: 37103)...
[Client] Connection established

[Client] Sending: ping
[Service] New connection established
[Service] Received message of type: ping
[Service] Responding with pong
[Client] Response: pong

[Client] Sending: echo 'Hello, XPC!'
[Service] Received message of type: echo
[Service] Echoing: Hello, XPC!
[Client] Response: Hello, XPC!

[Client] Sending: add 42 + 23
[Service] Received message of type: add
[Service] Adding 42 + 23 = 65
[Client] Result: 65

...
```

## Understanding the Code

### Main Demo ([demo_simple.c](demo_simple.c))

This is the recommended starting point. It demonstrates all key XPC C API features:

**Service Side:**
1. Creates anonymous listener with `xpc_connection_create(NULL, NULL)`
2. Creates endpoint with `xpc_endpoint_create(listener)`
3. Sets up event handler for new connections
4. Handles incoming messages by type
5. Creates and sends replies using `xpc_dictionary_create_reply()`

**Client Side:**
1. Receives endpoint
2. Creates connection with `xpc_connection_create_from_endpoint()`
3. Sets up event handler for errors
4. Sends messages with `xpc_connection_send_message_with_reply_sync()`
5. Processes replies

### Additional Files

**[service.c](service.c)** - Standalone service (reference implementation)
**[client.c](client.c)** - Standalone client (reference implementation)
**[demo.c](demo.c)** - Multi-process attempt using fork() (experimental)

Note: The multi-process demo demonstrates the challenges of sharing XPC objects across fork() boundaries. For real multi-process XPC, use named services with launchd or XPC service bundles.

## XPC API Reference

### Core Functions Used

- `xpc_connection_create()` - Create anonymous listener
- `xpc_connection_create_from_endpoint()` - Create connection from endpoint
- `xpc_endpoint_create()` - Create endpoint from connection
- `xpc_connection_set_event_handler()` - Set event/message handler
- `xpc_connection_resume()` - Activate connection
- `xpc_connection_send_message()` - Send message (fire-and-forget)
- `xpc_connection_send_message_with_reply_sync()` - Send and wait for reply
- `xpc_dictionary_create()` - Create message dictionary
- `xpc_dictionary_create_reply()` - Create reply to a message
- `xpc_dictionary_set_string()` - Set string value
- `xpc_dictionary_get_string()` - Get string value
- `xpc_dictionary_set_int64()` - Set integer value
- `xpc_dictionary_get_int64()` - Get integer value
- `xpc_connection_get_pid()` - Get peer process ID

## Error Handling

The code handles common XPC errors:

- `XPC_ERROR_CONNECTION_INVALID` - Connection closed or invalid
- `XPC_ERROR_CONNECTION_INTERRUPTED` - Connection temporarily interrupted
- `XPC_ERROR_TERMINATION_IMMINENT` - Service about to terminate

## Limitations

1. **Anonymous Connections**: This demo uses anonymous connections which require passing endpoints between processes (via fork in this case). For system-level services, you'd use named services with launchd.

2. **No Persistence**: Anonymous connections can't be re-established if broken, unlike named services.

3. **macOS Only**: XPC is a macOS-specific API, not portable to other Unix systems.

## Further Reading

- `man xpc` - Overview of XPC framework
- `man xpc_connection_create` - Connection creation and management
- `man xpc_object` - XPC object types
- `man xpc_dictionary_create` - Dictionary operations
- `man xpc_main` - XPC service runtime (for bundled services)

## Production XPC Services

For production use, you'd typically:

1. Create an XPC service bundle (`.xpc`) within your app
2. Use `xpc_connection_create("com.example.myservice", queue)` for named services
3. Or use `xpc_connection_create_mach_service()` with launchd for system services
4. Handle service lifecycle with `xpc_main()` in the service

This demo focuses on the core C API mechanics using anonymous connections for simplicity.

## License

This is demonstration code for educational purposes.
