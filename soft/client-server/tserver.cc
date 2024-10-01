/*
 vim: ai:et:ic:sw=2:nows
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
#include <limits.h>

#define MAX_EVENTS 100000
#define LISTEN_BACKLOG 65535

int get_remote_port(int sockfd) {
  struct sockaddr_in sa_in;
  socklen_t sa_in_len = sizeof(sa_in);

  int sn = getpeername(sockfd, (struct sockaddr *)&sa_in, &sa_in_len);
  if (sn != 0) {
    fprintf(stderr, "Getpeername error: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return ntohs(sa_in.sin_port);
}

void get_clock(struct timespec *ts) {
  int res = clock_gettime(CLOCK_REALTIME, ts);
  if (res != 0) {
    fprintf(stderr, "Clock_gettime error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
//  printf("%ld.%09ld\n", ts->tv_sec, ts->tv_nsec);
}

int64_t clock_diff(struct timespec *begin, struct timespec *end) {
  return (end->tv_sec - begin->tv_sec) * 1000000000 + end->tv_nsec - begin->tv_nsec;
}

void setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD)|O_NONBLOCK) == -1) {
    fprintf(stderr, "Fcntl error: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

struct cmd_opts {
  char port[8]   = { 0 }; // listen port
  long min_delay = 1000;
  bool debug     = false;
};

void read_fd(int fd, const struct cmd_opts *cmd_opts) {
  char buf[1000];
  while (1) {
    int len = recv(fd, buf, sizeof(buf), 0);
    if (len == -1) {
      if (errno == EAGAIN) {
        break;
      } else {
        fprintf(stderr, "Recv error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }
    } else if (len == 0) {
      // fprintf(stderr, "Recv is zero, the socket is closed\n");
      close(fd);
      break;
    } else {
      struct timespec current_ts, received_ts;
      get_clock(&current_ts);
      memcpy((void *)&received_ts, buf, sizeof(struct timespec));
      int64_t t_diff = clock_diff(&received_ts, &current_ts)/1000;
      if (t_diff >= cmd_opts->min_delay or t_diff < 0) {
        printf("%ld ", t_diff);
        if (cmd_opts->debug) {
          printf("recv_ts: %ld.%09ld current_ts: %ld.%09ld ", received_ts.tv_sec, received_ts.tv_nsec, current_ts.tv_sec, current_ts.tv_nsec);
        }
        printf("remote_port: %d\n", get_remote_port(fd));
      }
    }
  }
}

void usage() {
  fprintf(stderr, "Usage: %s [-d] [-h] [-m min_delay] port\n", program_invocation_short_name);
  fprintf(stderr, "  -d            Print some debug info\n");
  fprintf(stderr, "  -h            Print this help\n");
  fprintf(stderr, "  -m min_delay  Show connection times only when delay larger than or equal to min_delay, in microseconds. Default is 1000\n");
  fprintf(stderr, "                Requires the client to send timestamps. Default is 1000\n");
  exit(EXIT_FAILURE);
}

long parse_number(const char*str) {
  if (strspn(str, "0123456789") != strlen(str)) {
    fprintf(stderr, "Can't parse number %s\n", str);
    usage();
  }
  errno = 0;
  long result = strtol(str, NULL, 10);
  if (errno != 0) {
    fprintf(stderr, "Strtol error: %d %s\n", errno, strerror(errno));
    usage();
  }
  return result;
}

void parse_command_line(struct cmd_opts *opts, int argc, char **argv) {
  int opt;

  while ((opt = getopt(argc, argv, "dhm:")) != -1) {
    switch (opt) {
    case 'd':
      opts->debug = true;
      break;

    case 'h':
      usage();
      break;

    case 'm':
      opts->min_delay = parse_number(optarg);
      break;

    default:
      usage();
    }
  }

  int nonopt_args = 0;
  long parsed_port;
  while (optind < argc) {
    switch (nonopt_args) {
    case 0: // port
      parsed_port = parse_number(argv[optind]); // just to check the value
      if (parsed_port > USHRT_MAX) {
        fprintf(stderr, "Port must be less than %d, %ld given\n", USHRT_MAX, parsed_port);
        usage();
      }
      strncpy(opts->port, argv[optind], sizeof(opts->port)-1);
      break;

    default:
      fprintf(stderr, "Unexpected arg %s\n", argv[optind]);
      usage();
    }
    optind++;
    nonopt_args++;
  }
  if (nonopt_args != 1) {
    usage();
  }
}

int main(int argc, char **argv) {
  struct cmd_opts cmd_opts;
  parse_command_line(&cmd_opts, argc, argv);

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  struct addrinfo *ai;
  int s = getaddrinfo(NULL, cmd_opts.port, &hints, &ai);
  if (s != 0) {
    fprintf(stderr, "Getaddrinfo error: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (struct addrinfo *rp = ai; rp != NULL; rp = rp->ai_next) {
    struct sockaddr_in *sa = (struct sockaddr_in *)(rp->ai_addr);

    int listen_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listen_sock == -1) {
      fprintf(stderr, "Socker error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    setnonblocking(listen_sock);

    if (bind(listen_sock, rp->ai_addr, rp->ai_addrlen) == -1) {
      fprintf(stderr, "Bind error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, LISTEN_BACKLOG) == -1) {
      fprintf(stderr, "Listen error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Listening on %s:%d, show only connections slower than %ldus, if client sends timestamps\n", inet_ntoa(sa->sin_addr), ntohs(sa->sin_port), cmd_opts.min_delay);

    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
      fprintf(stderr, "Epoll_create1 error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_sock;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
      fprintf(stderr, "Epoll_ctl error: %d %s\n", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    while (1) {
      int epoll_events_count = epoll_wait(epollfd, events, sizeof(events)/sizeof(struct epoll_event), -1);
      if (epoll_events_count == -1) {
        fprintf(stderr, "Epoll_wait error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      for (int i = 0; i < epoll_events_count; i++) {
        if (events[i].data.fd == listen_sock) {
          struct sockaddr_in remote_addr;
          socklen_t addrlen = sizeof(struct sockaddr_in);

          int conn_sock = accept(listen_sock, (struct sockaddr *)&remote_addr, &addrlen);
          if (conn_sock == -1) {
            fprintf(stderr, "Accept error: %d %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
          }
//          fprintf(stderr, "Connection from %s:%d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
          setnonblocking(conn_sock);
          ev.events = EPOLLIN | EPOLLET;
          ev.data.fd = conn_sock;

          if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
            fprintf(stderr, "Epoll_ctl error: %d %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
          }

        } else {
          read_fd(events[i].data.fd, &cmd_opts);
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
