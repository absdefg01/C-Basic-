// Inclusion des bibliothèques
#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// Déclaration des constantes
#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille du tampon de demande de connexion
#define MAXBUFFERLEN 1024
#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXLOGLEN 64
#define MAXMACHLEN 64

// fonction de communication
void * communiquer(void * descSockCOM);

// programme principal
int main(){

	// Déclaration des variables locales
    int ecode;                       // Code retour des fonctions serveur
    char serverAddr[MAXHOSTLEN];     // Adresse du serveur
    char serverPort[MAXPORTLEN];     // Port du server
    int descSockRDV;                 // Descripteur de socket de rendez-vous
    int descSockCOM;                 // Descripteur de socket de communication
    struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les 
				     // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur

	pthread_t id;
	int err;

	//
	// Initialisation d'un socket de connexion entrante
	//

		// Initialisation de la socket de RDV IPv4/TCP
		descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
		if (descSockRDV == -1) {
		     perror("Erreur création socket RDV\n");
		     exit(2);
		}

		// Publication de la socket au niveau du système
		// Assignation d'une adresse IP et un numéro de port
		// Mise à zéro de hints
		memset(&hints, 0, sizeof(hints));
		// Initailisation de hints
		hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
		hints.ai_socktype = SOCK_STREAM;  // TCP
		hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
						 				 // la fonction getaddrinfo

		 // Récupération des informations du serveur
		 ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
		 if (ecode) {
		     fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
		     exit(1);
		 }
		 // Publication de la socket
		 ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
		 if (ecode == -1) {
		     perror("Erreur liaison de la socket de RDV");
		     exit(3);
		 }
		 // Nous n'avons plus besoin de cette liste chainée addrinfo
		 freeaddrinfo(res);

		 // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran
		 len=sizeof(struct sockaddr_storage);
		 ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
		 if (ecode == -1)
		 {
		     perror("SERVEUR: getsockname");
		     exit(4);
		 }
		 ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
		                     serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
		 if (ecode != 0) {
		         fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
		         exit(4);
		 }
		 printf("L'adresse d'ecoute est: %s\n", serverAddr);
		 printf("Le port d'ecoute est: %s\n", serverPort);

	//
	// Mise en attente de connexion
	//

		 // Definition de la taille du tampon contenant les demandes de connexion
		 ecode = listen(descSockRDV, LISTENLEN);
		 if (ecode == -1) {
		     perror("Erreur initialisation buffer d'écoute");
		     exit(5);
		 }

	//
	// connexion d'un client
	//
		do {
			len = sizeof(struct sockaddr_storage);
		 	// Attente connexion du client
		 	// Lorsque demande de connexion, creation d'une socket de communication avec le client
		 	descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
		 	if (descSockCOM == -1){
			    perror("Erreur accept\n");
			    exit(6);
			}
		 	err=pthread_create(&id,NULL,communiquer,(void *)&descSockCOM);
		 	if (err!=0)
		 	{
				perror("ECHEC CREATION THREAD");
		 	}
		} while (1>0);


	close(descSockRDV);
}

void * communiquer(void * descSockCOM)
{
	char commande[MAXLOGLEN];		// identifiant serveur
	char machine[MAXMACHLEN];   // identifiant machine
    int descSockSERV;                 // Descripteur de socket de serveur
	int ecode;

	struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les 
				     // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur

	// Echange de données avec le client connecté
	strcpy(buffer, "220 Server FTP Bienvenue ^^\n");
	write(*((int *)descSockCOM), buffer, strlen(buffer));
		
	//
	//	Récupération des données serveur FTP à contacter
	//

		// lecture
  		ecode = read(*((int *)descSockCOM), buffer, MAXBUFFERLEN);
  		if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
  		buffer[ecode] = '\0';

		// recuperation dans la chaine
		sscanf(buffer,"%[^@]@%[^\r]",commande,machine);
		
	//
	//	Connexion au serveur distant
	//

		//Création de la socket IPv4/TCP
		descSockSERV = socket(AF_INET, SOCK_STREAM, 0);
		if (descSockSERV == -1) {
		     perror("Erreur creation socket");
		  exit(8);
		}

		//Récupération des informations sur le serveur	
		ecode = getaddrinfo(machine,"21",&hints,&res);
		if (ecode){
			fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
			exit(9);
		}

		//Connexion au serveur
		ecode = connect(descSockSERV, res->ai_addr, res->ai_addrlen);
		if (ecode == -1) {
			close(descSockSERV);
			freeaddrinfo(res);
			perror("Erreur connect");
			exit(10);
		}
		freeaddrinfo(res);

		ecode = read(descSockSERV, buffer, MAXBUFFERLEN);
	    if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
	    buffer[ecode] = '\0';

		// identification
		strcpy(buffer,commande);
		strcat(buffer,"\n");
		write(descSockSERV, buffer, strlen(buffer));

		ecode = read(descSockSERV, buffer, MAXBUFFERLEN);
	    if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
	    buffer[ecode] = '\0';
	    write(*((int *)descSockCOM), buffer, strlen(buffer));

		ecode = read(*((int *)descSockCOM), buffer, MAXBUFFERLEN);
	    if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
	    buffer[ecode] = '\0';
		
	    write(descSockSERV, buffer, strlen(buffer));

		ecode = read(descSockSERV, buffer, MAXBUFFERLEN);
	    if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
	    buffer[ecode] = '\0';
	    write(*((int *)descSockCOM), buffer, strlen(buffer));

	//
	// Boucle de conversation
	//

		while (0<1) {

			// lecture demande client
  			ecode = read(*((int *)descSockCOM), buffer, MAXBUFFERLEN);
  			if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
  			buffer[ecode] = '\0';

			// envoi des données au serveur distant
			write(descSockSERV, buffer, strlen(buffer));

			// renvoi des données recues du serveur au client
			ecode = read(descSockSERV, buffer, MAXBUFFERLEN);
	   		if (ecode == -1) {perror("Problème de lecture\n"); exit(6);}
	   		buffer[ecode] = '\0';
	   		write(*((int *)descSockCOM), buffer, strlen(buffer));
		}

	//
	// Fin
	//

	//Fermeture de la connexion
	close(*((int *)descSockCOM));
}
