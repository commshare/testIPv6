/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#define LOG (printf("(%d) - <%s> ",/*__FILE__,*/__LINE__,__FUNCTION__),printf)
#define PORT "9034"   // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
        LOG("this sockaddr is V4 \n");
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
    LOG("this sockaddr is V6 \n");
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

#if 0
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}
        char * ver ="unkown";
        if(p->ai_family == AF_INET6){
            ver = "v6";
        }else{
            ver = "v4";
        }
        LOG("bind %d ver %s ok\n",listener,ver);
		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this
#else
/*
 *
 * struct sockaddr_in6 sin6;
...
sin6.sin6_addr = in6addr_loopback;
...
if(connect(s, (struct sockaddr *)&sin6, sizeof(sin6)) == -1)
...
 * */
    int sockfd ;
    struct sockaddr_in6 addr;
    bzero(&addr,sizeof(addr));
    addr.sin6_len = sizeof(struct sockaddr_in6); //TODO 这个对不对呢？
    addr.sin6_family = AF_INET6;
    char *port = "9024";
    addr.sin6_port = htons(9024);
    addr.sin6_addr = in6addr_loopback;
    sockfd = socket(PF_INET6,SOCK_STREAM,IPPROTO_TCP);
    if(sockfd < 0 ){
        LOG("SOCKET FAIL %d  %s \n",sockfd,strerror(errno));
        exit(2);
    }
    printf("address created %d \n",sockfd);


    // lose the pesky "address already in use" error message
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    /*注意，这个大小要填写sockaddr_in6的，如果填写sockaddr 会认为有参数错误*/
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) < 0) {
        LOG(" bind sockfd %d fail %s \n",sockfd,strerror(errno));
        close(sockfd);
        exit(2);
    }

    listener = sockfd;
    LOG(" bind sockfd %d OK  listener %d \n", sockfd,listener);

#endif

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);
                        char * msg =  "loopbacksocketsendMsgToClient";
                       int rs =  send(newfd,msg,strlen(msg),0);
                       LOG("SEND CLIENT  %d \n",rs);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}

