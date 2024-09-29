/*
 vim: ai:et:sw=2:nows
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_EVENTS 100000

int get_remote_port(int sockfd) {
  struct sockaddr_in sa_in;
  socklen_t sa_in_len = sizeof(sa_in);

  int sn = getpeername(sockfd, (struct sockaddr *)&sa_in, &sa_in_len);
  if (sn != 0) {
    fprintf(stderr, "getpeername error: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return ntohs(sa_in.sin_port);
}

void get_clock(struct timespec *ts) {
  int res = clock_gettime(CLOCK_REALTIME, ts);
  if (res != 0) {
    fprintf(stderr, "clock_gettime error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
//  printf("%ld.%09ld\n", ts->tv_sec, ts->tv_nsec);
}

uint64_t clock_diff(struct timespec *begin, struct timespec *end) {
  return (end->tv_sec - begin->tv_sec) * 1000000000 + end->tv_nsec - begin->tv_nsec;
}

void setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD)|O_NONBLOCK) == -1) {
    fprintf(stderr, "fcntl error: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void read_fd(int fd) {
  char buf[1000];
  while (1) {
    int len = recv(fd, buf, sizeof(buf), 0);
    if (len == -1) {
      if (errno == EAGAIN) {
        break;
      } else {
        fprintf(stderr, "recv error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
    } else if (len == 0) {
      // fprintf(stderr, "recv is zero, the socket is closed\n");
      close(fd);
      break;
    } else {
      // normal read
      struct timespec received_ts, current_ts;
      get_clock(&current_ts);
      memcpy((void *)&received_ts, buf, sizeof(struct timespec));
//      printf("%ld.%09ld\n", received_ts.tv_sec, received_ts.tv_nsec);
      uint64_t t_diff = clock_diff(&received_ts, &current_ts)/1000;
      if (t_diff >= 1000) {
        printf("%lu remote port: %d\n", t_diff, get_remote_port(fd));
      }
    }
  }
}

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(-1);
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  struct addrinfo *ai;
  int s = getaddrinfo(NULL, argv[1], &hints, &ai);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (struct addrinfo *rp = ai; rp != NULL; rp = rp->ai_next) {
    struct sockaddr_in *sa = (struct sockaddr_in *)(rp->ai_addr);

    int listen_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listen_sock == -1) {
      fprintf(stderr, "socker error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    setnonblocking(listen_sock);

    if (bind(listen_sock, rp->ai_addr, rp->ai_addrlen) == -1) {
      fprintf(stderr, "bind error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, 65535) == -1) {
      fprintf(stderr, "listen error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Listening on %s:%d\n", inet_ntoa(sa->sin_addr), ntohs(sa->sin_port));

    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
      fprintf(stderr, "epoll_create1 error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_sock;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
      fprintf(stderr, "epoll_ctl error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    while (1) {
      int epoll_events_count = epoll_wait(epollfd, events, sizeof(events)/sizeof(struct epoll_event), -1);
      if (epoll_events_count == -1) {
        fprintf(stderr, "epoll_wait error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      for (int i = 0; i < epoll_events_count; i++) {
        if (events[i].data.fd == listen_sock) {
          struct sockaddr_in remote_addr;
          socklen_t addrlen = sizeof(struct sockaddr_in);

          int conn_sock = accept(listen_sock, (struct sockaddr *)&remote_addr, &addrlen);
          if (conn_sock == -1) {
            fprintf(stderr, "accept error: %d %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
          }
//          fprintf(stderr, "Connection from %s:%d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
          setnonblocking(conn_sock);
          ev.events = EPOLLIN | EPOLLET;
          ev.data.fd = conn_sock;

          if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
            fprintf(stderr, "epoll_ctl error: %d %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
          }

        } else {
          read_fd(events[i].data.fd);
        }
      }
    }

    close(listen_sock);
    close(epollfd);

    if (rp->ai_next) {
      fprintf(stderr, "There is another address, skipping\n");
    }
    break;
  }
  freeaddrinfo(ai);

  exit(EXIT_SUCCESS);
}
