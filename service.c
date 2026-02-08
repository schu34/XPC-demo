#include <stdio.h>
#include <stdlib.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>

static void handle_message(xpc_connection_t peer, xpc_object_t message) {
    xpc_type_t type = xpc_get_type(message);

    if (type != XPC_TYPE_DICTIONARY) {
        fprintf(stderr, "Received non-dictionary message\n");
        return;
    }

    // Get the message type
    const char *msg_type = xpc_dictionary_get_string(message, "type");
    if (!msg_type) {
        fprintf(stderr, "Message has no type field\n");
        return;
    }

    printf("[Service] Received message of type: %s\n", msg_type);

    // Create reply
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

    } else {
        xpc_dictionary_set_string(reply, "error", "Unknown message type");
        fprintf(stderr, "[Service] Unknown message type: %s\n", msg_type);
    }

    // Send the reply
    xpc_connection_send_message(peer, reply);
}

static void handle_peer_event(xpc_connection_t peer, xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);

    if (type == XPC_TYPE_ERROR) {
        if (event == XPC_ERROR_CONNECTION_INVALID) {
            printf("[Service] Client disconnected\n");
        } else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
            printf("[Service] Termination imminent\n");
        } else {
            printf("[Service] Error: %s\n", xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION));
        }
    } else if (type == XPC_TYPE_DICTIONARY) {
        handle_message(peer, event);
    }
}

static void handle_new_connection(xpc_connection_t peer) {
    printf("[Service] New connection from PID: %d\n", xpc_connection_get_pid(peer));

    xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
        handle_peer_event(peer, event);
    });

    xpc_connection_resume(peer);
}

int main(int argc, char *argv[]) {
    printf("[Service] Starting XPC service (PID: %d)\n", getpid());

    // Create an anonymous listener connection
    xpc_connection_t listener = xpc_connection_create(NULL, NULL);

    if (!listener) {
        fprintf(stderr, "[Service] Failed to create listener\n");
        return 1;
    }

    // Create an endpoint that clients can use to connect
    xpc_endpoint_t endpoint = xpc_endpoint_create(listener);

    if (!endpoint) {
        fprintf(stderr, "[Service] Failed to create endpoint\n");
        return 1;
    }

    printf("[Service] Anonymous listener created\n");
    printf("[Service] Endpoint created: %p\n", endpoint);
    printf("[Service] To connect from client, you'll need to pass the endpoint\n");
    printf("[Service] For this demo, client will connect via fork/exec pattern\n");
    printf("[Service] Waiting for connections...\n\n");

    // Set up event handler for new connections
    xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
        if (xpc_get_type(peer) == XPC_TYPE_CONNECTION) {
            handle_new_connection((xpc_connection_t)peer);
        } else if (xpc_get_type(peer) == XPC_TYPE_ERROR) {
            printf("[Service] Listener error\n");
        }
    });

    xpc_connection_resume(listener);

    // Run the dispatch main loop
    dispatch_main();

    return 0;
}
