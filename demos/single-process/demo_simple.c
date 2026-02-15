#include <stdio.h>
#include <stdlib.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <xpc/connection.h>
#include "../xpc_helpers.h"

// This demo shows XPC communication between two processes
// Instead of using anonymous connections (which are complex to share),
// we'll demonstrate the XPC API using a parent-child setup where
// they share the same dispatch queue

static void handle_message(xpc_connection_t peer, xpc_object_t message)
{
    xpc_type_t type = xpc_get_type(message);

    if (type != XPC_TYPE_DICTIONARY)
    {
        print_error("Service", "Received non-dictionary message");
        return;
    }

    const char *msg_type = xpc_dictionary_get_string(message, "type");
    if (!msg_type)
    {
        print_error("Service", "Message has no type field");
        return;
    }

    printf("[Service] Received message of type: %s\n", msg_type);

    xpc_object_t reply = xpc_dictionary_create_reply(message);

    if (strcmp(msg_type, "ping") == 0)
    {
        xpc_dictionary_set_string(reply, "response", "pong");
        printf("[Service] Responding with pong\n");
    }
    else if (strcmp(msg_type, "echo") == 0)
    {
        const char *data = xpc_dictionary_get_string(message, "data");
        if (data)
        {
            printf("[Service] Echoing: %s\n", data);
            xpc_dictionary_set_string(reply, "response", data);
        }
        else
        {
            xpc_dictionary_set_string(reply, "error", "No data to echo");
        }
    }
    else if (strcmp(msg_type, "add") == 0)
    {
        int64_t a = xpc_dictionary_get_int64(message, "a");
        int64_t b = xpc_dictionary_get_int64(message, "b");
        int64_t sum = a + b;

        printf("[Service] Adding %lld + %lld = %lld\n", a, b, sum);
        xpc_dictionary_set_int64(reply, "result", sum);
    }
    else if (strcmp(msg_type, "info") == 0)
    {
        xpc_dictionary_set_int64(reply, "pid", getpid());
        xpc_dictionary_set_int64(reply, "ppid", getppid());
        xpc_dictionary_set_string(reply, "status", "running");
        printf("[Service] Sending service info\n");
    }
    else
    {
        xpc_dictionary_set_string(reply, "error", "Unknown message type");
        print_error("Service", "Unknown message type: %s", msg_type);
    }

    xpc_connection_send_message(peer, reply);
}

static void handle_peer_event(xpc_connection_t peer, xpc_object_t event)
{
    xpc_type_t type = xpc_get_type(event);

    if (type == XPC_TYPE_ERROR)
    {
        if (event == XPC_ERROR_CONNECTION_INVALID)
        {
            printf("[Service] Client disconnected\n");
        }
        else if (event == XPC_ERROR_TERMINATION_IMMINENT)
        {
            printf("[Service] Termination imminent\n");
        }
    }
    else if (type == XPC_TYPE_DICTIONARY)
    {
        handle_message(peer, event);
    }
}

