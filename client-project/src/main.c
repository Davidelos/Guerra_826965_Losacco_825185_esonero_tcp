#include "protocol.h"

// Funzione main
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); // No buffering

    // --- 1. Variabili di Configurazione e Parsing ---
    char server_ip[32] = DEFAULT_IP;
    int port = DEFAULT_PORT;         // 56700
    weather_request_t req;
    memset(&req, 0, sizeof(req));
    int request_provided = 0;

    // Parsing Argomenti (uguale a prima)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(server_ip, argv[i + 1], 31);
            i++;
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            char* input_str = argv[i + 1];
            req.type = input_str[0]; // Primo char è il tipo
            char* space_ptr = strchr(input_str, ' ');
            if (space_ptr != NULL) {
                strncpy(req.city, space_ptr + 1, 63);
            } else {
                if (strlen(input_str) > 1) strncpy(req.city, input_str + 1, 63);
            }
            request_provided = 1;
            i++;
        }
    }

    if (!request_provided) {
        printf("Errore: Parametro -r obbligatorio.\n");
        printf("Uso: %s [-s server_ip] [-p port] -r \"type city\"\n", argv[0]);
        return -1;
    }

    // --- 2. Inizializzazione Winsock (Windows) ---
    #if defined _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            printf("Errore: WSAStartup fallita.\n");
            return -1;
        }
    #endif

    // --- 3. Creazione Socket (TCP) ---
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Errore creazione socket.\n");
        return -1;
    }

    // --- 4. Connessione al Server (Connect) ---
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore: Connessione al server %s:%d fallita.\n", server_ip, port);
        close(client_socket);
        #if defined _WIN32
            WSACleanup();
        #endif
        return -1;
    }

    // --- 5. Invio Richiesta (Send) ---
    if (send(client_socket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
        printf("Errore invio richiesta.\n");
        close(client_socket);
        return -1;
    }

    // --- 6. Ricezione Risposta (Recv) ---
    weather_response_t resp;
    int bytes_recv = recv(client_socket, (char*)&resp, sizeof(resp), 0);

    if (bytes_recv <= 0) {
        printf("Errore ricezione o server chiuso.\n");
        close(client_socket);
        return -1;
    }

    // --- 7. Stampa Formattata del Risultato ---
    printf("Ricevuto risultato dal server ip %s. ", server_ip);

    if (resp.status == 1) {
        printf("Città non disponibile\n");
    } else if (resp.status == 2) {
        printf("Richiesta non valida\n");
    } else if (resp.status == 0) {
        // Formattazione specifica richiesta dall'esonero
        // Bari: Temperatura = 22.5°C
        // Nota: %.1f significa "stampa float con 1 cifra decimale"
        printf("%s: ", req.city);
        switch (resp.type) {
            case 't': printf("Temperatura = %.1f°C\n", resp.value); break;
            case 'h': printf("Umidità = %.1f%%\n", resp.value); break; // %% stampa %
            case 'w': printf("Vento = %.1f km/h\n", resp.value); break;
            case 'p': printf("Pressione = %.1f hPa\n", resp.value); break;
            default:  printf("Dato sconosciuto = %.1f\n", resp.value); break;
        }
    } else {
        printf("Status sconosciuto ricevuto dal server.\n");
    }

    // --- 8. Chiusura ---
    close(client_socket);
    #if defined _WIN32
        WSACleanup();
    #endif
        return 0;
}
