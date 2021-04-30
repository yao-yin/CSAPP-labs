#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

void proxyDoIt(int fd);
void clientError(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void parse_uri(char *uri, char *target, int *port, char *filePath);
int build_requestHdrs(rio_t *rp, char *newreq, char *hostname);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    // portNumber, clientfd;
    //TODO: set portNumber from command line
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Check command line args */
    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(1);
    }
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        //connfd = Malloc(sizeof(int));
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
            port, MAXLINE, 0);
        printf("Accepted connection from(%s, %s)\n", hostname, port);
        proxyDoIt(connfd);
        Close(connfd);
    }
    Close(listenfd);
    return 0;
}

void proxyDoIt(int clientfd) 
{
    char method[MAXLINE], hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
    char uri[MAXLINE], version[MAXLINE];
    char asClientBuf[MAXLINE];
    rio_t rioAsServer, rioAsClient;
    rio_readinitb(&rioAsServer, clientfd);
    int asClientfd;
    if (!Rio_readlineb(&rioAsServer, asClientBuf, MAXLINE)) {
        return;
    }
    printf("%s", asClientBuf);
    sscanf(asClientBuf, "%s %s %s", method, uri, version);
    if (strcmp(method, "GET")) {
        clientError(clientfd, method, "502", "Not implemented", "We only support GET method.");
        return;
    }
    int portNumber = 0;
    parse_uri(uri, hostname, &portNumber, path);
    sprintf(port, "%d", portNumber);
    asClientfd = Open_clientfd(hostname, port);
    if (asClientfd < 0) {
        printf("Connected failed.\n");
        return;
    }
    Rio_readinitb(&rioAsClient, asClientfd);
    char newRequest[MAXLINE];
    sprintf(newRequest, "GET %s HTTP/1.0\n", path);
    build_requestHdrs(&rioAsServer, newRequest, hostname);
    Rio_writen(asClientfd, newRequest, strlen(newRequest));
    int n = 0;
    while ((n = Rio_readlineb(&rioAsClient, asClientBuf, MAXLINE)) > 0) {
        Rio_writen(clientfd, asClientBuf, n);
    }
    Close(asClientfd);
}

void parse_uri(char *uri, char* target, int* port, char * filePath) {
    *port = 80;
    //uri http://www.cmu.edu/hub/index.html
    char* pos1 = strstr(uri,"//");
    if (pos1 == NULL) {
        pos1 = uri;
    } else pos1 += 2;
    //printf("parse uri pos1 %s\n",pos1);//pos1 www.cmu.edu/hub/index.html

    char* pos2 = strstr(pos1,":");
    /*pos1 www.cmu.edu:8080/hub/index.html, pos2 :8080/hub/index.html */
    if (pos2 != NULL) {
        *pos2 = '\0'; //pos1 www.cmu.edu/08080/hub/index.html
        strncpy(target, pos1, MAXLINE);
        sscanf(pos2+1, "%d%s", port, filePath); //pos2+1 8080/hub/index.html
        *pos2 = ':';
    } else {
        pos2 = strstr(pos1, "/");//pos2 /hub/index.html
        if (pos2 == NULL) {/*pos1 www.cmu.edu*/
            strncpy(target, pos1, MAXLINE);
            strcpy(filePath, "");
            return;
        }
        *pos2 = '\0';
        strncpy(target, pos1, MAXLINE);
        *pos2 = '/';
        strncpy(filePath, pos2, MAXLINE);
    }
}

int build_requestHdrs(rio_t *rp, char *newreq, char *hostname) {
    //already have sprintf(newreq, "GET %s HTTP/1.0\r\n", path);
    char buf[MAXLINE];
    int content_length = 0;
    while(Rio_readlineb(rp, buf, MAXLINE) > 0) {        
	if (!strcmp(buf, "\r\n")) break;
        if (strstr(buf,"Host:") != NULL) continue;
        if (strstr(buf,"User-Agent:") != NULL) continue;
        if (strstr(buf,"Connection:") != NULL) continue;
        if (strstr(buf,"Proxy-Connection:") != NULL) continue;
        if (!strncasecmp(buf, "Content-length:", 15)) {
            sscanf(buf + 15, "%u", &content_length);
        }
	    sprintf(newreq,"%s%s", newreq, buf);
    }
    sprintf(newreq, "%sHost: %s\r\n", newreq, hostname);
    sprintf(newreq, "%s%s%s%s", newreq, user_agent_hdr,conn_hdr,prox_hdr);
    sprintf(newreq, "%s\r\n", newreq);
    return content_length;
}

void clientError(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}