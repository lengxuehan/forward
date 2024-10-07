#include <iostream>
#include <netdb.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/socket.h>
#include <malloc.h>

#include "common/file_utility.h"
#include "xudp.h"
#include "tools/json_unity.h"
#include "data_storager.h"
#include "common/time_sync.h"

struct connect{
    xudp *x;
    xudp_channel *ch;
    void (*handler)(struct connect *);
};

static bool g_loop = true;

StructAStorager structA_storager(forward::common::FileUtility::get_process_path(), "csv");
StructBStorager structB_storager(forward::common::FileUtility::get_process_path(), "csv");
// 初始化纳秒生成器
forward::common::TimeSync ts;

void handler(struct connect *c)
{
    xudp_msg *m;
    xudp_channel *ch = c->ch;
    int n, i, ret;

    xudp_def_msg(hdr, 100);

    while (true) {
        hdr->used = 0;

        n = xudp_recv_channel(ch, hdr, 0);
        if (n < 0)
            break;

        for (i = 0; i < hdr->used; ++i) {
            m = hdr->msg + i;

            //printf("recv msg: %.*s", m->size, m->p);
            printf("recv msg: %d\n", m->size);
            if(m->size == 40) {
                StructA struct_a;
                struct_a.deserialize(m->p, m->size);
                //printf("total id : %lu\n", struct_a.ns);
                //printf("total id : %lu\n", struct_a.total_id);
                //printf("total id : %lu\n", struct_a.data_id);
                struct_a.recv_ns = ts.get_ns();
                structA_storager.asyncWrite(struct_a);
            } else {
                StructB struct_b;
                struct_b.deserialize(m->p, m->size);
                //printf("total id : %lu\n", struct_a.ns);
                //printf("total id : %lu\n", struct_a.total_id);
                //printf("total id : %lu\n", struct_a.data_id);
                struct_b.recv_ns = ts.get_ns();
                structB_storager.asyncWrite(struct_b);
            }
        }

        xudp_recycle(hdr);
        xudp_commit_channel(ch);
    }
}

static int epoll_add(xudp *x, int efd)
{
    struct epoll_event e;
    struct connect *c;
    xudp_channel *ch;
    xudp_group *g;
    int fd;

    e.events = EPOLLIN;

    g = xudp_group_get(x, 0);

    xudp_group_channel_foreach(ch, g) {

        fd = xudp_channel_get_fd(ch);

        c = (struct connect*)(malloc(sizeof(*c)));
        c->ch = ch;
        c->x = x;
        c->handler = handler;

        e.data.ptr = c;

        epoll_ctl(efd, EPOLL_CTL_ADD, fd, &e);
    }

    return 0;
}

static int loop(int efd)
{
    struct connect *c;
    struct epoll_event e[1024];
    int n, i;

    while (g_loop) {
        n = epoll_wait(efd, e, sizeof(e)/sizeof(e[0]), -1);

        if (n == 0)
            continue;

        if (n < 0) {
            //printf("epoll wait error: %s\n", strerror(errno));
            continue;
        }

        for (i = 0; i < n; ++i) {
            c = (struct connect*)(e[i].data.ptr);
            c->handler(c);
        }
    }
    return 0;
}

static void int_exit(int sig)
{
    (void)sig;
    g_loop = 0;
}

int main() {
    using namespace forward::tool;
    std::cout << "RuntimeForward starting.\n";
    // 1.解析配置文件到结构体中
    std::string str_current_dir = forward::common::FileUtility::get_process_path();
    std::string path_to_config_file{str_current_dir + "/receiver_config.json"};
    std::ifstream ifs(path_to_config_file, std::ios::binary);
    if (!ifs.is_open()){
        std::cout << "failed to open " << path_to_config_file << std::endl;
        return 0;
    }
    nlohmann::json config = nlohmann::json::parse(ifs, nullptr, false);
    if (config.is_discarded()){
        std::cout << "the input is invalid JSON." << std::endl;
        return 0;
    }
    xudp *x;
    int efd, ret;
    std::string addr, port;
    if(!JsonUnity::get(config, "local_ip" ,addr)) {
        std::cout << "cannot get local_ip in json config" << std::endl;
        return 0;
    }
    if(!JsonUnity::get(config, "local_port" ,port)) {
        std::cout << "cannot get local_port in json config" << std::endl;
        return 0;
    }
    std::cout << "ip:" << addr << " port:" << port << std::endl;

    // 初始化 TimeSync 类
    ts.init();
    // 启动校准线程
    ts.start_calibration_thread();

    int size;
    struct addrinfo *info;

    xudp_conf conf = {};

    ret = getaddrinfo(addr.c_str(), port.c_str(), NULL, &info);
    if (ret) {
        printf("addr format err\n");
        return -1;
    }

    conf.group_num     = 1;
    conf.log_with_time = true;
    conf.log_level = XUDP_LOG_WARN;

    x = xudp_init(&conf, sizeof(conf));
    if (!x) {
        printf("xudp init fail\n");
        return -1;
    }

    if (info->ai_family == AF_INET) {
        printf("AF_INET addr.\n");
        size = sizeof(struct sockaddr_in);
    } else {
        printf("AF_INET6 addr.\n");
        size = sizeof(struct sockaddr_in6);
    }

    ret = xudp_bind(x, (struct sockaddr *)info->ai_addr, size, 1);
    if (ret) {
        xudp_free(x);
        printf("xudp bind fail %d\n", ret);
        return -1;
    }

    efd = epoll_create(1024);

    epoll_add(x, efd);

    signal(SIGINT, int_exit);
    signal(SIGTERM, int_exit);
    signal(SIGABRT, int_exit);

    loop(efd);
    xudp_free(x);
    return 0;
}
