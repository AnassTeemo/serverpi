/*
 * HttpServer.h
 *
 *  Created on: May 24, 2014
 *      Author: anass
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <pthread.h>



	extern char param[32];
	extern pthread_mutex_t mutexf;


	void start();

	int cree_socket_ecoute(int port);
	void fin_connexion(FILE* stream,const char* msg);
	void my_fgets(char* buf, int size, FILE* stream);
	void lit_get(FILE* stream, char* url, int size);
	int lit_en_tetes(FILE* stream);
	void envoie_404(FILE* stream, char* url);
	char* type_fichier(char* chemin);
	void envoie_fichier(FILE* stream, char* chemin, int keepalive);
	int get_param(char *url,char* param);
	void* traite_connexion(void*);
	void exit_error(const char *chaine);
	void writeInXmlFile(char *vec);





#endif /* HTTPSERVER_H_ */
