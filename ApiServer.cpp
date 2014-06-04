/*
 * ApiServer.cpp
 *
 *  Created on: May 24, 2014
 *      Author: anass
 */

#include "ApiServer.h"
namespace net{

char param[33]={'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','\0'};
pthread_mutex_t mutexf = PTHREAD_MUTEX_INITIALIZER;

ApiServer::ApiServer(short port) {
	this->initialize(port);
}

ApiServer::~ApiServer() {
	// TODO Auto-generated destructor stub
}

void ApiServer::initialize(short port){
	master = Create_Socket_Server_TCP(port);
}

int ApiServer::start(){
	puts("Waiting for incoming connections...");
	while( (slave = accept(master, NULL, 0) ) ){
	puts("Connection accepted");
	short id = 0;
		while( (read( slave, &id, sizeof( id ) )) ){

			id = ntohs(id);
			switch(id){
			case REPLY_SEND:
					{
						char vec[4];
						read(slave,vec,4);
						puts("wsel vec");
						pthread_mutex_lock(&mutexf);
						writeInXmlFile(vec);
						pthread_mutex_unlock(&mutexf);
					};break;
			case REPLY_GET:
					{
						char vec[4] = {'1','1','1','0'};
					    write(slave,vec,4);
					} ;break;
			}

		}
	}

		if (slave < 0)
			perror("accept failed");

		puts("End Server");

		return 0;
	}

void ApiServer::writeInXmlFile(char *vec){
	std::ofstream myfile;
	myfile.open("webfiles/testbit.xml");

	if(myfile.is_open()){
	myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	       <<  "<bits>\n";

	for(int i=0;i<4;i++){
		myfile << "<bit id=\"" << i << "\" value=\"" << vec[i] << "\" />\n";
	}

	myfile << "</bits>\n";
	myfile.close();

}
else std::cout << "Unable to open file";
}

int ApiServer::Create_Socket_Server_TCP(short port) {
	int socket_desc;
	struct sockaddr_in server;

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		throw("Could not create socket");
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		throw("bind failed");
	}
	puts("bind done");

	//Listen
	if (listen(socket_desc, 3) < 0)
		throw("listen failed");

	return socket_desc;
}

void ApiServer::Write_vector_str(int slave,std::vector<std::string> vec){
	long length = htonl( vec.size() );
	write( slave, &length, sizeof(length) );
	for ( int i = 0; i < vec.size(); ++i ) {
	    length = htonl( vec[i].length() );
	    write( slave, &length, sizeof(length) );
	    write( slave, vec[i].data(), vec[i].length() );
	}
}

void ApiServer::Write_word32(int slave,char* vec){
	int len = send(slave,vec,33,0);
	if (len == -1)
			throw("Writer Error");
}

}
