# XPC Demo Quick Start Guide

## TL;DR

```bash
# Run the recommended XPC service bundle demo (production pattern)
make run-service-based

# Or run the simple single-process demo
make run-single-process
```

## What You'll See

The **XPC service bundle demo** shows:
- Client app (PID: 12345)
- Service process (PID: 12346, PPID: 1)
- True inter-process communication!

## Files to Look At

### Production Pattern (Recommended)

1. **[xpc_service.c](demos/service-based/xpc_service.c)** - Service using `xpc_main()`
2. **[xpc_app.c](demos/service-based/xpc_app.c)** - Client connecting by service name
3. **[ServiceInfo.plist](demos/service-based/ServiceInfo.plist)** - Service bundle config
4. **[build_bundle.sh](demos/service-based/build_bundle.sh)** - Bundle creation script

### Learning/Simple Pattern

1. **[demo_simple.c](demos/single-process/demo_simple.c)** - All-in-one demonstration

## Key XPC Concepts

### XPC Service Bundle (Production)

**Service:**
```c
xpc_main(connection_handler);  // Never returns
```

**Client:**
```c
xpc_connection_t conn = xpc_connection_create("com.example.Service", NULL);
xpc_connection_resume(conn);
// Service launches automatically!
```

### Anonymous Connections (Advanced)

**Service:**
```c
xpc_connection_t listener = xpc_connection_create(NULL, NULL);
xpc_endpoint_t endpoint = xpc_endpoint_create(listener);
// Share endpoint with client somehow
```

**Client:**
```c
xpc_connection_t conn = xpc_connection_create_from_endpoint(endpoint);
```

## Message Format

All XPC messages are dictionaries:

```c
// Create message
xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
xpc_dictionary_set_string(msg, "type", "ping");
xpc_dictionary_set_int64(msg, "value", 42);

// Send and get reply
xpc_object_t reply = xpc_connection_send_message_with_reply_sync(conn, msg);

// Read reply
const char *response = xpc_dictionary_get_string(reply, "response");
int64_t result = xpc_dictionary_get_int64(reply, "result");
```

## Bundle Structure

After `make service-based`, you get:

```
build/XPCDemo.app/
└── Contents/
    ├── MacOS/XPCDemo           ← Client executable
    ├── Info.plist
    └── XPCServices/
        └── com.example.DemoService.xpc/
            └── Contents/
                ├── MacOS/
                │   └── com.example.DemoService  ← Service executable
                └── Info.plist
```

## Common Commands

```bash
# Build and run production demo
make run-service-based

# Run simple demo
make run-single-process

# Build all demos
make all

# Clean everything
make clean

# Show all options
make help
```

## Next Steps

1. Read the full [README.md](README.md)
2. Check the man pages: `man xpc`, `man xpc_connection_create`, `man xpc_main`
3. Modify [xpc_service.c](xpc_service.c) to add your own message types
4. Experiment with the service bundle structure

## Common Issues

**Service won't launch:**
- Check that bundle structure is correct
- Verify Info.plist files are valid XML
- Look for service crashes in Console.app

**Connection fails:**
- Ensure CFBundleIdentifier in ServiceInfo.plist matches the name in `xpc_connection_create()`
- Check that service executable is in the right place

**Want to see service output:**
- Service logs go to unified logging (Console.app)
- Or add explicit logging to a file in the service code

## Resources

- `man xpc` - Overview of XPC framework
- `man xpc_connection_create` - Connection management
- `man xpc_main` - XPC service runtime
- `man xpcservice.plist` - Service configuration
- Apple's [Creating XPC Services](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingXPCServices.html)
