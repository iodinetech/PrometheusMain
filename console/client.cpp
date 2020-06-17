#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>

#define MAXLINE 1024

int main(int argc, char *argv[])
{
    char sendbuf[MAXLINE], receivebuf[MAXLINE];
    char *path = "console.ipc";
    struct sockaddr_un servaddr;
    int client_sockfd;
    int rec_len;
    int exit_flag = 0;
    /* 创建客户端套接字--UNIX域套接字，面向连接通信*/
    if ((client_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(0);
    }
    /* 初始化 */
    memset(&servaddr, 0, sizeof(servaddr)); /* 数据初始化-清零 */
    servaddr.sun_family = AF_UNIX;          /* 设置UNIX域通信 */
    strcpy(servaddr.sun_path,path);   /* 设置套接字文件名 */
    /* 将套接字绑定到服务器的网络地址上*/
    if (connect(client_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connected failed");
        exit(0);
    }
    /* 循环发送接收数据，send发送数据，recv接收数据 */
    printf("Please input your choice\n>>");

    while (1)
    {
        memset(sendbuf,0,sizeof(sendbuf));
        fgets(sendbuf, 1024, stdin);
        /* 向服务器端发送数据 */
        if(!strcmp(sendbuf,"exit\n"))
        {
            close(client_sockfd);
            return 0;
        }
        if (send(client_sockfd, sendbuf, strlen(sendbuf), 0) < 0)
        {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
        if(exit_flag == 1)
        {

        }
        /* 接受服务器端传过来的数据 */
        if ((rec_len = recv(client_sockfd, receivebuf, MAXLINE, 0)) == -1)
        {
            perror("recv error");
            exit(1);
        }
        receivebuf[rec_len] = '\0';
        printf("%s\n>>", receivebuf);
    }
    /* 关闭套接字 */
    close(client_sockfd);
    return 0;
}
