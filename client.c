#include "common.h"


int server_sock(const char serverip[],const char portnum[]);
int do_register(int sockfd,MSG *msg);
int do_login(int sockfd,MSG *msg);
int do_query(int sockfd,MSG *msg);
int do_history(int sockfd,MSG *msg);

int main(int argc,const char *argv[])
{
    int sockfd;
    MSG msg;
    if(argc != 3){
        printf("Usage:%s serverip port",argv[0]);
        return -1;
    }
    
    sockfd = server_sock(argv[1],argv[2]);
    while(1){
        int op;
        printf("-------------------------------\n");
        printf("1.register   2.login     3.quit \n");
        printf("-------------------------------\n");
        printf("Input the numebr of your option:\n");
        
        scanf("%d",&op);
        getchar();

        switch(op){
        case 1:
            do_register(sockfd,&msg);
            break;
        case 2:
            if(do_login(sockfd,&msg) == 1){
                goto next;
            }
            break;
        case 3:
            close(sockfd);
            exit(0);
            break;
        default:
            printf("Invalid option\n");
        }

    }
next:
    while(1){
        int op;
        printf("-------------------------------\n");
        printf("1.query   2.history    3.quit  \n");
        printf("-------------------------------\n");
   
        scanf("%d",&op);
        getchar();
        
            switch(op){
                case 1:
                    do_query(sockfd,&msg);
                    break;
                case 2:
                    do_history(sockfd,&msg);
                    break;
                case 3:
                    close(sockfd);
                    exit(0);
                    break;
                default:
                    printf("Invalid option\n");
            }
   
    }
}
int server_sock(const char serverip[],const char portnum[]){
    int sockfd;
    struct sockaddr_in server_addr; 
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socket");
        exit(-1);
    }
    
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverip);
    server_addr.sin_port = htons(atoi(portnum));

    if(connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0){
        perror("connerct");
        exit(-1);
    }
    return sockfd;
}

int do_register(int sockfd,MSG *msg){
    msg->type = R;
    printf("Input name:");
    scanf("%s",msg->name);
    getchar();

    printf("Input password:");
    scanf("%s",msg->data);
    getchar();

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        perror("send");
        exit(-1);
    }

    if(recv(sockfd,msg,sizeof(MSG),0) < 0){
        perror("recv");
        exit(-1);
    }

    printf("%s\n",msg->data);

    return 0;
}

int do_login(int sockfd,MSG *msg){
    msg->type = L;
   
    printf("Input name:");
    scanf("%s",msg->name);
    getchar();

    printf("Input password:");
    scanf("%s",msg->data);
    getchar();
    
    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        perror("send");
        exit(-1);
    }

    if(recv(sockfd,msg,sizeof(MSG),0) < 0){
        perror("recv");
        exit(-1);
    }

    if(strncmp(msg->data,"OK",3) == 0){
        printf("Login done.\n");
        return 1;
    }else{
        printf("%s\n",msg->data);
    }
    return 0;
}

int do_query(int sockfd,MSG *msg){
    msg->type = Q;
    printf("-----------------\n");
    while(1){
        printf("Input word:");
        scanf("%s",msg->data);
        if(strncmp(msg->data,"#",1) == 0){
            break;
        }
        if(send(sockfd,msg,sizeof(MSG),0) < 0){
            perror("send");
            exit(-1);
        }
        if(recv(sockfd,msg,sizeof(MSG),0) < 0){
            perror("recv");
            exit(-1);
        }

        printf("%s\n",msg->data);
    }
    return 0;
}

int do_history(int sockfd,MSG *msg){
    msg->type = H;
    
    printf("Input your name:");
    scanf("%s",msg->name);

    if(send(sockfd,msg,sizeof(MSG),0) < 0){
        perror("send");
        exit(-1);
    }
    while(1){
        if(recv(sockfd,msg,sizeof(MSG),0) < 0){
            perror("recv");
            exit(-1);
        }
        if(msg->data[0] == '\0'){
            break;
        }
        printf("%s",msg->data);
    }
    return 0;    
}
