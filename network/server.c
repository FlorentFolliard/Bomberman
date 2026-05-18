#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    TestPacket packet;

    // 1. Initialisation de Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Échec de l'initialisation de Winsock. Code d'erreur : %d\n", WSAGetLastError());
        return 1;
    }

    // 2. Création du socket serveur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Échec de la création du socket. Code d'erreur : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 3. Configuration de l'adresse du serveur
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Écoute sur toutes les interfaces réseau (Ethernet inclus)
    address.sin_port = htons(DEFAULT_PORT);

    // 4. Liaison (Bind) du socket au port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Échec du Bind. Code d'erreur : %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 5. Mode écoute (Listen) - On accepte jusqu'à 3 connexions en attente
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Échec du Listen. Code d'erreur : %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("=========================================\n");
    printf(" SERVEUR BOMBERMAN - EN ATTENTE SUR LE PORT %d\n", DEFAULT_PORT);
    printf("=========================================\n");

    // 6. Boucle pour accepter les 3 clients un par un
    for (int i = 1; i <= 3; i++) {
        printf("\nEn attente du client %d/3...\n", i);
        
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            printf("Échec de l'acceptation du client %d. Code d'erreur : %d\n", i, WSAGetLastError());
            continue;
        }

        // Réception du paquet de test
        int bytes_received = recv(new_socket, (char *)&packet, sizeof(TestPacket), 0);
        if (bytes_received > 0) {
            printf("[REÇU] Client %d dit : \"%s\"\n", packet.client_id, packet.message);

            // Réponse au client
            sprintf(packet.message, "Bien reçu Client %d ! Connexion OK.", packet.client_id);
            send(new_socket, (char *)&packet, sizeof(TestPacket), 0);
            printf("[RÉPONSE] Accusé de réception envoyé au Client %d.\n", packet.client_id);
        }

        closesocket(new_socket);
    }

    printf("\n=========================================\n");
    printf("TEST TERMINÉ : Les 3 clients se sont connectés avec succès !\n");
    printf("=========================================\n");

    closesocket(server_fd);
    WSACleanup();
    
    system("pause");
    return 0;
}