/*
 * protocol.h
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//  Compatibilità Cross-Platform
#if defined _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
<<<<<<< HEAD
=======
    // Definiamo 'close' come 'closesocket' per uniformitï¿½
>>>>>>> b4c1d5b71e5a89d5ca4ce93d5de237b402c33b50
    #define close closesocket
    #define strcasecmp _stricmp
    void clear_winsock() { WSACleanup(); }
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

// Costanti di Rete
#define DEFAULT_PORT 56700
<<<<<<< HEAD
=======
// Ho mantenuto l'IP originale, ma useremo 127.0.0.1 per il test locale
>>>>>>> b4c1d5b71e5a89d5ca4ce93d5de237b402c33b50
#define DEFAULT_IP "127.0.0.1"
#define BUFFER_SIZE 512
#define CITY_LEN 64


// Messaggio di Richiesta (Client -> Server)
typedef struct {
<<<<<<< HEAD
    char type;           // 't', 'h', 'w', 'p'
    char city[CITY_LEN]; // Nome della città
=======
    char type;         // 't', 'h', 'w', 'p'
    char city[CITY_LEN]; // Nome cittï¿½ (terminato da \0)
>>>>>>> b4c1d5b71e5a89d5ca4ce93d5de237b402c33b50
} weather_request_t;

// Messaggio di Risposta (Server -> Client)
typedef struct {
<<<<<<< HEAD
    unsigned int status; // 0=OK, 1=No Città, 2=Tipo non valido
    char type;           // Eco del tipo di dato richiesto
    float value;         // Il valore meteo
=======
    unsigned int status; // 0=OK, 1=No Cittï¿½, 2=Invalid Type
    char type;           // Eco della richiesta
    float value;         // Valore meteo
>>>>>>> b4c1d5b71e5a89d5ca4ce93d5de237b402c33b50
} weather_response_t;

// Funzioni ---
float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

#endif /* PROTOCOL_H_ */
