/*
 * StartingServer.cpp
 *
 *  Created on: May 24, 2014
 *      Author: anass
 */

#include "HttpServer.h"
#include "ApiServer.h"
#include <pthread.h>

using namespace net;

static void *HundleConx(void *a)
{
	HttpServer *h = reinterpret_cast<HttpServer *>(a);
    h->start();
    return 0;
}

static void *RunHttpServer(void *a)
{
	HttpServer *h = reinterpret_cast<HttpServer *>(a);
	pthread_t t1;
	while(1){
			pthread_create(&t1,NULL,HundleConx,h);

	}
    return 0;
}

static void *RunApiServer(void *a)
{
	ApiServer *h = reinterpret_cast<ApiServer *>(a);
    h->start();
    return 0;
}

int main(int argc, char *argv[]) {

	HttpServer webServ(9090);
	ApiServer raspberryServer(8888);
	pthread_t t1,t2;

	pthread_create(&t1,NULL,RunHttpServer,&webServ);
	pthread_create(&t2,NULL,RunApiServer,&raspberryServer);

	pthread_join(t1,NULL);
	pthread_join(t2,NULL);

	return 0;
}
