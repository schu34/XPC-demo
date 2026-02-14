#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>

// Client application that connects to the XPC service by name

static void send_and_print(xpc_connection_t conn, xpc_object_t message, const char *desc) {
    printf("[Client] Sending: %s\n", desc);

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(conn, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR) {
        if (reply == XPC_ERROR_CONNECTION_INVALID) {
            fprintf(stderr, "[Client] Error: Connection invalid\n");
        } else if (reply == XPC_ERROR_CONNECTION_INTERRUPTED) {
            fprintf(stderr, "[Client] Error: Connection interrupted\n");
        } else {
            fprintf(stderr, "[Client] Error: Request failed\n");
        }
    } else {
        const char *response = xpc_dictionary_get_string(reply, "response");
        const char *error = xpc_dictionary_get_string(reply, "error");
        int64_t result = xpc_dictionary_get_int64(reply, "result");
        int64_t pid = xpc_dictionary_get_int64(reply, "pid");

        if (response) {
            printf("[Client] Response: %s\n", response);
        } else if (error) {
            fprintf(stderr, "[Client] Error: %s\n", error);
        } else if (result != 0 || xpc_dictionary_get_value(reply, "result")) {
            printf("[Client] Result: %lld\n", result);
        } else if (pid != 0) {
            int64_t ppid = xpc_dictionary_get_int64(reply, "ppid");
            const char *status = xpc_dictionary_get_string(reply, "status");
            printf("[Client] Service PID: %lld, PPID: %lld, Status: %s\n",
                   pid, ppid, status ? status : "unknown");
        }
    }
    printf("\n");

    // Release the reply object (error constants don't need release)
    if (type != XPC_TYPE_ERROR) {
        xpc_release(reply);
    }
}

int main(int argc, char *argv[]) {
    // Disable output buffering to ensure messages appear immediately
    // This is especially helpful when debugging multi-process XPC applications
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("===========================================\n");
    printf("XPC Service Demo - Client Application\n");
    printf("===========================================\n\n");

    // Determine service name from command line or use default
    const char *service_name = "com.example.DemoService";
    if (argc > 1) {
        service_name = argv[1];
    }

    printf("[Client] Connecting to XPC service: %s\n", service_name);
    printf("[Client] Client PID: %d\n\n", getpid());

    // Create connection to the XPC service by name
    // The service must be in the app's XPCServices directory or be a system framework service
    xpc_connection_t connection = xpc_connection_create(service_name, NULL);

    if (!connection) {
        fprintf(stderr, "[Client] Error: Failed to create connection to service\n");
        return 1;
    }

    // Set up error handler for the connection
    xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
        xpc_type_t type = xpc_get_type(event);
        if (type == XPC_TYPE_ERROR) {
            if (event == XPC_ERROR_CONNECTION_INVALID) {
                fprintf(stderr, "[Client] Error: Service connection became invalid\n");
            } else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
                fprintf(stderr, "[Client] Error: Service connection interrupted (service may have crashed)\n");
            } else {
                fprintf(stderr, "[Client] Error: Service connection error\n");
            }
        } else {
            // Unexpected event in connection handler
            fprintf(stderr, "[Client] Error: Unexpected event in connection handler\n");
        }
    });

    // Activate the connection
    xpc_connection_resume(connection);

    printf("[Client] Connection established\n");
    printf("[Client] Service will launch on-demand if not running\n\n");

    // Give the service a moment to launch (launchd starts it on first message)
    // In production code, the first message send would trigger launch automatically
    sleep(1);

    // Send test messages
    xpc_object_t msg;

    // Test 1: Ping
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "ping");
    send_and_print(connection, msg, "ping");
    xpc_release(msg);

    // Test 2: Echo
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "echo");
    xpc_dictionary_set_string(msg, "data", "Hello from XPC client!");
    send_and_print(connection, msg, "echo 'Hello from XPC client!'");
    xpc_release(msg);

    // Test 3: Add numbers
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "add");
    xpc_dictionary_set_int64(msg, "a", 42);
    xpc_dictionary_set_int64(msg, "b", 23);
    send_and_print(connection, msg, "add 42 + 23");
    xpc_release(msg);

    // Test 4: Get service info
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "info");
    send_and_print(connection, msg, "get service info");
    xpc_release(msg);

    // Test 5: More echo
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "echo");
    xpc_dictionary_set_string(msg, "data", "XPC services are great!");
    send_and_print(connection, msg, "echo 'XPC services are great!'");
    xpc_release(msg);

    // Test 6: More math
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "add");
    xpc_dictionary_set_int64(msg, "a", 100);
    xpc_dictionary_set_int64(msg, "b", 200);
    send_and_print(connection, msg, "add 100 + 200");
    xpc_release(msg);

    // Test 7: Invalid message
    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "unknown");
    send_and_print(connection, msg, "unknown message type");
    xpc_release(msg);

    printf("[Client] All messages sent successfully!\n");
    printf("\n===========================================\n");
    printf("Demo complete!\n");
    printf("===========================================\n\n");

    printf("[Client] The XPC service will continue running for a while\n");
    printf("[Client] and will automatically exit when idle\n");

    // Clean up
    xpc_connection_cancel(connection);

    return 0;
}
