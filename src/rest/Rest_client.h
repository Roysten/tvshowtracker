#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>
#include <wininet.h>

struct Rest_client
{
    char *buf;
    size_t buf_len;
    HINTERNET inet_handle;
    HINTERNET conn_handle;
};
typedef struct Rest_client Rest_client;

bool REST_CLIENT_create( Rest_client *client, size_t recv_buf_len );

void REST_CLIENT_destroy( Rest_client *client );

/**
 * Calls InternetConnect for the provided host. Note that it does not actually
 * connects to the remove host, it merely creates a handle. You only need to
 * call this function once for each rest client.
 */
bool REST_CLIENT_connect(Rest_client *client, const wchar_t *host);

void REST_CLIENT_disconnect(Rest_client *client);

/**
 * Does a request. Expect an established connection (see REST_CLIENT_connect).
 */
bool REST_CLIENT_get_resource(Rest_client *client, const wchar_t *uri, const wchar_t *method);

void REST_CLIENT_urlencode(const wchar_t *to_encode, wchar_t *encoded_buf);
