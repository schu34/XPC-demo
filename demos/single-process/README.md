# Single-Process XPC Demo

This demo shows XPC communication within a **single process**. Both the client and service run in the same process, making it easier to understand the XPC APIs without the complexity of inter-process communication.

## What's Included

- `demo_simple.c` - Complete XPC demo in one file
- `Makefile` - Build configuration

## How It Works

This approach:
- Creates an XPC service and client in the same process
- Uses anonymous XPC connections (no service name required)
- Shares the same dispatch queue between client and service
- Perfect for learning and testing XPC APIs

## Running the Demo

```bash
make run
```

This will:
1. Compile `demo_simple.c`
2. Run the demo, which shows:
   - Ping/pong messages
   - Echo functionality
   - Adding numbers
   - Process information

## Expected Output

```
[Service] Service started and listening...
[Client] Sending: ping
[Service] Received message of type: ping
[Service] Responding with pong
[Client] Received response: pong
...
```

## When to Use This Pattern

✅ **Good for:**
- Learning XPC APIs
- Testing XPC functionality
- Quick prototyping
- Understanding message flow

❌ **Not suitable for:**
- Production applications
- Security isolation
- Process separation
- System services

For production use, see the [service-based demo](../service-based/) which shows the proper XPC service bundle pattern.
