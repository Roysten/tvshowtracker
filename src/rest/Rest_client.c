#include "Rest_client.h"

#include <stdio.h>

#include "../Logger.h"

bool REST_CLIENT_create(Rest_client *client, size_t recv_buf_len)
{
    *client = (Rest_client){0};

    client->buf_len = recv_buf_len;
    client->buf = calloc(recv_buf_len, 1);
    if (client->buf == NULL) {
        REST_CLIENT_destroy(client);
        return false;
    }

    client->inet_handle = InternetOpen(L"TVShowTracer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (client->inet_handle == NULL) {
        REST_CLIENT_destroy(client);
        return false;
    }

    return true;
}

void REST_CLIENT_destroy(Rest_client *client)
{
    if (client->buf != NULL) {
        free(client->buf);
        client->buf = NULL;
    }

    REST_CLIENT_disconnect(client);

    if (client->inet_handle != NULL) {
        InternetCloseHandle(client->inet_handle);
        client->inet_handle = NULL;
    }
}

bool REST_CLIENT_connect(Rest_client *client, const wchar_t *host) {
    if (client->conn_handle != NULL) {
        InternetCloseHandle(client->conn_handle);
    }
    
    client->conn_handle = InternetConnect(client->inet_handle, host, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR) NULL);
    if (client->conn_handle == NULL) {
        return false;
    }

    return true;
}

void REST_CLIENT_disconnect(Rest_client *client) {
    if (client->conn_handle != NULL) {
        InternetCloseHandle(client->conn_handle);
        client->conn_handle = NULL;
    }
}

bool REST_CLIENT_get_resource(Rest_client *client, const wchar_t *uri, const wchar_t *method)
{
    if (client->conn_handle == NULL) {
        return false;
    }

    PCTSTR content_type[] = {L"application/json", NULL};
    HINTERNET req_handle = HttpOpenRequest(client->conn_handle, method, uri, NULL, NULL, content_type, INTERNET_FLAG_SECURE, (DWORD_PTR) NULL);
    if (req_handle == NULL) {
        return false;
    }
    
    bool send_ok = HttpSendRequest(req_handle, NULL, 0, 0, 0);
    if (!send_ok) {
        LOG("Sending HTTP request failed");
        InternetCloseHandle(req_handle);
        return false;
    }

    DWORD bytes_read = 0;
    bool read_ok = InternetReadFile(req_handle, client->buf, client->buf_len, &bytes_read);
    InternetCloseHandle(req_handle);
    return read_ok && bytes_read > 0;
}

void REST_CLIENT_urlencode( const wchar_t *to_encode, wchar_t *encoded_buf)
{
    wchar_t c = to_encode[0];
    size_t idx = 0;
    size_t buf_offset = 0;
    while (c != L'\0') {
        if ((c >= L'0' && c <= L'9')
            || (c >= L'A' && c <= L'Z')
            || (c >= L'a' && c <= L'z')
            || c == L'-'
            || c == L'_'
            || c == L'.'
            || c == L'~') {
            encoded_buf[buf_offset++] = c;
        } else {
            wsprintf(&encoded_buf[buf_offset], L"%%%02X", c);
            buf_offset += 3;
        }
        c = to_encode[++idx];
    }
    encoded_buf[buf_offset] = L'\0';
}
