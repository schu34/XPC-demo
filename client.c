#include <stdio.h>
#include <stdlib.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>

static void send_ping(xpc_connection_t connection) {
    printf("[Client] Sending ping...\n");

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(message, "type", "ping");

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR) {
        fprintf(stderr, "[Client] Error sending ping\n");
    } else {
        const char *response = xpc_dictionary_get_string(reply, "response");
        printf("[Client] Received: %s\n\n", response ? response : "(no response)");
    }
}

static void send_echo(xpc_connection_t connection, const char *text) {
    printf("[Client] Sending echo with text: '%s'\n", text);

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(message, "type", "echo");
    xpc_dictionary_set_string(message, "data", text);

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR) {
        fprintf(stderr, "[Client] Error sending echo\n");
    } else {
        const char *response = xpc_dictionary_get_string(reply, "response");
        const char *error = xpc_dictionary_get_string(reply, "error");

        if (response) {
            printf("[Client] Echo response: %s\n\n", response);
        } else if (error) {
            fprintf(stderr, "[Client] Error: %s\n\n", error);
        }
    }
}

static void send_add(xpc_connection_t connection, int64_t a, int64_t b) {
    printf("[Client] Sending add request: %lld + %lld\n", a, b);

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(message, "type", "add");
    xpc_dictionary_set_int64(message, "a", a);
    xpc_dictionary_set_int64(message, "b", b);

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR) {
        fprintf(stderr, "[Client] Error sending add\n");
    } else {
        int64_t result = xpc_dictionary_get_int64(reply, "result");
        printf("[Client] Result: %lld\n\n", result);
    }
}

static void send_info(xpc_connection_t connection) {
    printf("[Client] Requesting service info...\n");

    xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(message, "type", "info");

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR) {
        fprintf(stderr, "[Client] Error requesting info\n");
    } else {
        int64_t pid = xpc_dictionary_get_int64(reply, "pid");
        int64_t ppid = xpc_dictionary_get_int64(reply, "ppid");
        const char *status = xpc_dictionary_get_string(reply, "status");

        printf("[Client] Service PID: %lld\n", pid);
        printf("[Client] Service PPID: %lld\n", ppid);
        printf("[Client] Service Status: %s\n\n", status ? status : "unknown");
    }
}

void run_client(xpc_endpoint_t endpoint) {
    printf("[Client] Starting (PID: %d)\n", getpid());
    printf("[Client] Creating connection from endpoint...\n");

    // Create connection from the endpoint
    xpc_connection_t connection = xpc_connection_create_from_endpoint(endpoint);

    if (!connection) {
        fprintf(stderr, "[Client] Failed to create connection from endpoint\n");
        exit(1);
    }

    // Set up error handler
    xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
        xpc_type_t type = xpc_get_type(event);
        if (type == XPC_TYPE_ERROR) {
            if (event == XPC_ERROR_CONNECTION_INVALID) {
                fprintf(stderr, "[Client] Connection invalid\n");
            } else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
                fprintf(stderr, "[Client] Connection interrupted\n");
            } else {
                fprintf(stderr, "[Client] Connection error\n");
            }
        }
    });

    xpc_connection_resume(connection);

    printf("[Client] Connection established\n\n");

    // Run various tests
    send_ping(connection);
    send_echo(connection, "Hello, XPC!");
    send_add(connection, 42, 23);
    send_info(connection);

    // Send a few more messages
    send_echo(connection, "Testing inter-process communication");
    send_add(connection, 100, 200);
    send_ping(connection);

    printf("[Client] Demo complete. Cleaning up...\n");

    xpc_connection_cancel(connection);

    // Give a moment for cleanup
    sleep(1);
}

int main(int argc, char *argv[]) {
    // This client is designed to be called from the demo program
    // which will pass the endpoint

    fprintf(stderr, "[Client] This client should be called from the demo program\n");
    fprintf(stderr, "[Client] Run './demo' instead\n");

    return 1;
}
