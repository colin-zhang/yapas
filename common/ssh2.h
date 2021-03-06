#ifndef _SSH2_H_
#define _SSH2_H_
#include <arpa/inet.h>
#include <string>
#include "libssh2.h"

namespace yapas {

class SshChannel
{
public:
    enum Type {
        kChannelUnknow,
        kChannelExeCommand,
        kChannelShell,
        kChannelScpSend,
        kChannelScpReceive,
    };
public:
    SshChannel(LIBSSH2_CHANNEL* channel);
    SshChannel(LIBSSH2_SESSION* session);
    SshChannel(LIBSSH2_SESSION* session, SshChannel::Type type);
    ~SshChannel();
    int read(char* buffer, int buffer_len);
    int readStderr(char* buffer, int buffer_len);
    int write(const char* buffer, int buffer_len);
    int poll(int ms);
    void setEnv(const char* name, const char* value) {
        libssh2_channel_setenv(m_channel, name, value);
    }
    void setBlocking(int blocking) {
        libssh2_channel_set_blocking(m_channel, blocking);
    }
    void setType(Type type) {
        m_type = type;
    }
    Type getType() {
        return m_type;
    }
    int finish();
    void free();
    int exe(const char* command);
private:
    LIBSSH2_CHANNEL* m_channel;
    Type m_type;
};

class SshChannelScp : public SshChannel
{
public:
    static const int SEND = 0;
    static const int RECEIVE = 1;
public:
    SshChannelScp(LIBSSH2_CHANNEL* channel, std::string& scppath, std::string& local_path, int sr)
        : SshChannel(channel) {
        if (sr) {
            setType(SshChannel::kChannelScpReceive);
        } else {
            setType(SshChannel::kChannelScpSend);
        }
        m_rpath = scppath;
        m_lpath = local_path;
    }
    int transfer();
public:
    struct stat m_fileinfo;
private:
    std::string m_rpath;
    std::string m_lpath;
};

class SshSession
{
public:
    SshSession(const char* host, uint16_t port);
    ~SshSession();
    int connect();
    int auth(const char* user, const char* passwd);
    int waitSocket(int ms);
    SshChannel* startChannelCmd(const char* command);
    SshChannel* startChannelShell();
    SshChannelScp* startChannelScpGet(std::string& scppath, std::string& local_path);
    SshChannelScp* startChannelScpPost(std::string& local_path, std::string& scppath, int mode = 0777);
private:
    LIBSSH2_SESSION* m_session;
    std::string m_host;
    uint16_t m_port;
    int m_sock;
};

class LibSsh2 {
public:
    static void init();
    static void exit();
    static const char* version() {
        return libssh2_version(0);
    }
};

}

#endif
