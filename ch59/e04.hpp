#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <map>

#define MAX_TEXT 8192
#define SERVER 1
#define CLIENT 2
#define SERVICE "9901"
#define BACK_LOG 30

static std::map< std::string, std::pair<std::string, std::string> > kvstore;
static int ParseData(char *data, int size);
std::string GetMap(std::string key)
{
    auto it = kvstore.end();
    it = kvstore.find(key);
    if (it == kvstore.end())
        return "none";
    return (it->second).first;
}

int GetMap()
{
    std::cerr << kvstore.size() << std::endl;
    for (auto v : kvstore)
        std::cerr << (v.second).first << std::endl;
    return 0;
}

#pragma pack(1)
struct Data{
    int length; //实际传递的长度
    char msg[0];
};
#pragma unpack(1)

class Buffer{
public:
    Buffer() : data(nullptr)
    {}
    virtual ~Buffer()
    {
        delete data;
    }
    Data *data;
};


class Socket{
public:
    Socket(int type, int fd);
    virtual ~Socket();
private:
    int fd;
    int type;
    Buffer *buf;
public:
    int ReceiveMsg();
    int SendMsg(std::string operation, std::string key,
                std::string value, std::string client_id);
    int SetBuf(int size);
    int Connect();
    int Accept();
    int Listen();
};

Socket::~Socket()
{
    if (this->fd != -1)
        close(this->fd);
}

Socket::Socket(int type, int fd)
{
    this->fd = fd;
    this->type = type;
    buf = new Buffer;
}

int Socket::SetBuf(int size){
    int actualLen = ntohl(size);
    this->buf->data = reinterpret_cast<Data*>(new char[actualLen + sizeof(int)]);
}

int Socket::Listen()
{
    struct addrinfo hint;
    struct addrinfo *result;
    hint.ai_addr = NULL;
    hint.ai_addrlen = NULL;
    hint.ai_canonname = NULL;
    hint.ai_family = AF_INET;
    hint.ai_flags = 0;
    if (type == SERVER)
        hint.ai_flags = AI_PASSIVE;
    hint.ai_next = NULL;
    hint.ai_protocol = 0;
    hint.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(NULL, SERVICE,
                            &hint, &result);
    if (0 != ret){
        std::cerr << __LINE__ << "getaddrinfo failed" << std::endl;
        return -1;
    }
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next)
    {
        int sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        int optval = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        ret = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (-1 == ret) {
            close(sfd);
            std::cerr << __LINE__ << "bind failed" << std::endl;
        }
        //在sfd可被监听前进来的连接在listen执行成功后会被立即连接
        ret = listen(sfd, BACK_LOG);
        if (-1 == ret) {
            close(sfd);
            std::cerr << __LINE__ << "bind failed" << std::endl;
            return -1;
        }
        this->fd = sfd;
        return 0;
    }
    return -1;
}

int Socket::Accept()
{
    int cfd = accept(this->fd, NULL, NULL);
    //fcntl(cfd, F_SETFL, (fcntl(cfd, F_GETFL)) | O_NONBLOCK);
    if (-1 == cfd)
    {
        std::cerr << __LINE__ << "accept failed" << std::endl;
        return -1;
    }
    return cfd;
}

int Socket::Connect()
{
    struct addrinfo hint;
    struct addrinfo *result;
    hint.ai_addr = NULL;
    hint.ai_addrlen = NULL;
    hint.ai_canonname = NULL;
    hint.ai_family = AF_INET;
    hint.ai_flags = 0;
    hint.ai_next = NULL;
    hint.ai_protocol = 0;
    hint.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(NULL, SERVICE, &hint, &result);
    if (0 != ret){
        std::cerr << __LINE__ << "getaddrinfo failed" << std::endl;
        return -1;
    }
    for(struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next)
    {
        int cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (-1 == cfd){
            std::cerr << __LINE__ << "socket failed" << std::endl;
            return -1;
        }
        ret = connect(cfd, rp->ai_addr, rp->ai_addrlen);
        if (-1 == ret){
            std::cerr << __LINE__ << "connect failed" << std::endl;
            return -1;
        }
        fd = cfd;
        //return 0;
        return cfd;
    }
    return -1;
}

int Socket::ReceiveMsg()
{
    int ret;
    int totalData;  //需要读取的总数据
    ret = read(fd, &totalData, sizeof(int));
    totalData = ntohl(totalData);
    this->SetBuf(totalData);
    std::cerr << "line194" << totalData << std::endl;
    int index = 0;
    while (totalData)
    {
        ret = read(fd, this->buf->data->msg + index, totalData);
        if (ret == EAGAIN)
            continue;
        if (ret == -1)  //认为客户端断开连接
            {perror("line201"); return ret;}
        if (ret == 0)
            break;
        index += ret;
        totalData -= ret;
    }

    if (this->type == SERVER)
        ParseData(this->buf->data->msg, index);
    std::cout << "write success!" << std::endl;
    return 0;
}

int 
Socket::SendMsg(std::string operation, std::string key,
                std::string value, std::string client_id)
{
    std::string data;
    data.append(operation);
    data.append("|");
    data.append(key);
    data.append("|");
    data.append(value);
    data.append("|");
    data.append(client_id);
    //std::cout << "line226 " << data.c_str() << std::endl;
    //std::cout << "line226 " << data.size() << std::endl;

    this->SetBuf(data.size());
    this->buf->data->length = htonl(data.size());
    memcpy(this->buf->data->msg, data.c_str(), data.size());

    int ret;
    int index = 0;
    int len = data.size() + sizeof(int);
    while (1)
    {
        ret = write(fd, this->buf->data + index, len);
        if (ret == EAGAIN)
            continue;
        if (ret == -1)  //认为客户端断开连接
            {perror("line242"); return ret;}
        if (ret == 0)
            break;
        index += ret;
        if (index == len)   //缓冲区的地址空间会在下次读取时变得无效
            break;
    }
    std::cout << "send success!" << std::endl;
    return 0;
}

static int ParseData(char *data, int size){
    std::string operation, key, value, client_id;
    std::string str(data);
    std::string::size_type index = 0;
    std::string::size_type preindex = 0;
    {
        index = str.find('|', preindex);  //关键字标识
        operation = str.substr(preindex, index - preindex);

        preindex = index + 1;
        index = str.find('|', preindex);  //关键字标识
        key = str.substr(preindex, index - preindex);

        preindex = index + 1;
        index = str.find('|', preindex);  //关键字标识
        value = str.substr(preindex, index - preindex);

        preindex = index + 1;
        client_id = str.substr(preindex, str.size()- preindex);
    }
    std::cout << operation << key << value << client_id << std::endl;
    kvstore.insert({key, {value, client_id}});
    return 0;
}
