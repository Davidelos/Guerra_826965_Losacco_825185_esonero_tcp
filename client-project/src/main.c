#include "protocol.h"

// Funzione main del Client
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); // No buffering

    // --- 1. Variabili di Configurazione e Parsing ---
    char server_ip[CITY_LEN] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    weather_request_t req;
    memset(&req, 0, sizeof(req));
    int request_provided = 0;

    // Parsing Argomenti
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

            // Logica robusta per: type city
            req.type = input_str[0];

            // Trova il primo spazio
            char* space_ptr = strchr(input_str, ' ');

            if (space_ptr != NULL) {
                // Se c'è uno spazio, la città inizia dopo lo spazio
                strncpy(req.city, space_ptr + 1, CITY_LEN - 1);
            } else {
                // Se non c'è spazio, la città inizia dal secondo carattere (e.g. "troma")
                // Se la stringa è lunga almeno 2 caratteri, copiamo dalla posizione 1
                if (strlen(input_str) > 1) {
                    strncpy(req.city, input_str + 1, CITY_LEN - 1);
                }
            }
            req.city[CITY_LEN - 1] = '\0'; // Assicura la terminazione
            request_provided = 1;
            i++;
        }
    }

    if (!request_provided) {
        printf("Errore: Parametro -r obbligatorio.\n");
        printf("Uso: %s [-s server_ip] [-p port] -r \"type city\"\n", argv[0]);
        return -1;
    }

    // DEBUG:
    // printf("Richiesta preparata: type='%c', city='%s', IP='%s'\n", req.type, req.city, server_ip);


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
        #if defined _WIN32
            WSACleanup();
        #endif
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
    printf("Connessione stabilita con successo a %s:%d\n", server_ip, port);


    // --- 5. Invio Richiesta (Send) ---
    if (send(client_socket, (char*)&req, sizeof(req), 0) != sizeof(req)) {
        printf("Errore invio richiesta.\n");
        close(client_socket);
        #if defined _WIN32
             WSACleanup();
        #endif
        return -1;
    }

    // --- 6. Ricezione Risposta (Recv) ---
    weather_response_t resp;
    int bytes_recv = recv(client_socket, (char*)&resp, sizeof(resp), 0);

    if (bytes_recv <= 0) {
        printf("Errore ricezione o server chiuso.\n");
        close(client_socket);
        #if defined _WIN32
             WSACleanup();
        #endif
        return -1;
    }

    // --- 7. Stampa Formattata del Risultato ---
    printf("Ricevuto risultato dal server ip %s. ", server_ip);

    if (resp.status == 1) {
        printf("Città non disponibile\n");
    } else if (resp.status == 2) {
        printf("Richiesta non valida\n");
    } else if (resp.status == 0) {
        printf("%s: ", req.city);
        switch (resp.type) {
            case 't': printf("Temperatura = %.1f°C\n", resp.value); break;
            case 'h': printf("Umidità = %.1f%%\n", resp.value); break;
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
