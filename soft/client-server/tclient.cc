/*
 vim: ai:et:ic:sw=2:nows
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
#include <limits.h>
#include <netinet/tcp.h>

// Don't allow timestaps to be sent to these ports. So we don't disturb prod web servers.
#define WEB_PORTS { 80, 443 }

int get_local_port(int sockfd) {
  struct sockaddr_in sa_in;
  socklen_t sa_in_len = sizeof(sa_in);

  int sn = getsockname(sockfd, (struct sockaddr *)&sa_in, &sa_in_len);
  if (sn != 0) {
    fprintf(stderr, "Getsockname error: %d %s\n", errno, strerror(errno));
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

bool is_web_port(int port) {
  int ports[] = WEB_PORTS;
  for (int i=0; i<sizeof(ports)/sizeof(int); i++) {
    if (port == ports[i]) {
      return true;
    }
  }
  return false;
}

struct cmd_opts {
  char host[64]        = { 0 }; // server to connect to
  char port[8]         = { 0 }; // server port
  bool send_timestamps = false; // send timestamps to the server upon connection
  long repetitions     = 10;
  long min_delay       = 0;
  bool debug           = false;
};

void usage() {
  fprintf(stderr, "Usage: %s [-d] [-h] [-m min_delay] [-n repetitions] [-t] host port\n", program_invocation_short_name);
  fprintf(stderr, "  -d              Print some debug info\n");
  fprintf(stderr, "  -h              Print this help\n");
  fprintf(stderr, "  -m min_delay    Show connection times only when delay larger than or equal to min_delay, in microseconds. Default is 0\n");
  fprintf(stderr, "  -n repetitions  Repeat connections. Default is 10\n");
  fprintf(stderr, "  -t              Send timestamps\n");
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

  while ((opt = getopt(argc, argv, "dhm:n:t")) != -1) {
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

    case 'n':
      opts->repetitions = parse_number(optarg);
      break;

    case 't':
      opts->send_timestamps = true;
      break;

    default:
      usage();
    }
  }

  int nonopt_args = 0;
  long parsed_port;
  while (optind < argc) {
    switch (nonopt_args) {
    case 0: // host
      strncpy(opts->host, argv[optind], sizeof(opts->host)-1);
      break;

    case 1: // port
      parsed_port = parse_number(argv[optind]); // just to check the value
      if (parsed_port > USHRT_MAX) {
        fprintf(stderr, "Port must be less than %d, %ld given\n", USHRT_MAX, parsed_port);
        usage();
      }
      // sanity check
      if (opts->send_timestamps and is_web_port(parsed_port)) {
        fprintf(stderr, "Won't send timestamps on web port %ld\n", parsed_port);
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
  if (nonopt_args != 2) {
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

  struct addrinfo *result;
  int s = getaddrinfo(cmd_opts.host, cmd_opts.port, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "Getaddrinfo error: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
    struct timespec begin_ts, end_ts;
    int64_t t_diff;

    struct sockaddr_in *sa = (struct sockaddr_in *)(rp->ai_addr);
    fprintf(stderr, "Connecting to %s:%d, %ld repetitions, %sshow only connections slower than %ldus\n", inet_ntoa(sa->sin_addr),
        ntohs(sa->sin_port), cmd_opts.repetitions, cmd_opts.send_timestamps ? "sending timestamps, " : "", cmd_opts.min_delay);

    int i=0;
    while (i++ < cmd_opts.repetitions) {
      int error = 0;
      std::string error_descr;

      int sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sd == -1) {
        fprintf(stderr, "Socker error: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      get_clock(&begin_ts);
      int c = connect(sd, rp->ai_addr, rp->ai_addrlen);
      if (c == -1) {
        if (errno == ECONNREFUSED) {
          error = 1;
          error_descr.assign(strerror(errno));
        } else {
          fprintf(stderr, "Connect error: %d %s\n", errno, strerror(errno));
          exit(EXIT_FAILURE);
        }
      }
      get_clock(&end_ts);

      bool ts_was_sent = false;
      if (cmd_opts.send_timestamps) {
        struct tcp_info tcp_info;
        int tcp_info_len = sizeof(tcp_info);

        if (getsockopt(sd, IPPROTO_TCP, TCP_INFO, &tcp_info, (socklen_t *)&tcp_info_len) == -1) {
          fprintf(stderr, "Getsockopt error: %d %s\n", errno, strerror(errno));
          exit(EXIT_FAILURE);
        }

        if (tcp_info.tcpi_state == TCP_ESTABLISHED) {
          int sent_bytes = send(sd, &begin_ts, sizeof(struct timespec), 0);
          if (sent_bytes == -1) {
            fprintf(stderr, "Send error: %d %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
          }
          ts_was_sent = true;
        }
      }

      t_diff = clock_diff(&begin_ts, &end_ts)/1000;
      if (t_diff >= cmd_opts.min_delay) {
        printf("%ld ", t_diff);
        if (ts_was_sent and cmd_opts.debug) {
          printf("sent_ts: %ld.%09ld ", begin_ts.tv_sec, begin_ts.tv_nsec);
        }
        printf("local_port: %d", get_local_port(sd));
        if (error) {
          printf(" (%s)", error_descr.c_str());
        }
        printf("\n");
//      } else {
//        printf(".");
//        fflush(stdout);
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
