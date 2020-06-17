#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <kademlia/common.h>
#include <kademlia/kademliaHost.h>
#include <boost/asio.hpp>
#include <tools/Logger.h>
#define SERV_PORT 3000
#define MAXLINE 4096

namespace Prometheus
{
    class rpcHost
    {
    public:
        rpcHost(std::shared_ptr<KademliaHost> host, Logger &l) : kadhost(host), logger(l) {}
        void run()
        {
            int socket_fd, connect_fd;
            pid_t pid;
            char *path = "console.ipc";
            struct sockaddr_un servaddr; /* 服务器端网络地址结构体 */
            char buf[MAXLINE], sendbuf[MAXLINE];
            memset(buf, 0, sizeof(buf));
            int len;
            /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/
            if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
            {
                printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
                exit(0);
            }
            /*初始化*/
            memset(&servaddr, 0, sizeof(servaddr)); /*数据初始化-清零 */
            servaddr.sun_family = AF_UNIX;          /*设置IPv4通信*/
            strcpy(servaddr.sun_path, path);        /*设置unix域文件名*/

            /*将本地地址绑定到所创建的套接字上*/
            if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
            {
                printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
                exit(0);
            }
            /*开始监听是否有客户端连接*/
            if (listen(socket_fd, 10) < 0)
            {
                printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
                exit(0);
            }
            printf("waiting for client's connection......\n");
            while (true)
            {
                /*阻塞直到有客户端连接，不然多浪费CPU资源*/
                if ((connect_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL)) < 0)
                {
                    printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
                    exit(1);
                }
                pid = fork();
                if (pid == 0)
                {
                    /*接受客户端传过来的数据*/
                    while (1)
                    {
                        if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                        {
                            buf[len] = '\0';
                            /*向客户端发送回应数据*/
                            //printf("receive message from client: %s\n", buf);
                            std::string choice(buf);
                            std::string nodeID;
                            std::string nodeAddress;
                            uint16_t udpPort;

                            choice.pop_back();
                            //printf("send message to client: \n");

                            if (choice == "SendFile")
                            {
                                std::string filePath;
                                printf("SendFile processed\n");
                                //向客户端请求NodeId
                                sprintf(sendbuf, "Please input NodeId:");
                                if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                {
                                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                    exit(0);
                                }
                                if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                                {
                                    buf[len] = '\0';
                                    printf("NodeId:%s", buf);
                                    nodeID = std::string(buf);
                                    memset(buf, 0, sizeof(buf));
                                }
                                sprintf(sendbuf, "Please input filepath:");
                                if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                {
                                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                    exit(0);
                                }
                                if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                                {
                                    buf[len - 1] = '\0';
                                    printf("filePath:%s", buf);
                                }
                                filePath = std::string(buf);
                                memset(buf, 0, sizeof(buf));
                                NodeID id = hex2char(nodeID);
                                std::shared_ptr<NodeInformation> tmp = kadhost->LookUpNodeInformation(id);
                                if (tmp == nullptr)
                                {
                                    sprintf(sendbuf, "Node doesn't exist!\n");
                                    if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                    {
                                        printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                        exit(0);
                                    }
                                }
                                else
                                {
                                    bool result = kadhost->SendFile(tmp, filePath);
                                    if (result == true)
                                    {
                                        sprintf(sendbuf, "File sent successfully.\n");
                                        if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                        {
                                            printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                            exit(0);
                                        }
                                    }
                                }
                            }
                            else if (choice == "addNode")
                            {
                                //向客户端请求NodeId
                                sprintf(sendbuf, "Please input NodeId:");
                                if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                {
                                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                    exit(0);
                                }
                                if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                                {
                                    buf[len] = '\0';
                                    printf("NodeId:%s", buf);
                                    nodeID = std::string(buf);
                                    memset(buf, 0, sizeof(buf));
                                }
                                //向客户端请求NodeAddress
                                sprintf(sendbuf, "Please input nodeAddress:");
                                if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                {
                                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                    exit(0);
                                }
                                if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                                {
                                    buf[len] = '\0';
                                    printf("NodeAddress:%s", buf);
                                    nodeAddress = std::string(buf);
                                    memset(buf, 0, sizeof(buf));
                                }
                                //向客户端请求UDPPort
                                sprintf(sendbuf, "Please input UDPPort:");
                                if (send(connect_fd, sendbuf, strlen(sendbuf), 0) < 0)
                                {
                                    printf("send messaeg error: %s(errno: %d)\n", strerror(errno), errno);
                                    exit(0);
                                }
                                if ((len = recv(connect_fd, buf, MAXLINE, 0)) > 0)
                                {
                                    buf[len] = '\0';
                                    printf("UDPPort:%s", buf);
                                    udpPort = std::stoi(std::string(buf));
                                    memset(buf, 0, sizeof(buf));
                                }
                                printf("addNode processed\n");
                                NodeID id = hex2char(nodeID);
                                NodeInformation *tmp = new NodeInformation(id, bi::address::from_string(nodeAddress), udpPort);
                                kadhost->addNode(std::make_shared<NodeInformation>(*tmp));
                            }
                            else
                            {
                                printf("wrong choice\n");
                                //printf("%s", choice.c_str());
                            }
                        }
                        sprintf(sendbuf, "Please input your choice:\n");
                        send(connect_fd, sendbuf, strlen(sendbuf), 0);
                    }
                    close(connect_fd);
                    close(socket_fd);
                    return;
                }
            }
        }

    private:
        std::shared_ptr<KademliaHost> kadhost;
        Logger logger;
    }; // namespace Prometheus

} //namespace Prometheus
