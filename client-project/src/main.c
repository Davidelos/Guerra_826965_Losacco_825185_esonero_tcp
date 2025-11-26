#include "protocol.h"

static int send_n(int sock, const char* buffer, size_t length, int flags) {
    size_t total_sent = 0;
    int bytes_sent_this_turn;
    while (total_sent < length) {
        bytes_sent_this_turn = send(sock, buffer + total_sent, length - total_sent, flags);
        if (bytes_sent_this_turn <= 0) return bytes_sent_this_turn;
        total_sent += bytes_sent_this_turn;
    }
    return total_sent;
}

static int recv_n(int sock, char* buffer, size_t length, int flags) {
    size_t total_received = 0;
    int bytes_received_this_turn;
    while (total_received < length) {
        bytes_received_this_turn = recv(sock, buffer + total_received, length - total_received, flags);
        if (bytes_received_this_turn <= 0) return bytes_received_this_turn;
        total_received += bytes_received_this_turn;
    }
    return total_received;
}


int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    char server_ip[CITY_LEN] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    weather_request_t req;
    memset(&req, 0, sizeof(req));
    int request_provided = 0;

    // Parsing Argomenti (rimosso per brevità)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(server_ip, argv[i + 1], CITY_LEN - 1);
            server_ip[CITY_LEN - 1] = '\0';
            i++;
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            char* input_str = argv[i + 1];
            req.type = input_str[0];

            char* space_ptr = strchr(input_str, ' ');
            if (space_ptr != NULL) {
                strncpy(req.city, space_ptr + 1, CITY_LEN - 1);
            } else if (strlen(input_str) > 1) {
                strncpy(req.city, input_str + 1, CITY_LEN - 1);
            }
            req.city[CITY_LEN - 1] = '\0';
            request_provided = 1;
            i++;
        }
    }

    if (!request_provided) {
        printf("Uso: [..]\n");
        return -1;
    }

    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) return -1;
    #endif

    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket error");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore connessione a %s:%d\n", server_ip, port);
        close(client_socket);
        #if defined _WIN32
            WSACleanup();
        #endif
        return -1;
    }

    if (send_n(client_socket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
        printf("Errore invio.\n");
        close(client_socket);
        return -1;
    }

    weather_response_t resp;
    if (recv_n(client_socket, (char*)&resp, sizeof(resp), 0) != sizeof(resp)) {
        printf("Errore ricezione o timeout.\n");
        close(client_socket);
        return -1;
    }

    if (resp.status == 1) {
        printf("Città non disponibile\n");
    } else if (resp.status == 2) {
        printf("Richiesta non valida\n");
    } else if (resp.status == 0) {
        switch (resp.type) {
            case 't': printf("Temperatura = %.1f C\n", resp.value); break;
            case 'h': printf("Umidità = %.1f%%\n", resp.value); break;
            case 'w': printf("Vento = %.1f km/h\n", resp.value); break;
            case 'p': printf("Pressione = %.1f hPa\n", resp.value); break;
            default: printf("Dato sconosciuto = %.1f\n", resp.value);
        }
    } else {
        printf("Ricevuto status sconosciuto (%d).\n", resp.status);
    }

    close(client_socket);
    #if defined _WIN32
        WSACleanup();
    #endif
    return 0;
}