static void send_and_print(xpc_connection_t conn, xpc_object_t message, const char *desc)
{
    printf("[Client] Sending: %s\n", desc);

    xpc_object_t reply = xpc_connection_send_message_with_reply_sync(conn, message);

    xpc_type_t type = xpc_get_type(reply);
    if (type == XPC_TYPE_ERROR)
    {
        print_xpc_error("Client", reply);
    }
    else
    {
        const char *response = xpc_dictionary_get_string(reply, "response");
        const char *error = xpc_dictionary_get_string(reply, "error");
        int64_t result = xpc_dictionary_get_int64(reply, "result");
        int64_t pid = xpc_dictionary_get_int64(reply, "pid");

        if (response)
        {
            printf("[Client] Response: %s\n", response);
        }
        else if (error)
        {
            fprintf(stderr, "[Client] Error: %s\n", error);
        }
        else if (result != 0 || xpc_dictionary_get_value(reply, "result"))
        {
            printf("[Client] Result: %lld\n", result);
        }
        else if (pid != 0)
        {
            const char *status = xpc_dictionary_get_string(reply, "status");
            printf("[Client] Service PID: %lld, Status: %s\n", pid, status ? status : "unknown");
        }
    }
    printf("\n");
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    // Disable output buffering to ensure messages appear immediately
    // This is especially helpful when debugging or demonstrating async operations
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("===========================================\n");
    printf("XPC C API Demo - Single Process\n");
    printf("===========================================\n\n");

    printf("This demo shows the XPC C API in action.\n");
    printf("The service and client run in the SAME PROCESS, which makes it\n");
    printf("easier to understand XPC APIs without the complexity of separate\n");
    printf("processes. This is perfect for learning!\n\n");

    printf("For real inter-process XPC communication, you would typically:\n");
    printf("1. Use xpc_main() in an XPC service bundle (.xpc)\n");
    printf("2. Use xpc_connection_create(\"service.name\", queue) from client\n");
    printf("3. Or use xpc_connection_create_mach_service() with launchd\n\n");

    printf("Starting demonstration...\n\n");

    // Create anonymous listener
    printf("[Service] Creating anonymous XPC listener...\n");
    xpc_connection_t listener = xpc_connection_create(NULL, NULL);

    if (!listener)
    {
        print_error("Service", "Failed to create listener");
        return 1;
    }

    // Create endpoint from listener
    xpc_endpoint_t endpoint = xpc_endpoint_create(listener);

    if (!endpoint)
    {
        print_error("Service", "Failed to create endpoint");
        return 1;
    }

    printf("[Service] Endpoint created (PID: %d)\n", getpid());
    printf("[Service] Setting up event handler...\n");

    // Set up listener event handler
    xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
      if (xpc_get_type(peer) == XPC_TYPE_CONNECTION)
      {
          printf("[Service] New connection established\n");

          xpc_connection_set_event_handler((xpc_connection_t)peer, ^(xpc_object_t event) {
            handle_peer_event((xpc_connection_t)peer, event);
          });

          xpc_connection_resume((xpc_connection_t)peer);
      }
    });

    xpc_connection_resume(listener);
    printf("[Service] Listener active and waiting for connections...\n\n");

    // Create client connection from endpoint
    printf("[Client] Creating connection from endpoint (PID: %d)...\n", getpid());
    xpc_connection_t client = xpc_connection_create_from_endpoint(endpoint);

    if (!client)
    {
        print_error("Client", "Failed to create connection");
        return 1;
    }

    xpc_connection_set_event_handler(client, ^(xpc_object_t event) {
      xpc_type_t type = xpc_get_type(event);
      if (type == XPC_TYPE_ERROR)
      {
          print_xpc_error("Client", event);
      }
    });

    xpc_connection_resume(client);
    printf("[Client] Connection established\n\n");

    // Give the XPC runtime a moment to set up event handlers
    // In production code, you would rely on connection state callbacks instead
    sleep(1);

    // Send test messages
    xpc_object_t msg;

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "ping");
    send_and_print(client, msg, "ping");

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "echo");
    xpc_dictionary_set_string(msg, "data", "Hello, XPC!");
    send_and_print(client, msg, "echo 'Hello, XPC!'");

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "add");
    xpc_dictionary_set_int64(msg, "a", 42);
    xpc_dictionary_set_int64(msg, "b", 23);
    send_and_print(client, msg, "add 42 + 23");

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "info");
    send_and_print(client, msg, "get service info");

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "echo");
    xpc_dictionary_set_string(msg, "data", "Testing XPC dictionaries");
    send_and_print(client, msg, "echo 'Testing XPC dictionaries'");

    msg = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(msg, "type", "add");
    xpc_dictionary_set_int64(msg, "a", 100);
    xpc_dictionary_set_int64(msg, "b", 200);
    send_and_print(client, msg, "add 100 + 200");

    printf("[Client] All messages sent successfully!\n");
    printf("\n===========================================\n");
    printf("Demo complete!\n");
    printf("===========================================\n");

    xpc_connection_cancel(client);
    xpc_connection_cancel(listener);

    return 0;
}
