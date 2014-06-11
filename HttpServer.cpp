/*
 * HttpServer.cpp
 *
 *  Created on: May 24, 2014
 *      Author: anass
 */

#include "HttpServer.h"
pthread_mutex_t mutexf = PTHREAD_MUTEX_INITIALIZER;

void start(){
		int listen_fd;
		int client;
		pthread_t id;

		listen_fd=cree_socket_ecoute(9090);

		while (1) {
			client = accept(listen_fd, NULL, 0);
			if (client==-1) {
				if (errno!=EINTR && errno!=ECONNABORTED) exit_error("échec de accept");
			} else {
				errno = pthread_create(&id, NULL, traite_connexion, (void*)client);
				if (errno) exit_error("échec de pthread_create");
				errno = pthread_detach(id);
				if (errno) exit_error("échec de pthread_detach");
			}

		}
}

int cree_socket_ecoute(int port) {
	int listen_fd;
	struct sockaddr_in addr;
	int one = 1;
	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd==-1) exit_error("échec de socket");
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))==-1)
		exit_error("échec de setsockopt(SO_REUSEADDR)");
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listen_fd, (const struct sockaddr*) &addr, sizeof(addr))==-1)
		exit_error("échec de bind");
	if (listen(listen_fd, SOMAXCONN)==-1)
		exit_error("échec de listen");
	fprintf(stderr, "Serveur actif sur le port %i\n", port);
	return(listen_fd);
}

void fin_connexion(FILE* stream,const char* msg) {
	fprintf(stderr, "fin de connexion (%s, errno=%s)\n", msg, strerror(errno));
	fclose(stream);
	pthread_exit(NULL);
}

void my_fgets(char* buf, int size, FILE* stream) {
	if (!fgets(buf, size-1, stream)) fin_connexion(stream, "échec de fgets");
}

void lit_get(FILE* stream, char* url, int size) {
	char buf[4096];
	int i,j;

	my_fgets(buf, sizeof(buf), stream);
	if (strncmp(buf, "GET", 3)) fin_connexion(stream, "la requête n'est pas GET");
	for (i=3; buf[i]==' '; i++);
	if (!strncmp(buf+i, "http://", 7)) i+=7;
	for (; buf[i] && buf[i]!='/'; i++);
	if (buf[i]=='/') i++;
	for (j=0; buf[i] && buf[i]!=' ' && j<size-1; j++, i++) url[j] = buf[i];
	url[j] = 0;
	for (; buf[i]==' '; i++);
	if (strncmp(buf+i, "HTTP/1.1", 8))
		fin_connexion(stream, "la version n'est pas HTTP/1.1");

}

int lit_en_tetes(FILE* stream) {
	char buf[4096];
	int keepalive = 1;
	while (1) {
		my_fgets(buf, sizeof(buf), stream);
		if (buf[0]=='\n' || buf[0]=='\r') break;
		if (strncasecmp(buf, "Connection:", 11) || strstr(buf, "close")) //g enlevé le ! devant strncasecmp
			keepalive = 0;
	}
	return(keepalive);
}

void envoie_404(FILE* stream, char* url) {
	fprintf(stream, "HTTP/1.1 404 Not found\r\n");
	fprintf(stream, "Connection: close\r\n");
	fprintf(stream, "Content-type: text/html\r\n");
	fprintf(stream, "\r\n");
	fprintf(stream,
		"<html><head><title>Not Found</title></head>"
		"<body><p>Sorry, the object you requested was not found: "
		"<tt>/%s</tt>.</body></html>\r\n",
		url);
	fin_connexion(stream, "erreur 404");
}

char* type_fichier(char* chemin) {
	int len = strlen(chemin);
	if (!strcasecmp(chemin+len-5, ".html") ||
		!strcasecmp(chemin+len-4, ".htm")) return "text/html";
	if (!strcasecmp(chemin+len-4, ".css")) return "text/css";
	if (!strcasecmp(chemin+len-4, ".xml")) return "text/xml";
	if (!strcasecmp(chemin+len-4, ".png")) return "image/png";
	if (!strcasecmp(chemin+len-4, ".gif")) return "image/gif";
	if (!strcasecmp(chemin+len-5, ".jpeg") ||
		!strcasecmp(chemin+len-4, ".jpg")) return "image/jpeg";
	return("text/ascii");
}

