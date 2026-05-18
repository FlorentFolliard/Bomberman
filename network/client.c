#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    TestPacket packet;
    char server_ip[100];

    // 1. Initialisation de Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Échec de l'initialisation de Winsock.\n");
        return 1;
    }

    // Configuration manuelle pour le test sur le switch
    printf("Entrez l'adresse IP du Serveur (ex: 192.168.1.XX) : ");
    scanf("%99s", server_ip);
    
    printf("Entrez votre numéro de client (1, 2 ou 3) : ");
    scanf("%d", &packet.client_id);

    // Préparation du message
    sprintf(packet.message, "Hello depuis le PC Client %d !", packet.client_id);

    // 2. Création du socket client
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Échec de la création du socket.\n");
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DEFAULT_PORT);

    // 3. Conversion de l'adresse IP (Version corrigée)
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        printf("Format de l'adresse IP invalide.\n");
        closesocket(sock);
        WSACleanup();
        system("pause");
        return 1;
    }

    printf("\nTentative de connexion à %s:%d...\n", server_ip, DEFAULT_PORT);

    // 4. Connexion au serveur
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        printf("CONNEXION ÉCHOUÉE ! Vérifiez l'IP, le switch ou le Pare-feu Windows.\n");
        closesocket(sock);
        WSACleanup();
        system("pause");
        return 1;
    }

    printf("[SUCCÈS] Connecté au serveur !\n");

    // 5. Envoi du paquet de test
    send(sock, (char *)&packet, sizeof(TestPacket), 0);
    printf("[ENVOI] Message envoyé au serveur.\n");

    // 6. Attente de la réponse du serveur
    int bytes_received = recv(sock, (char *)&packet, sizeof(TestPacket), 0);
    if (bytes_received > 0) {
        printf("[RÉPONSE DU SERVEUR] : \"%s\"\n", packet.message);
    } else {
        printf("Le serveur a fermé la connexion prématurément.\n");
    }

    closesocket(sock);
    WSACleanup();

    printf("\nTest réussi avec succès !\n");
    system("pause");
    return 0;
}