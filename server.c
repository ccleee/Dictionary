#include "common.h"
#define MAX_QUE 10

int server_sock(const char serverip[],const char portnum[]);
int do_register(int acceptfd,MSG *msg,sqlite3 *db);
int do_login(int acceptfd,MSG *msg,sqlite3 *db);
int do_query(int acceptfd,MSG *msg,sqlite3 *db);
int do_history(int acceptfd,MSG *msg,sqlite3 *db);
int do_client(int acceptfd,sqlite3 *db);
int H_callback(void *arg,int f_num,char **f_value,char **f_name);
void get_date(char *date);
int do_search(int acceptfd,MSG *msg,char word[]);

int main(int argc,const char *argv[])
{
    int sockfd,acceptfd;
    pid_t pid;
    MSG msg;
    sqlite3 *db;

    if(argc != 3){
        printf("Usage:%s serverip port",argv[0]);
        return -1;
    }
    sockfd = server_sock(argv[1],argv[2]);
    if(sqlite3_open(DATABASE,&db) != SQLITE_OK){
        printf("%s\n",sqlite3_errmsg(db));
        exit(-1);
    }
    else{
        printf("open database success.\n");
    }

    signal(SIGCHLD,SIG_IGN); 
    while(1){
        if((acceptfd = accept(sockfd,NULL,NULL)) < 0){
            perror("accept");
            exit(-1);
        }
        if((pid = fork()) < 0){
            perror("fork");
            exit(-1);
        }
        else if(pid == 0){
            close(sockfd);
            do_client(acceptfd,db);
        }
        else{
            close(acceptfd);
        }
    }
    return 0;    
}

int server_sock(const char serverip[],const char portnum[]){
    int sockfd;
    int on = 1;
    struct sockaddr_in server_addr; 
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socket");
        exit(-1);
    }
    
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverip);
    server_addr.sin_port = htons(atoi(portnum));

    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int)) < 0){
        perror("setsockopt:");
        exit(-1);
    }
    if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0){
        perror("bind");
        exit(-1);
    }

    if(listen(sockfd,MAX_QUE) < 0){
        perror("listen");
        exit(-1);
    }
    return sockfd;
}

int do_client(int acceptfd,sqlite3 *db){
    MSG msg;
    while(recv(acceptfd,&msg,sizeof(msg),0) > 0){
        switch(msg.type){
            case R:
                do_register(acceptfd,&msg,db);
                break;
            case L:
                do_login(acceptfd,&msg,db);
                break;
            case H:
                do_history(acceptfd,&msg,db);
                break;
            case Q:
                do_query(acceptfd,&msg,db);
                break;
            default:
                printf("Invalid option.\n");
        }
    }
    printf("client exit.\n");
    close(acceptfd);
    exit(0);

    return 0;
}


int do_register(int acceptfd,MSG *msg,sqlite3 *db){
    char *errmsg;
    char sql[128];
    sprintf(sql,"insert into usr values('%s',%s);",msg->name,msg->data);
    printf("%s\n",sql);

    if((sqlite3_exec(db,sql,NULL,NULL,&errmsg)) != SQLITE_OK){
        printf("%s\n",errmsg);
        strcpy(msg->data,"usr name already exist");
        exit(-1);
    }
    else{
        printf("register done.\n");
        strcpy(msg->data,"OK!");
    }
    if(send(acceptfd,msg,sizeof(MSG),0) < 0){
        perror("send");
        exit(-1);
    }
    return 0;
}

int do_login(int acceptfd,MSG *msg,sqlite3 *db){
    char sql[128];
    char *errmsg;
    int nrow;
    int ncloumn;
    char **resultp;
    sprintf(sql,"select * from usr where name = '%s' and pass = '%s';",msg->name,msg->data);
    printf("%s\n",sql);

    if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg) != SQLITE_OK){
        printf("%s\n",errmsg);
        exit(-1);
    }
    else{
        printf("get_table success.\n");
    }

    if(nrow == 1){
        strcpy(msg->data,"OK");
        send(acceptfd,msg,sizeof(MSG),0);
        return 1;
    }
    else{
        strcpy(msg->data,"usrname/password wrong.\n");
        send(acceptfd,msg,sizeof(MSG),0);
    }
    return 0;
}

int do_search(int acceptfd,MSG *msg,char word[]){
    FILE *fp;
    int len = 0;
    char temp[512]={};
    int result;
    char *p;

    if((fp = fopen("EtoR.txt","r")) == NULL){
        perror("fopen:");
        strcpy(msg->data,"Fail to open dictionary.\n");
        send(acceptfd,msg,sizeof(MSG),0);
        return -1;
    }
    
    len = strlen(word);
    printf("%s,len = %d\n",word,len);
    while(fgets(temp,512,fp) != NULL){
        result = strncmp(temp,word,len);
        
        if(result < 0){
            continue;
        }
        if(result > 0 || ((result==0)&&(temp[len] != ' '))){
            break;
        }
        
        p = temp + len;
        while(*p == ' '){
            p++;
        }
        strcpy(msg->data,p);
        fclose(fp);
        return 1;
    }
    return 0;
    fclose(fp);
}

void get_date(char *date){
    time_t t;
    struct tm *tp;
    time(&t);
    tp = localtime(&t);

    sprintf(date,"%d-%d-%d %d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,tp->tm_mday,
           tp->tm_hour,tp->tm_min,tp->tm_sec);

}
int do_query(int acceptfd,MSG *msg,sqlite3 *db){
    char sql[128];
    char *errmsg;
    int found = 0;

    char word[64];
    strcpy(word,msg->data);

    char date[64];
    found = do_search(acceptfd,msg,word);

    if(found == 1){
        get_date(date);
        sprintf(sql,"insert into record values('%s','%s','%s');",msg->name,date,word);
        printf("%s",sql);

        if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) < 0){
            printf("%s\n",errmsg);
            exit(-1);
        }
    }
    else{
        strcpy(msg->data,"not found");
    }
    send(acceptfd,msg,sizeof(MSG),0);
}
void print_res(){

}
int H_callback(void *arg,int f_num,char **f_value,char **f_name){
    
    int acceptfd = *((int*)arg);
    MSG msg;

    sprintf(msg.data,"%s  %s",f_value[1],f_value[2]);
    send(acceptfd,&msg,sizeof(MSG),0);
   
    return 0;
}
int  do_history(int acceptfd,MSG *msg,sqlite3 *db){
    char sql[128];
    char *errmsg;
    
    sprintf(sql,"select * from record where name='%s';",msg->name);
    if(sqlite3_exec(db,sql,H_callback,(void*)&acceptfd,&errmsg) != SQLITE_OK){
        printf("%s\n",errmsg);
        exit(-1);
    }else{
        printf("History found.\n");
    }
    msg->data[0] = '\0';
    send(acceptfd,msg,sizeof(MSG),0);

}