void envoie_fichier(FILE* stream, char* chemin, int keepalive) {
	char modiftime[30];
	char curtime[30];
	struct timeval  tv;
	char buf[4096];
	struct stat s;
	int fd;
	char errmsg[4096];
	if (strstr(chemin,"..")) {
		envoie_404(stream, chemin); return;
	}
	fd = open(chemin, O_RDONLY);
	if (fd==-1) {
		snprintf(errmsg,sizeof(errmsg),"open(«%s»)",chemin);
		envoie_404(stream, chemin);
		return;
	}
	if (fstat(fd, &s)==-1 || !S_ISREG(s.st_mode) || !(s.st_mode & S_IROTH)) {
		close(fd);
		envoie_404(stream, chemin);
		return;
	}
	if (gettimeofday(&tv, NULL) || !ctime_r(&s.st_mtime, modiftime) ||
		!ctime_r(&tv.tv_sec, curtime)) {
		close(fd);
		envoie_404(stream, chemin);
		return;
	}
	modiftime[strlen(modiftime)-1] = 0;
	curtime[strlen(curtime)-1] = 0;
	fprintf(stream, "HTTP/1.1 200 OK\r\n");
	fprintf(stream, "Connection: %s\r\n", keepalive ? "keep-alive" : "close");
	fprintf(stream, "Content-length: %li\r\n", (long)s.st_size);
	fprintf(stream, "Content-type: %s\r\n", type_fichier(chemin));
	fprintf(stream, "Date: %s\r\n", curtime);
	fprintf(stream, "Last-modified: %s\r\n", modiftime);
	fprintf(stream, "\r\n");
	while (1) {
		int r = read(fd, buf, sizeof(buf)), w;
		if (r==0) break;
		if (r<0) {
			if (errno==EINTR) continue;
			close(fd);
			fin_connexion(stream, "échec de read");
		}
		for (w=0; w<r; ) {
			int a = fwrite(buf+w, 1, r-w, stream);
			if (a<=0) {
				if (errno==EINTR) continue;
				close(fd);
				fin_connexion(stream, "échec de write");
			}
			w += a;
		}
	}
	close(fd);
}

int get_param(char *url,char* param){

	std::string surl(url),rest="NO REST!";
	int pos = surl.find("?");

	if(pos!=std::string::npos){
	std::string str1 = surl.substr (0,pos);
	strcpy(url, str1.c_str());
	rest = surl.substr (pos+1,std::string::npos);
	for(int j=0;j<32;j++) param[j]='0';
	for (unsigned epos = rest.find("="),pos=-1;
			 epos!= std::string::npos || pos!= std::string::npos;
			 epos = rest.find("=", epos + 1),pos = rest.find("&", pos + 1) )
		{
			param[( atoi(( rest.substr(pos+1, epos-pos-1)).c_str() ) ) - 1]= '1';
		}
	  return 1;
	}
	return 0;

}

void* traite_connexion(void* arg) {
	int slave = (int)arg;
	FILE* stream = fdopen(slave, "r+");
	char url[4096];
	int keepalive = 1;

	errno = 0;
	setlinebuf(stream);
	while (keepalive){
 		lit_get(stream, url, sizeof(url));

		printf("URL: %s\n",url);

		if(!strcasecmp(url,"SEND_BYTE")){
			char buffer[128],vec[32];

			my_fgets(buffer,sizeof(buffer),stream);

			memcpy(vec,buffer,32);

			for(int j=0;j<32;j++)printf("%c ",vec[j]);
			pthread_mutex_lock(&mutexf);
			writeInXmlFile(vec);
			pthread_mutex_unlock(&mutexf);
			puts("wsel vec");
		}
		else if (!strcasecmp(url,"laal")) return 0;
		else {
			keepalive = lit_en_tetes(stream);
			pthread_mutex_lock(&mutexf);
			envoie_fichier(stream, url, keepalive);
			pthread_mutex_unlock(&mutexf);

		}

	}

	fin_connexion(stream, "ok");
	return(NULL);

}

void exit_error(const char *chaine) {
	if (errno!=0) perror(chaine);
	else printf("ERREUR %s\n",chaine);
	exit(EXIT_FAILURE);
}

void writeInXmlFile(char *vec){
	std::ofstream myfile;
	myfile.open("webfiles/testbit.xml");

	if(myfile.is_open()){
	myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	       <<  "<bits>\n";

	for(int i=0;i<32;i++){
		myfile << "<bit id=\"" << i << "\" value=\"" << vec[i] << "\" />\n";
	}

	myfile << "</bits>\n";
	myfile.close();

}
}
