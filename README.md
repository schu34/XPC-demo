# XPC C API Demo

This project demonstrates inter-process communication using Apple's XPC (Cross-Process Communication) C API on macOS. It showcases multiple XPC patterns including **production-ready XPC service bundles**.

## Overview

XPC is a structured, asynchronous IPC library native to macOS. This demo project includes:

- **XPC Service Bundle** (xpc_service.c + xpc_app.c) - **RECOMMENDED** - Production pattern with true inter-process communication
- **demo_simple**: Working demo showing XPC API mechanics within a single process
- **service.c / client.c**: Standalone programs (for reference)
- **demo.c**: Multi-process attempt (experimental, demonstrates fork limitations with XPC)

## Project Structure

```
.
├── README.md               # This file
├── Makefile               # Build configuration
├── build_bundle.sh        # Script to create app bundle
│
├── xpc_service.c          # XPC Service using xpc_main() [PRODUCTION PATTERN]
├── xpc_app.c              # Client app connecting by service name
├── ServiceInfo.plist      # XPC Service bundle configuration
├── AppInfo.plist          # Application bundle configuration
│
├── demo_simple.c          # Working XPC demo (single process)
├── demo.c                 # Multi-process demo (experimental)
├── service.c              # Standalone service (reference)
├── client.c               # Standalone client (reference)
└── main.c                 # Original exploration code
```

### Built Bundle Structure

When you run `make bundle`, it creates:

```
build/XPCDemo.app/
└── Contents/
    ├── MacOS/
    │   └── XPCDemo                          (client executable)
    ├── XPCServices/
    │   └── com.example.DemoService.xpc/
    │       └── Contents/
    │           ├── MacOS/
    │           │   └── com.example.DemoService  (service executable)
    │           └── Info.plist
    └── Info.plist
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
# Build XPC service bundle (production pattern - RECOMMENDED)
make bundle

# Or build all standalone programs
make

# Or build specific targets
make demo_simple
make demo
make service
make client
```

## Running

### Quick Start - XPC Service Bundle (RECOMMENDED)

Run the production-style XPC service bundle demo:

```bash
make run-bundle
```

This is the **recommended approach** and demonstrates:
- **True inter-process communication** - Client and service are separate processes
- **xpc_main()** - Standard XPC service initialization
- **Named service connection** - Client connects by service name, not endpoint
- **Application-type service** - One service instance per application
- **Automatic launch-on-demand** - Service launches when first message is sent
- **Proper bundle structure** - How real macOS apps embed XPC services

### Alternative: Simple Demo (Single Process)

For a simpler demonstration within a single process:

```bash
make run
# or
./demo_simple
```

This demonstrates the XPC C API mechanics:
- Creating anonymous XPC connections
- Creating and using endpoints
- Sending structured messages (dictionaries)
- Request-reply patterns
- Different message types (ping, echo, add, info)

### Expected Output (XPC Service Bundle)

```
===========================================
XPC Service Demo - Client Application
===========================================

[Client] Connecting to XPC service: com.example.DemoService
[Client] Client PID: 71890

[Client] Connection established
[Client] Service will launch on-demand if not running

[Client] Sending: ping
[Client] Response: pong

[Client] Sending: echo 'Hello from XPC client!'
[Client] Response: Hello from XPC client!

[Client] Sending: add 42 + 23
[Client] Result: 65

[Client] Sending: get service info
[Client] Service PID: 71896, PPID: 1, Status: running
                      ^^^^^         ^
                      Different PID!  Parent is launchd!

[Client] Sending: echo 'XPC services are great!'
[Client] Response: XPC services are great!

[Client] Sending: add 100 + 200
[Client] Result: 300

[Client] All messages sent successfully!

===========================================
Demo complete!
===========================================

[Client] The XPC service will continue running for a while
[Client] and will automatically exit when idle
```

Notice that:
- **Client PID: 71890** - The client application
- **Service PID: 71896** - A completely separate process!
- **Service PPID: 1** - The service's parent is launchd, showing it's managed by the system

This demonstrates true inter-process communication via XPC!

