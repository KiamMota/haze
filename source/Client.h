#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#include "proto/RawBuffer.h"
#include "proto/Response.h"

typedef struct HazeClient HazeClient;

/* Cria uma conexão com o servidor */
HazeClient *HazeClientNew(
    const char *host,
    int port
);

/* Fecha e libera o cliente */
void HazeClientFree(
    HazeClient **client
);

/*
 * Executa uma função remota.
 *
 * method: nome da função RPC
 * params: parâmetros já serializados em MessagePack
 *
 * Retorna a resposta do servidor.
 */
Response *HazeClientCall(
    HazeClient *client,
    const char *method,
    RawBuffer *params
);

#endif
