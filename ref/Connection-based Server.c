/*
http://retrogeeks.org/sgi_bookshelves/SGI_Developer/books/IRIX_NetPG/sgi_html/ch03.html
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define MAX_CONN        5

main(argc, argv)
    int argc;
    char *argv[];
    {
    struct addrinfo hints, *res, *ressave;
    struct sockaddr_storage from;
    int fromlen;
    int err_num;
    int oldsock, newsock;
    int readval;
    char buf[1024];
    int conn_num = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;

    err_num = getaddrinfo(NULL, argv[1], &hints, &res);
    if(err_num) {
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(err_num));
        exit(1);
    }
    ressave = res;
    do {
       oldsock = socket(res->ai_family, res->ai_socktype,
                  res->ai_protocol);
       if(oldsock < 0)
              continue;
       if(bind(oldsock, res->ai_addr, res->ai_addrlen) < 0) {
             perror("bind");
             close(oldsock);
             continue;
       }
       if(listen(oldsock, 5) < 0) {
             perror("listen");
             close(oldsock);
             continue;
       }
       do    {
             if((newsock = accept(oldsock, (struct sockaddr *)&from,
                              &fromlen)) < 0) {
                   perror("accept");
                   close(oldsock);
             }
             else {
               do {
                  memset(&buf, 0, sizeof(buf));
                  if((readval = read(newsock, buf, 1024)) == -1)
                      perror("read");
                  if(readval == 0)
                     printf("Ending connection\n");
                  else
                     printf("---> %s\n", buf);
               } while (readval > 0);
               conn_num++;
             }
             close(newsock);
      } while (conn_num < MAX_CONN);
      /* break after establishing the required no. of connections */
      close(oldsock);
      break;
   } while ((res = res->ai_next) != NULL);
   if(!res) {
      fprintf(stderr, "bind/listen/accept failed for all addresses\n");
      freeaddrinfo(ressave);
      exit(1);
   }
   freeaddrinfo(ressave);
} /* end of main */