#include <stdio.h>
#include <xpc/xpc.h>

int main()
{
    /* printf("Hello, World!\n"); */
    /* return 0; */

    xpc_connection_t connection = xpc_connection_create(NULL, NULL);
    xpc_endpoint_t endpoint = xpc_endpoint_create(connection);

    xpc_connection_t new_connection = xpc_connection_create_from_endpoint(endpoint, NULL);

    xpc_connection_send_message_with_reply(new_connection, xpc_dictionary_create(NULL, NULL, 0), NULL, ^(xpc_object_t reply) {
      printf("Received reply\n");
    });
}
