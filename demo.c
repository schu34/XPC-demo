#include <stdio.h>
#include <stdlib.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

// Service functions (from service.c)
static void handle_message(xpc_connection_t peer, xpc_object_t message) {
    xpc_type_t type = xpc_get_type(message);

    if (type != XPC_TYPE_DICTIONARY) {
        fprintf(stderr, "Received non-dictionary message\n");
        return;
    }

    const char *msg_type = xpc_dictionary_get_string(message, "type");
    if (!msg_type) {
        fprintf(stderr, "Message has no type field\n");
        return;
    }

    printf("[Service] Received message of type: %s\n", msg_type);

    xpc_object_t reply = xpc_dictionary_create_reply(message);

    if (strcmp(msg_type, "ping") == 0) {
        xpc_dictionary_set_string(reply, "response", "pong");
        printf("[Service] Responding with pong\n");

    } else if (strcmp(msg_type, "echo") == 0) {
        const char *data = xpc_dictionary_get_string(message, "data");
        if (data) {
            printf("[Service] Echoing: %s\n", data);
            xpc_dictionary_set_string(reply, "response", data);
        } else {
            xpc_dictionary_set_string(reply, "error", "No data to echo");
        }

    } else if (strcmp(msg_type, "add") == 0) {
        int64_t a = xpc_dictionary_get_int64(message, "a");
        int64_t b = xpc_dictionary_get_int64(message, "b");
        int64_t sum = a + b;

        printf("[Service] Adding %lld + %lld = %lld\n", a, b, sum);
        xpc_dictionary_set_int64(reply, "result", sum);

    } else if (strcmp(msg_type, "info") == 0) {
        xpc_dictionary_set_int64(reply, "pid", getpid());
        xpc_dictionary_set_int64(reply, "ppid", getppid());
        xpc_dictionary_set_string(reply, "status", "running");
        printf("[Service] Sending service info\n");

    } else {
        xpc_dictionary_set_string(reply, "error", "Unknown message type");
        fprintf(stderr, "[Service] Unknown message type: %s\n", msg_type);
    }

    xpc_connection_send_message(peer, reply);
}

static void handle_peer_event(xpc_connection_t peer, xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);

    if (type == XPC_TYPE_ERROR) {
        if (event == XPC_ERROR_CONNECTION_INVALID) {
            printf("[Service] Client disconnected\n");
        } else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
            printf("[Service] Termination imminent\n");
        }
    } else if (type == XPC_TYPE_DICTIONARY) {
        handle_message(peer, event);
    }
}

static void handle_new_connection(xpc_connection_t peer) {
    printf("[Service] New connection attempt...\n");
    printf("[Service] Peer type: %p\n", xpc_get_type(peer));
    printf("[Service] New connection from PID: %d\n", xpc_connection_get_pid(peer));

    xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
        printf("[Service] Received event in peer handler\n");
        handle_peer_event(peer, event);
    });

    xpc_connection_resume(peer);
    printf("[Service] Connection resumed\n");
}


// Client functions (from client.c)
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

    xpc_connection_t connection = xpc_connection_create_from_endpoint(endpoint);

    if (!connection) {
        fprintf(stderr, "[Client] Failed to create connection from endpoint\n");
        exit(1);
    }

    xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
        xpc_type_t type = xpc_get_type(event);
        printf("[Client] Received event, type: %p\n", type);
        if (type == XPC_TYPE_ERROR) {
            if (event == XPC_ERROR_CONNECTION_INVALID) {
                fprintf(stderr, "[Client] Connection invalid\n");
            } else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
                fprintf(stderr, "[Client] Connection interrupted\n");
            } else {
                fprintf(stderr, "[Client] Connection error (unknown)\n");
            }
        } else {
            printf("[Client] Received non-error event\n");
        }
    });

    printf("[Client] Resuming connection...\n");
    xpc_connection_resume(connection);

    printf("[Client] Connection established\n\n");

    // Give time for connection to be fully established
    sleep(1);

    // Run various tests
    send_ping(connection);
    send_echo(connection, "Hello, XPC!");
    send_add(connection, 42, 23);
    send_info(connection);

    send_echo(connection, "Testing inter-process communication");
    send_add(connection, 100, 200);
    send_ping(connection);

    printf("[Client] Demo complete. Cleaning up...\n");

    xpc_connection_cancel(connection);
    sleep(1);
}

// Global endpoint shared between parent and children via fork
static xpc_endpoint_t g_endpoint = NULL;
static xpc_connection_t g_listener = NULL;

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    // Make stdout unbuffered so we can see output from all processes
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("===========================================\n");
    printf("XPC C API Demo - Two Process Communication\n");
    printf("===========================================\n\n");

    // Create listener and endpoint in parent process BEFORE forking
    // so both children can access it
    printf("[Main] Creating anonymous XPC listener...\n");
    g_listener = xpc_connection_create(NULL, NULL);

    if (!g_listener) {
        fprintf(stderr, "[Main] Failed to create listener\n");
        return 1;
    }

    g_endpoint = xpc_endpoint_create(g_listener);

    if (!g_endpoint) {
        fprintf(stderr, "[Main] Failed to create endpoint\n");
        return 1;
    }

    printf("[Main] Endpoint created successfully\n\n");

    // Fork service process
    pid_t service_pid = fork();

    if (service_pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (service_pid == 0) {
        // Child process - run service
        // Service will use the global g_listener
        printf("[Service] Starting XPC service (PID: %d)\n", getpid());
        printf("[Service] Waiting for connections...\n\n");

        xpc_connection_set_event_handler(g_listener, ^(xpc_object_t peer) {
            if (xpc_get_type(peer) == XPC_TYPE_CONNECTION) {
                handle_new_connection((xpc_connection_t)peer);
            }
        });

        xpc_connection_resume(g_listener);
        dispatch_main(); // Service runs forever
        exit(0);
    }

    // Parent process
    printf("[Main] Service started with PID: %d\n", service_pid);
    printf("[Main] Waiting for service to initialize...\n\n");

    // Give service time to start and setup event handlers
    sleep(2);

    // Fork client process
    pid_t client_pid = fork();

    if (client_pid == -1) {
        perror("fork failed");
        kill(service_pid, SIGTERM);
        return 1;
    }

    if (client_pid == 0) {
        // Child process - run client
        // Client will use the global g_endpoint
        run_client(g_endpoint);
        exit(0);
    }

    // Parent - wait for client to finish
    printf("[Main] Client started with PID: %d\n\n", client_pid);

    int status;
    waitpid(client_pid, &status, 0);

    printf("\n[Main] Client finished. Shutting down service...\n");

    // Kill service
    kill(service_pid, SIGTERM);
    waitpid(service_pid, &status, 0);

    printf("[Main] Demo complete!\n");

    return 0;
}
