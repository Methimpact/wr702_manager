#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "b64.h"

#define S(x) (strlen(x))


extern char 	 gV, *gPass, *gUser;
char 		*gAddr, *gAuth;

struct in_addr *makeAddr(char *ip) {
	static struct in_addr x;
	inet_aton(ip, &x);
	gAddr = ip;
	return &x;
}

char *encodeAuthString() {
	char *p, *q;
	size_t l;
	if (!(p = malloc(S(gUser)+S(gPass)+2))) {
		puts("Couldn't malloc auth string"); exit(1);
	}
	strcpy(p, gUser);
	p[S(gUser)] = ':';
	strcpy(p+S(gUser)+1, gPass);
	if (gV) printf("Encoding auth: %s\n", p);
	q = base64_encode(p, S(p), &l);
	free(p);
	return (q);
};

int contact(struct in_addr *x) {
	static struct sockaddr_in ad;

	ad.sin_family = AF_INET;
        ad.sin_port = htons(80);
	memcpy(&(ad.sin_addr.s_addr), &x, sizeof(struct in_addr));
	return socket(AF_INET, SOCK_STREAM, 0);
}

char *makeReq(const char *page, const char *req, size_t *sz) {
	const static char bp0[] = "GET http://", bp1[]="/userRpm/", bpst[] = "Rpm.htm?";
	const static char rp0[] = "\r\nReferer: https://", rp1[] = "/";
	const static char ap0[] ="\r\nAuthentification: Basic ", ap1[] = "\r\n\r\n\r\n";

	char *p, *beg;

	gAuth = encodeAuthString();
	if (gV) printf("Encoded to: %s\n", gAuth);
	*sz = S(gAddr) + S(bp0) + S(bp1) + S(bpst) + S(page) + S(req) + S(rp0)+ S(gAddr)+ S(rp1)+ S(ap0)+ S(gAuth)+ S(ap1) + 1;
	if (!(p = beg = malloc(*sz))) {
		puts("Couldn't malloc request"); exit(1);
	}
	if (gV) printf("Allocated %i@%x\n",*sz,p);
/* GET path */	strcpy(p, bp0); strcpy(p+=S(bp0), gAddr); strcpy(p+=S(gAddr),bp1);
/*  request */	strcpy(p+=S(bp1), page); strcpy(p+=S(page), bpst); strcpy(p+=S(bpst), req);
/*  Referer */	strcpy(p+=S(req), rp0); strcpy(p+=S(rp0), gAddr); strcpy(p+=S(gAddr), rp1);
/*   Auth   */	strcpy(p+=S(rp1), ap0); strcpy(p+=S(ap0), gAuth); strcpy(p+=S(gAuth), ap1);
	if (gV) printf("%s\n-END-\n", beg);
	return beg;
}
/*
TODO:
		Ecouter la reponse
		Returns the HTTP Status code
*/

unsigned short sendReq(short socket, const char *buf, size_t s) {
		static short err = 200;
		if ((err = send(socket, buf, s, MSG_NOSIGNAL)) == -1) {
			perror("sendreq");
		}
		return err;
}

void cleanup(int s) {
  base64_cleanup();
  close(s);
}
