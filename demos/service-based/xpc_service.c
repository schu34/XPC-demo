#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpc/xpc.h>
#include <stdarg.h>
#include "../xpc_helpers.h"

// XPC Service implementation using xpc_main()
// This is the standard pattern for XPC services bundled within an application

static void handle_message(xpc_connection_t peer, xpc_object_t message) {
    xpc_type_t type = xpc_get_type(message);

    if (type != XPC_TYPE_DICTIONARY) {
        print_error("Service", "Received non-dictionary message");
        return;
    }

    const char *msg_type = xpc_dictionary_get_string(message, "type");
    if (!msg_type) {
        print_error("Service", "Message has no type field");
        return;
    }

    printf("[Service] Received message of type: %s\n", msg_type);

    // Create reply dictionary
    xpc_object_t reply = xpc_dictionary_create_reply(message);

    if (strcmp(msg_type, "ping") == 0) {
        // Simple ping-pong
        xpc_dictionary_set_string(reply, "response", "pong");
        printf("[Service] Responding with pong\n");

    } else if (strcmp(msg_type, "echo") == 0) {
        // Echo back the data
        const char *data = xpc_dictionary_get_string(message, "data");
        if (data) {
            printf("[Service] Echoing: %s\n", data);
            xpc_dictionary_set_string(reply, "response", data);
        } else {
            xpc_dictionary_set_string(reply, "error", "No data to echo");
        }

    } else if (strcmp(msg_type, "add") == 0) {
        // Add two numbers
        int64_t a = xpc_dictionary_get_int64(message, "a");
        int64_t b = xpc_dictionary_get_int64(message, "b");
        int64_t sum = a + b;

        printf("[Service] Adding %lld + %lld = %lld\n", a, b, sum);
        xpc_dictionary_set_int64(reply, "result", sum);

    } else if (strcmp(msg_type, "info") == 0) {
        // Return service information
        xpc_dictionary_set_int64(reply, "pid", getpid());
        xpc_dictionary_set_int64(reply, "ppid", getppid());
        xpc_dictionary_set_string(reply, "status", "running");
        printf("[Service] Sending service info\n");

    } else if (strcmp(msg_type, "shutdown") == 0) {
        // Allow client to request shutdown
        xpc_dictionary_set_string(reply, "response", "shutting down");
        printf("[Service] Shutdown requested\n");
        xpc_connection_send_message(peer, reply);

        // Exit after sending reply
        exit(0);

    } else {
        xpc_dictionary_set_string(reply, "error", "Unknown message type");
        print_error("Service", "Unknown message type: %s", msg_type);
    }

    // Send the reply
    xpc_connection_send_message(peer, reply);
    xpc_release(reply);
}

static void handle_peer_event(xpc_connection_t peer, xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);

    if (type == XPC_TYPE_ERROR) {
        if (event == XPC_ERROR_CONNECTION_INVALID) {
            printf("[Service] Client disconnected\n");
        } else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
            printf("[Service] Termination imminent\n");
        } else {
            printf("[Service] Error on connection\n");
        }
    } else if (type == XPC_TYPE_DICTIONARY) {
        handle_message(peer, event);
    } else {
        print_error("Service", "Unexpected event type");
    }
}

static void connection_handler(xpc_connection_t peer) {
    // This handler is called when a new connection is established
    printf("[Service] New connection established from PID: %d\n", xpc_connection_get_pid(peer));

    // Set up event handler for messages on this connection
    xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
        handle_peer_event(peer, event);
    });

    // Resume the connection to start receiving events
    xpc_connection_resume(peer);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    // Disable output buffering to ensure log messages appear immediately
    // Note: Service stdout/stderr go to unified logging (visible in Console.app)
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("[Service] XPC Service starting (PID: %d)\n", getpid());
    printf("[Service] Calling xpc_main()...\n");

    // xpc_main() initializes the XPC service runtime and starts listening for connections
    // It never returns - the connection_handler will be called for each new connection
    xpc_main(connection_handler);

    // This should never be reached
    print_error("Service", "xpc_main() returned unexpectedly");
    return 1;
}
