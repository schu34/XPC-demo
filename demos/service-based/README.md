# Service-Based XPC Demo

This demo shows the **PRODUCTION PATTERN** for XPC services on macOS. The XPC service is packaged as a proper `.xpc` bundle inside an application bundle, exactly how real macOS applications implement XPC services.

## What's Included

- `xpc_app.c` - Main application (client)
- `xpc_service.c` - XPC service implementation
- `AppInfo.plist` - Application bundle configuration
- `ServiceInfo.plist` - Service bundle configuration
- `build_bundle.sh` - Script to build the complete bundle structure
- `Makefile` - Build configuration

## How It Works

This is the **standard macOS pattern** for XPC services:

```
XPCDemo.app/
  Contents/
    Info.plist
    MacOS/
      XPCDemo                    ← Main app (connects by service name)
    XPCServices/
      com.example.DemoService.xpc/
        Contents/
          Info.plist
          MacOS/
            com.example.DemoService  ← XPC service (launched by launchd)
```

### Key Points

1. **Service Discovery**: The app connects to the service by name (`com.example.DemoService`)
2. **Automatic Launch**: macOS's `launchd` automatically launches the service when the app requests it
3. **Process Separation**: Client and service run in separate processes for security
4. **System Integration**: This is how all macOS apps use XPC services

## Running the Demo

```bash
make run
```

This will:
1. Compile both the app and service
2. Create the bundle structure
3. Copy Info.plist files and executables
4. Run the application

The app will automatically connect to the service, and launchd will launch it if needed.

## Expected Output

```
[Client] Connecting to service: com.example.DemoService
[Service] Peer connection established
[Client] Sending: ping
[Service] Received message of type: ping
[Service] Responding with pong
[Client] Received response: pong
...
```

## Bundle Structure

The build process creates:
- **Application Bundle**: Standard macOS .app with the client
- **Service Bundle**: .xpc bundle inside the app's XPCServices directory
- **Info.plist files**: Configure bundle identifiers and service names

## When to Use This Pattern

✅ **Use this pattern for:**
- Production applications
- Apps distributed via App Store
- Services requiring security isolation
- Background processing
- System-level services
- Any real-world XPC usage

This is the **correct way** to implement XPC services in macOS applications.

## Comparison with Single-Process Demo

| Feature | Single-Process | Service-Based (This Demo) |
|---------|---------------|---------------------------|
| Processes | 1 | 2 (app + service) |
| Service Discovery | Anonymous | By name via launchd |
| Security | None | Process isolation |
| Production Ready | No | Yes ✓ |
| Learning Curve | Easy | Moderate |

For learning XPC basics, start with the [single-process demo](../single-process/). For production use, use this pattern.
