#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MSS 60000

int main(int argc, char *argv[])
{
	char *SERVERPORT = NULL;
	char *HOST = NULL;
	
	int pChecker = 0;
	int hChecker = 0;
	int opt;

	int video = 0;

	int status;

	printf("SEND_UDP\n");
	
	while ((opt = getopt(argc, argv, "a:p:")) != -1) {
		switch(opt) {
			case 'p':
			SERVERPORT = optarg;
			pChecker = 1;
			break;
			case 'a':
			HOST = optarg;
			hChecker = 1;
			break;
			case '?':
			fprintf(stderr, "Invalid argument\n");
			exit(1);
		}
	}

	if (pChecker == 0) {
		fprintf(stderr, "Missing port number\n");
		exit(1);
	}
	if (hChecker == 0) {
		fprintf(stderr, "Missing host address\n");
		exit(1);
	}

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	FILE *image;
	FILE *vid;
	
	int count = 0;
	long img_size;
	long vid_size;
	int i;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(HOST, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

    // loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("talker: socket");
		continue;
	}

	break;
}

if (p == NULL) {
	fprintf(stderr, "talker: failed to create socket\n");
	return 2;
}
while (1) {

	status = system("./Capture2_Bone -M -c 1 -o > image.jpg");

	if (status == -1) {
		fprintf(stderr, "Could not run Capture2 program\n");
	}


	image = fopen("image.jpg", "rb");
	if (image == NULL) {
		fprintf(stderr, "Error opening image\n");
	}

	fseek(image, 0, SEEK_END);
	img_size = ftell(image);
	fseek(image, 0, SEEK_SET);

	printf("sending picture size\n");
	if ((numbytes = sendto(sockfd, &img_size, sizeof(img_size), 0,
		p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
	exit(1);
}

printf("img_size: %ld\n", img_size);

char buffer[img_size];

if (img_size <= MSS) {
	while(!feof(image)) {
		fread (buffer, 1, sizeof(buffer), image);

		if ((numbytes = sendto(sockfd, buffer, sizeof(buffer), 0,
			p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
		exit(1);
	}
	bzero(buffer, sizeof(buffer));
}
} else {

	fread (buffer, 1, img_size, image);
	if ((numbytes = sendto(sockfd, buffer, MSS, 0,
		p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
	exit(1);
}

if ((numbytes = sendto(sockfd, buffer+MSS, sizeof(buffer)-MSS, 0,
	p->ai_addr, p->ai_addrlen)) == -1) {
	perror("talker: sendto");
exit(1);
}

usleep(1000000);
}
}
freeaddrinfo(servinfo);
close(sockfd);

return 0;
}

