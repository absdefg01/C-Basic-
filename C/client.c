#include  <stdio.h>
#include  <unistd.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <stdlib.h>
#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXBUFFERLEN 1024
int main(int argc, char* argv[]){
    int descSock;                 // Descripteur de la socket
    int ecode;                    // Retour des fonctions
    struct addrinfo *res;         // Résultat de la focntion getaddrinfo
    struct addrinfo hints = {     // Cette structure permet de contrôler l'exécution de la fonction getaddrinfo
         0,
         AF_INET,    //seule les adresses IPv4 seront présentées par la fonctiongetaddrinfo
         SOCK_STREAM,
         0,
         0,
         NULL,
         NULL,
         NULL
    };
    char serverName[MAXHOSTLEN]; // Nom de la machine serveur
    char serverPort[MAXPORTLEN]; // Numéro de port
    char buffer[MAXBUFFERLEN];     // buffer stockant les messages échangés entre le client et le serveur
    //On teste les valeurs rentrées par l'utilisateur
    if (argc != 3){ perror("Mauvaise utilisation de la commande: <nom serveur> <numero de port>\n"); exit(1);}
    if (strlen(argv[1]) >= MAXHOSTLEN){ perror("Le nom de la machine serveur est trop long\n"); exit(2);}
    if (strlen(argv[2]) >= MAXPORTLEN){ perror("Le numero de port du serveur est trop long\n"); exit(2);}
    strncpy(serverName, argv[1], MAXHOSTLEN);
    serverName[MAXHOSTLEN-1] = '\0';
    strncpy(serverPort, argv[2], MAXPORTLEN);
    serverPort[MAXPORTLEN-1] = '\0';
    //Création de la socket IPv4/TCP
    descSock = socket(AF_INET, SOCK_STREAM, 0);
    if (descSock == -1) {
         perror("Erreur creation socket");
      exit(4);
  }
  //Récupération des informations sur le serveur
  ecode = getaddrinfo(serverName,serverPort,&hints,&res);
  if (ecode){
      fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
      exit(3);
  }
  //Connexion au serveur
  ecode = connect(descSock, res->ai_addr, res->ai_addrlen);
  if (ecode == -1) {
      close(descSock);
      freeaddrinfo(res);
      perror("Erreur connect");
      exit(5);
  }
  freeaddrinfo(res);
  //Echange de donneés avec le serveur
  ecode = read(descSock, buffer, MAXBUFFERLEN);
  if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
  buffer[ecode] = '\0';
  printf("MESSAGE RECU DU SERVEUR: \"%s\".\n",buffer);
  //Fermeture de la socket
  close(descSock);
}

