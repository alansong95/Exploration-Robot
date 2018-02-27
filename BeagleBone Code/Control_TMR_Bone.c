#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 150

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
	
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	
	return 0;
}

int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
	
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
	
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
	
	close(fd);
	return 0;
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
	
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
	
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
	
	close(fd);
	return 0;
}


void ind_run(int on, int PWM) {
	FILE *file;
	if (PWM == 1) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_31.10/run", "w");
	} else if (PWM == 2) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_14.11/run", "w");
	} else if (PWM == 3) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P8_19.12/run", "w");
	} else {
		fprintf(stderr, "Please enter a correct PWM\n");
		exit(1);
	}

	if(on) {
		fprintf(file, "%d", 1);
	} else {
		fprintf(file, "%d", 0);
	}
	fclose(file);
}

void period(int period, int PWM) {
	FILE *file;
	if (PWM == 1) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_31.10/period", "w");
	} else if (PWM == 2) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_14.11/period", "w");
	} else if (PWM == 3) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P8_19.12/period", "w");
	} else {
		fprintf(stderr, "Please enter a correct PWM\n");
		exit(1);
	}

	if (period > 0) {
		fprintf(file, "%d", period);
	} else {
		fprintf(stderr, "Please enter a valid period\n");
	}

	fclose(file);
}

void duty(unsigned int duty, int PWM) {
	FILE *file;
	if (PWM == 1) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_31.10/duty", "w");
	} else if (PWM == 2) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P9_14.11/duty", "w");
	} else if (PWM == 3) {
		file = fopen("/sys/devices/ocp.3/pwm_test_P8_19.12/duty", "w");
	} else {
		fprintf(stderr, "Please enter a correct PWM\n");
		exit(1);
	}

	if (duty > 0) {
		fprintf(file, "%d", duty);
		fclose(file);
	} 
}

void vvTOwv(double* vv, double* wv) {
	double r = 0.02;
	double L = 0.10; 

	double T[3][3] = {{0, 1/r, L/r}, {-1.1547/r, -0.5/r, L/r}, {1.1547/r, -0.5/r, L/r}};

	double Tv = 0;
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			Tv = Tv + vv[j]*T[i][j];
		}
		wv[i] = Tv;
		Tv = 0;
	}
}

void quit() {
	ind_run(0, 1);
	ind_run(0, 2);
	ind_run(0, 3);
}

void run(double* wv) {
	int p=3000000;
	unsigned int duty1;
	unsigned int duty2;
	unsigned int duty3;
	unsigned int stopDuty = 1500000;

	if (wv[0] > 0) {
		duty1 = 1533400+(2999939-1533400)*wv[0]*0.01;
	} else if (wv[0] < 0) {
		duty1 = 1400000+(1400000-660)*wv[0]*0.01;
	} else {
		duty1 = stopDuty;
	}

	if (wv[1] > 0) {
		duty2 = 1530440+(2999999-1530440)*wv[1]*0.01;
	} else if (wv[1] < 0) {
		duty2 = 1395000+(1395000-720)*wv[1]*0.01;
	} else {
		duty2 = stopDuty;
	}

	if (wv[2] > 0) {
		duty3 = 1538700+(2999939-1538700)*wv[2]*0.01;
	} else if (wv[2] < 0) {
		duty3 = 1410000+(1410000-720)*wv[2]*0.01;
	} else {
		duty3 = stopDuty;
	}

	ind_run(0, 1);
	ind_run(0, 2);
	ind_run(0, 3);

	period(p, 1);
	period(p, 2);
	period(p, 3);

	duty(duty1, 1);
	duty(duty2, 2);
	duty(duty3, 3);

	ind_run(1, 1);
	ind_run(1, 2);
	ind_run(1, 3);
}


int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	int id;
	float time_elapsed;
	float vx;
	float vy;
	float vw;
	int ll;
	int lr;
	double vv[3];
	double wv[3];


	gpio_export(20);
	gpio_set_dir(20, 1);

	gpio_export(7);
	gpio_set_dir(7, 1);

	
	while(1) {

		memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_flags = AI_PASSIVE; // use my IP

      if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
      	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      	return 1;
      }

      // loop through all the results and bind to the first we can
      for(p = servinfo; p != NULL; p = p->ai_next) {
      	if ((sockfd = socket(p->ai_family, p->ai_socktype,
      		p->ai_protocol)) == -1) {
      		perror("listener: socket");
      	continue;
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      	close(sockfd);
      	perror("listener: bind");
      	continue;
      }

      break;
  }

  if (p == NULL) {
  	fprintf(stderr, "listener: failed to bind socket\n");
  	return 2;
  }
  freeaddrinfo(servinfo);

  printf("listener: waiting to recvfrom...\n");

  addr_len = sizeof their_addr;
  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
  	(struct sockaddr *)&their_addr, &addr_len)) == -1) {
  	perror("recvfrom");
  exit(1);
}

buf[numbytes] = '\0';

printf("listener: packet is %d bytes long\n", numbytes);
printf("listener: packet contains \"%s\"\n", buf);

id = atoi(strtok(buf, " "));
time_elapsed = atof(strtok(NULL, " "));
vx = atof(strtok(NULL, " "));
vy = atof(strtok(NULL, " "));
vw = atof(strtok(NULL, " "));
ll = atoi(strtok(NULL, " "));
lr = atoi(strtok(NULL, " "));

vv[0] = vx;
vv[1] = vy;
vv[2] = vw;

if(vv[0] == 0 && vv[1] == 0 && vv[2] == 0) {
	quit();
	gpio_set_value(20, ll);
	gpio_set_value(7, lr);
	close(sockfd);
	continue;
}

vvTOwv(vv, wv);

run(wv);

gpio_set_value(20, ll);
gpio_set_value(7, lr);

close(sockfd);
}

unexport(20);
unexport(7);
return 0;
}