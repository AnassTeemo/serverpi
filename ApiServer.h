/*
 * ApiServer.h
 *
 *  Created on: May 24, 2014
 *      Author: anass
 */

#ifndef APISERVER_H_
#define APISERVER_H_

#include<stdio.h>
#include<string.h>    //strlen
#include <iostream>
#include <vector>
#include <fstream>

#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

namespace net{

class ApiServer {
public:
	ApiServer(short port);
	virtual ~ApiServer();

	void initialize(short port);
	int start();

private:
	int Create_Socket_Server_TCP(short port);
	void Write_vector_str(int slave,std::vector<std::string>);
	void Write_word32(int slave,char* vec);
	void writeInXmlFile(char*);

private:
	int master,slave;
	static const short REPLY_SEND    = 1;
	static const short REPLY_GET     = 2;

};

}

#endif /* APISERVER_H_ */
