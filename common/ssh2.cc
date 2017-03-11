#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <string>
#include "ssh2.h"

namespace yapas {

void LibSsh2::init()
{
    libssh2_init(0);
}

void LibSsh2::exit() 
{
    libssh2_exit();
}

SshSession::SshSession(const char* host, uint16_t port)
{
    m_session = libssh2_session_init();
    assert(m_session != NULL);
    m_host = std::string(host);
    m_port = port;
}

SshSession::~SshSession()
{
    if (m_session) {
        libssh2_session_disconnect(m_session, "Bye");
        libssh2_session_free(m_session);
        ::close(m_sock);
    }
}

int SshSession::connect()
{
    int rc;
    struct addrinfo ai_hints, *result;
    m_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock < 0) {
        std::cerr << "socket error," << strerror(errno) << std::endl;
        return -1;
    }
    //nonblocking(m_fd);
    //keepalive(m_fd);
    memset(&ai_hints, 0, sizeof(struct addrinfo));
    ai_hints.ai_family   = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_protocol = IPPROTO_TCP;

    /*stringstream stream;  
    stream<<int_temp;  */
    char port_str[32];
    snprintf(port_str, sizeof port_str, "%d", m_port);

    rc = getaddrinfo(m_host.c_str(), port_str, &ai_hints, &result);
    if (rc || !result) {
        ::close(m_sock);
        return -1;
    }

    rc = ::connect(m_sock, result->ai_addr, result->ai_addrlen);
    if (rc < 0) {
        ::close(m_sock);
        std::cerr << "connect error " << strerror(errno) << std::endl;
        return -1;
    }
    freeaddrinfo(result);

    if (rc < 0) {
        ::close(m_sock);
        return -1;
    }
    rc = libssh2_session_handshake(m_session, m_sock);
    if (0 != rc) {
        std::cerr << "Failure establishing SSH session: " << rc << std::endl;
    }
    return 0;
}

int SshSession::auth(const char* user, const char* passwd)
{
    if (libssh2_userauth_password(m_session, user, passwd) != 0) {
        std::cerr << "Authentication by password failed." << std::endl;
        return -1;
    }
}

int SshSession::waitSocket(int ms)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;

    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;

    FD_ZERO(&fd);

    FD_SET(m_sock, &fd);

    /* now make sure we wait in the correct direction */
    dir = libssh2_session_block_directions(m_session);

    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = ::select(m_sock + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

SshChannel* SshSession::startChannel(const char* command)
{
    int rc;
    LIBSSH2_CHANNEL* channel;
    SshChannel* ssh_channel = NULL;

    ssh_channel = new SshChannel(m_session);
    ssh_channel->setType(SshChannel::kChannelExeCommand);
    ssh_channel->setBlocking(1);

    while ((rc = ssh_channel->exe(command)) == LIBSSH2_ERROR_EAGAIN) {
        //waitsocket(m_sock, m_session);
    }
    if (rc != 0) {
        ssh_channel->free();
        std::cerr << "ssh channel error " << std::endl;
        return NULL;
    }
    return ssh_channel;
}

SshChannel* SshSession::startChannelScpGet(std::string& scppath, struct stat* fileinfo)
{
    LIBSSH2_CHANNEL* channel;
    SshChannel* ssh_channel = NULL;

    channel = libssh2_scp_recv(m_session, scppath.c_str(), fileinfo);
    if (!channel) {
        std::cerr << "Unable to open a session: " << libssh2_session_last_errno(m_session);
        return NULL;
    }
    ssh_channel = new SshChannel(channel);
    ssh_channel->setType(SshChannel::kChannelScpReceive);
    ssh_channel->setBlocking(1);
    return ssh_channel;
}

SshChannel* SshSession::startChannelScpPost(std::string& scppath, int file_size, int mode)
{
    LIBSSH2_CHANNEL* channel;
    SshChannel* ssh_channel = NULL;
    channel = libssh2_scp_send(m_session, scppath.c_str(), mode, (unsigned long)file_size);
    if (!channel) {
        char *errmsg;
        int errlen;
        int err = libssh2_session_last_error(m_session, &errmsg, &errlen, 0);
        std::cerr << "Unable to open a session: " << err << " " << errmsg << std::endl;
        return NULL;
    }
    ssh_channel = new SshChannel(channel);
    ssh_channel->setType(SshChannel::kChannelScpSend);
    ssh_channel->setBlocking(1);
    return ssh_channel;
}

SshChannel::SshChannel(LIBSSH2_CHANNEL* channel)
{
    m_channel = channel;
}

SshChannel::SshChannel(LIBSSH2_SESSION *session)
{
    LIBSSH2_CHANNEL* channel;
    while ((channel = libssh2_channel_open_session(session)) == NULL &&
                libssh2_session_last_error(session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN) {
        //waitsocket(m_sock, m_session);
    }
    m_channel = channel;
}

SshChannel::~SshChannel() 
{
    if (m_channel) {
        libssh2_channel_free(m_channel);
    }
}

int SshChannel::exe(const char* command)
{
    return libssh2_channel_exec(m_channel, command);
}

void SshChannel::free()
{
    if (m_channel != NULL) {
        libssh2_channel_free(m_channel);
    }
}

int SshChannel::finish()
{
    int  exitcode = 127;
    char *exitsignal=(char *)"OK";

    if (m_type == kChannelScpSend) {
        libssh2_channel_send_eof(m_channel);
        libssh2_channel_wait_eof(m_channel);
    }

    while((exitcode = libssh2_channel_close(m_channel)) == LIBSSH2_ERROR_EAGAIN) {
        libssh2_channel_wait_closed(m_channel);
        //waitsocket(sock, session);
    }
    if (exitcode == 0) {
        exitcode = libssh2_channel_get_exit_status(m_channel);
        libssh2_channel_get_exit_signal(m_channel, &exitsignal, NULL, NULL, NULL, NULL, NULL);
    } else {
        std::cerr << "close error:" << exitcode << std::endl;
        return -1;
    }

    if (exitcode != 0) {
        std::cerr << "exitcode:" << exitcode << std::endl;
        char buf[512] =  {0,};
        readStderr(buf, sizeof(buf));
        std::cerr << buf << std::endl;
    }
    return exitcode;
}

/*int SshSession::SshChannel::scp(const char* command)
{
    //libssh2_scp_send(session, scppath, fileinfo.st_mode & 0777, (unsigned long)fileinfo.st_size);
}*/

int SshChannel::read(char* buffer, int buffer_len)
{
    int rc;
    rc = libssh2_channel_read(m_channel, buffer, buffer_len);
    return rc;
}

int SshChannel::write(char* buffer, int buffer_len)
{
    int rc;
    rc = libssh2_channel_write(m_channel, buffer, buffer_len);
    return rc;
}

int SshChannel::readStderr(char* buffer, int buffer_len)
{
    int rc;
    rc = libssh2_channel_read_stderr(m_channel, buffer, buffer_len);
    return rc;
}

int SshChannel::poll(int ms)
{
    LIBSSH2_POLLFD *fds = new LIBSSH2_POLLFD;
    fds->type = LIBSSH2_POLLFD_CHANNEL;
    fds->fd.channel = m_channel;
    fds->events = LIBSSH2_POLLFD_POLLIN;
    int rc = (libssh2_poll(fds, 1, ms));
    if (rc < 0)
        return -1;
    if (rc)
        return 1;
    // time out 
    return 0;
}

} //end namespace yapas

