/*
 vim: ai:et:sw=2:nows
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cerrno>
#include <string>
#include <iostream>
#include <iterator>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int get_local_port(int sockfd) {
  struct sockaddr_in sa_in;
  socklen_t sa_in_len = sizeof(sa_in);

  int sn = getsockname(sockfd, (struct sockaddr *)&sa_in, &sa_in_len);
  if (sn != 0) {
    fprintf(stderr, "getsockname error: %d %s\n", errno, strerror(errno));
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

int main(int argc, char **argv) {

  if (argc < 3) {
    fprintf(stderr, "Usage: %s host port\n", argv[0]);
    exit(-1);
  }
  int repeat = 10;
  if (argc > 3) {
    repeat = atoi(argv[3]);
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *result;
  int s = getaddrinfo(argv[1], argv[2], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
    struct timespec begin_ts, end_ts;
    uint64_t t_diff;

    struct sockaddr_in *sa = (struct sockaddr_in *)(rp->ai_addr);
//    fprintf(stderr, "Trying first addr %s:%d\n", inet_ntoa(sa->sin_addr), ntohs(sa->sin_port));

    int i=0;
    while (i++ < repeat) {
      int error = 0;
      std::string error_descr;

      int sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1) {
        fprintf(stderr, "socker error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      get_clock(&begin_ts);
      int c = connect(sd, rp->ai_addr, rp->ai_addrlen);
      if (c == -1) {
        if (errno == ECONNREFUSED) {
          error = 1;
          error_descr.assign(strerror(errno));
        } else {
          fprintf(stderr, "connect error: %d %s\n", errno, strerror(errno));
          exit(EXIT_FAILURE);
        }
      }
      get_clock(&end_ts);

      int sent = send(sd, &begin_ts, sizeof(struct timespec), 0);
      if (sent == -1) {
        fprintf(stderr, "send error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      t_diff = clock_diff(&begin_ts, &end_ts)/1000;
//      if (t_diff >= 1000) {
      if (t_diff >= 0) {
        printf("%lu local port: %d%s\n", t_diff, get_local_port(sd), error ? (" (" + error_descr + ")").c_str() : "");
      } else {
        printf(".");
        fflush(stdout);
      }
      close(sd);

      usleep(10000);
    }
    if (rp->ai_next) {
      fprintf(stderr, "There is another address, skipping\n");
    }
    break;
  }
  freeaddrinfo(result);

  exit(EXIT_SUCCESS);
}
