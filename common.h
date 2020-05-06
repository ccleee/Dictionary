#ifndef __COMMON_H__
#define __COMMON_H__
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>
 
#define   N  32
typedef struct {
	int type;
	char name[N];
	char data[256];
}MSG;
 
#define  R  1   // register
#define  L  2   // login
#define  Q  3   // query
#define  H  4   // history
 
#define  DATABASE  "dict.db" 
 
 
#define SERADDR "127.0.0.1"   
#define SERPORT 5001          
 
#endif