## Understanding the Code

### XPC Service Bundle (RECOMMENDED - Production Pattern)

#### Service Side ([xpc_service.c](xpc_service.c))

The service uses `xpc_main()`, which is the standard initialization for XPC services:

```c
int main() {
    // xpc_main() never returns - it starts the XPC service runtime
    xpc_main(connection_handler);
}

static void connection_handler(xpc_connection_t peer) {
    // Called for each new connection from a client
    printf("New connection from PID: %d\n", xpc_connection_get_pid(peer));

    xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
        // Handle messages from this client
        handle_peer_event(peer, event);
    });

    xpc_connection_resume(peer);
}
```

Key points:
- `xpc_main()` initializes the service and handles connections automatically
- Each client connection gets its own peer connection object
- Service launches on-demand when first message arrives
- Service exits automatically when idle

#### Client Side ([xpc_app.c](xpc_app.c))

The client connects by service name - no endpoints needed:

```c
// Connect to service by its bundle identifier
xpc_connection_t connection = xpc_connection_create("com.example.DemoService", NULL);

xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
    // Handle connection errors
});

xpc_connection_resume(connection);

// Send messages - service will launch automatically if needed
xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
xpc_dictionary_set_string(message, "type", "ping");

xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, message);
```

Key points:
- Connect using the service's `CFBundleIdentifier` from Info.plist
- Service automatically launches on first message
- No manual process management needed
- This is the standard pattern for app-bundled XPC services

### Simple Demo ([demo_simple.c](demo_simple.c))

A simpler demonstration within a single process showing XPC API mechanics:

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

### Additional Reference Files

**[service.c](service.c)** - Standalone service (reference implementation)
**[client.c](client.c)** - Standalone client (reference implementation)
**[demo.c](demo.c)** - Multi-process attempt using fork() (experimental)

Note: The multi-process demo demonstrates the challenges of sharing XPC objects across fork() boundaries. For real multi-process XPC, use XPC service bundles (as shown above) or Mach services with launchd.

## XPC API Reference

### Core Functions Used

**XPC Service Bundle Pattern:**
- `xpc_main()` - Initialize XPC service runtime (service side)
- `xpc_connection_create("service.name", queue)` - Connect to named service (client side)

**Anonymous Connection Pattern:**
- `xpc_connection_create(NULL, NULL)` - Create anonymous listener
- `xpc_connection_create_from_endpoint()` - Create connection from endpoint
- `xpc_endpoint_create()` - Create endpoint from connection

**Common to All Patterns:**
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
- `xpc_connection_cancel()` - Cancel connection

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

This project now includes a **production-ready XPC service bundle** example!

### What This Demo Provides

The `make run-bundle` command demonstrates the recommended pattern for production apps:

1. **XPC service bundle** (`.xpc`) embedded within your app bundle
2. **Named service connection** using `xpc_connection_create("com.example.DemoService", NULL)`
3. **Proper service lifecycle** with `xpc_main()` in the service
4. **Standard bundle structure** matching real macOS applications
5. **Automatic process management** - service launches on-demand and exits when idle

### Comparison: Development vs Production Patterns

| Feature | Simple Demo | XPC Service Bundle | Mach Service |
|---------|------------|-------------------|--------------|
| **Processes** | Same process | Separate processes | Separate processes |
| **Connection Type** | Anonymous + endpoint | Named (by bundle ID) | Named (Mach service) |
| **Service Init** | Manual setup | `xpc_main()` | `xpc_connection_create_mach_service()` |
| **Deployment** | N/A | Bundle with app | launchd plist |
| **Launch Management** | Manual | Automatic (launchd) | launchd |
| **Use Case** | Learning/testing | App-bundled services | System services |
| **Production Ready** | No | **Yes** | Yes |

### When to Use Each Pattern

- **XPC Service Bundle** (this demo): For services bundled with your application
- **Mach Services**: For system-wide services managed by launchd
- **Anonymous Connections**: For dynamic, peer-to-peer IPC (advanced use cases)

## License

This is demonstration code for educational purposes.
