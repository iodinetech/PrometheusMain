# PrometheusMain
西安电子科技大学 开源项目 “ProjectPrometheus（普罗米修斯计划）”主程序

<a href="https://github.com/iodinetech/PrometheusMain/blob/master/introduction.pdf">简介 PPT</a><br>
若无法查看，请改善您的网络环境。  
或，可尝试将本仓库代码整体下载到本地后进行查看。  
由于最近实在太忙，该简介比较简略，稍后会更新详细版本，敬请关注！  


| Operating system & Architecture | Status     |
| ------------------------------- | ---------- |
| Ubuntu 18.04 x86-64             | Build Pass |
| Debian 10 AArch64               | Build Pass |

使用时需要配置两个文件:config.json和network.json.文件格式示例在代码的根目录.
config.json 设置本节点的属性，network.json设置网络中其他节点的属性。

# 编译指引：
开启终端，执行下述命令，进行环境搭建和项目编译。  
1.apt-get install libspdlog-dev  
2.apt-get install libcrypto++-dev  
3.apt-get install libboost-all-dev  
4.apt-get install libprotobuf-dev  
5.apt-get install protobuf-compiler  
6.apt-get install cmake  
6.在代码树的根目录: mkdir build  
7.cd build  
8.cmake ..  
9.make  
10.主程序为build/kadfiletranporter,控制台为build/console/rpcclient  

testenv里是demo用的三个节点。编译成功后将生成的两个可执行文件放在三个目录中，每个目录中一份。然后启动三个程序就可以观察到节点之间互相通信。
使用控制台可以通过AddPeer命令添加节点，通过SendFile命令向想要的节点发送文件。执行这两个命令后按照提示操作即可。

# 关于体验版
要体验 0.8.1 的功能，只需要将编译后 ./build/kadfiletransporter 和 ./build/console/rpcclient 文件夹中的两个二进制文件分别放入 testEnv 文件夹中的三个 test 文件夹，再分别在三个文件夹中打开终端，启动 kadfiletransporter 即可看到三个节点已经开始相互通信了。

各个 test 文件夹中的两个 json 文件均是用于建立测试环境的。三个文件夹就相当于三个节点，我们称这种运作模式为“伪分布式”
其中：

1. config.json 描述了当前的节点信息：
   - id：id 是一个 256 位的字符串，它唯一标识了一个 Prometheus 网络中的节点。您可以自己使用 SHA256 来生成自己喜欢的串，但我们建议使用随机生成的值来作为 id，防止碰撞；
   - ip：运行该节点的硬件（物理硬件或逻辑硬件）的 ip 地址。由于该测试环境中均是本机运行，所以填写本地地址即可；
   - port：Prometheus p2p 模块所监听的端口地址。由于我们要建立伪分布式网络，所有节点运行在同一物理机上，所以三个文件夹内 config 中的该字段均不同。
2. network.json 用于为我们的测试节点提供节点列表——每个节点在自己的该文件中填写想要连接到的其他节点即可。这里面的三个值的含义与 config.json 中的同名字段完全相同。


如果想要使用控制台，请在节点启动后，启动对应节点文件夹内的 rpcclient 即可运行控制台。控制台会自动连接处于同一个文件夹内的 kadfiletransporter。
当看到连接成功的提示后，您可以键入“SendFile”并按下回车来启动该示例程序的文件传输功能，接下来的操作跟随提示即可，控制台要求输入的节点 id，即是上文介绍过的节点 id。填写哪个节点，文件就会被发送到哪个节点。

使用完毕后，输入 exit 即可退出控制台

如果您在使用中遇到问题，欢迎在 issue 中提出来，我们会尽快回答并解决
