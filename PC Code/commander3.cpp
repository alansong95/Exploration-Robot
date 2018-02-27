// 1st argument: receiver's ip address

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <SDL2/SDL_thread.h>
#include <unistd.h>
#include <sys/time.h>

#define TMR_PORT "4950"
#define IMG_PORT "4960"    // the port users will be connecting to
#define SERV_PORT "33834"

#define MAXBUFLEN 10000
#define MSS 60000

enum control
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    STOP
};

int count;

FILE *file;
FILE *image;
FILE *vid;

SDL_Joystick* gGameController = NULL;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia(int opt);

//Frees media and shuts down SDL
void close();

//Loads individual image
SDL_Surface* loadSurface( std::string path );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
    
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Current displayed PNG image
SDL_Surface* gPNGSurface = NULL;

SDL_Surface* graySurface = NULL;




// **************** TMR CONTROL START ************************


double* stop(double* vv) {
  vv[0] = 0;
  vv[1] = 0;
  vv[2] = 0;
  return vv;
}

double* forwards(double* vv) {
  vv[0] = 1;
  vv[1] = 0;
  vv[2] = 0;
  return vv;
}

double* backwards(double* vv) {
  vv[0] = -1;
  vv[1] = 0;
  vv[2] = 0;
  return vv;
}

double* right(double* vv) {
  vv[0] = 0;
  vv[1] = 1;
  vv[2] = 0;
  return vv;
}

double* left(double* vv) {
  vv[0] = 0;
  vv[1] = -1;
  vv[2] = 0;
  return vv;
}

double* rotatecw(double* vv) {
  vv[0] = 0;
  vv[1] = 0;
  vv[2] = -1;
  return vv;
}

double* rotateccw(double* vv) {
  vv[0] = 0;
  vv[1] = 0;
  vv[2] = 1;
  return vv;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

// **************** TMR CONTROL END ************************










// pika
int threadFunction( void* data )
{
    int sockfd2, new_fd2;
    struct addrinfo hints2, *servinfo2, *p2;
    int rv2;
    int numbytes2;
    struct sockaddr_storage their_addr2;
    char buf2[MAXBUFLEN];
    socklen_t addr_len2;
    char s2[INET6_ADDRSTRLEN];

    int yes = 1;
   
    int size2;

    int is_vid = 0;

    int total_received = 0;
    while(1){
    memset(&hints2, 0, sizeof hints2);
    hints2.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints2.ai_socktype = SOCK_DGRAM;
    hints2.ai_flags = AI_PASSIVE; // use my IP

    if ((rv2 = getaddrinfo(NULL, SERV_PORT, &hints2, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
        if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
                p2->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }


        if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
            close(sockfd2);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p2 == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo2);

    printf("listener: waiting to recvfrom...\n");


    


        addr_len2 = sizeof their_addr2;
        if ((numbytes2 = recvfrom(sockfd2, &size2, sizeof(size2) , 0,
            (struct sockaddr *)&their_addr2, &addr_len2)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("size: %d\n", size2);





            char image_array[size2];

            if (size2 <= MSS) {
                printf("yolo\n");
                if ((numbytes2 = recvfrom(sockfd2, image_array, size2, 0,
                    (struct sockaddr *)&their_addr2, &addr_len2)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                
            } else {
                if ((numbytes2 = recvfrom(sockfd2, image_array, size2, 0,
                    (struct sockaddr *)&their_addr2, &addr_len2)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }

                if ((numbytes2 = recvfrom(sockfd2, image_array+MSS, size2-MSS, 0,
                    (struct sockaddr *)&their_addr2, &addr_len2)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
            }

            image = fopen("rcv.jpg", "wb");

            // added
            if (image == NULL) {
                fprintf(stderr, "Could not open image\n");
            }
            fwrite(image_array, 1, sizeof(image_array), image);

            memset(&image_array, 0, sizeof image_array);

            
            close(sockfd2);
            fclose(image);      

            usleep(100000);
        }
}

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        if( SDL_NumJoysticks() < 1 )
        {
            printf( "Warning: No joysticks connected!\n" );
        }
        else
        {
            //Load joystick
            gGameController = SDL_JoystickOpen( 0 );
            if( gGameController == NULL )
            {
                printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
            }
        }


        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }  
        else
        {
            //Initialize PNG loading
            int imgFlags = IMG_INIT_JPG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                success = false;
            }
            else
            {
                //Get window surface
                gScreenSurface = SDL_GetWindowSurface( gWindow );
            }
        }
    }

    return success;
}

bool loadMedia(int opt)
{
    //Loading success flag
    bool success = true;

    /*
    if (count % 3 == 0) {
        gPNGSurface = loadSurface( "image1.jpg" );
    } else if (count % 3 == 1) {
        gPNGSurface = loadSurface( "image2.jpg" );
    } else if (count % 3 == 2) {
        gPNGSurface = loadSurface( "image3.jpg" );
    }
    //Load PNG surface
    */

    if (opt == 0) {
        gPNGSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(gPNGSurface, NULL, SDL_MapRGB(gPNGSurface->format, 128, 128, 128));
    } else {
        gPNGSurface = loadSurface("rcv.jpg");
    }

    
    
    if( gPNGSurface == NULL )
    {
        printf( "Failed to load PNG image!\n" );
        success = false;
    }

    SDL_BlitSurface( gPNGSurface, NULL, gScreenSurface, NULL );
            
    SDL_UpdateWindowSurface( gWindow );

    return success;
}

void close()
{
    //Free loaded image
    SDL_FreeSurface( gPNGSurface );
    gPNGSurface = NULL;

    //Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

SDL_Surface* loadSurface( std::string path )
{
    //The final optimized image
    SDL_Surface* optimizedSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Convert surface to screen format
        optimizedSurface = SDL_ConvertSurface( loadedSurface, gScreenSurface->format, NULL );
        if( optimizedSurface == NULL )
        {
            printf( "Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return optimizedSurface;
}
















// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// pika
int main(int argc, char *argv[])
{
    // network variables for TMR control
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    // image 
    image = fopen("rcv.jpg", "wb");

    // TMR control variables
    double *vv;
    double *wv;
    vv = (double *)malloc(3*sizeof(double));
    int duty[3];  
    int id = 0;
    double elapsed_time;

    float f_vv[3];
    float gain;
    int i;
    int ll = 0;
    int lr = 0;

    int video_toggle = 0;
    int capture = 0;
    int cam_tog = 0;

    int count = 0;

    double *temp = (double *) malloc(3*sizeof(double *));

    // ./Remote_Control_PC hostname(ip) port#
    struct timeval tvBegin, tvEnd, tvDiff;

    gettimeofday(&tvBegin, NULL);

    int TMR_TOG;
    





    printf("NS for TMR\n");

    
    // ************  network set up for TMR control ***********************

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(argv[1], TMR_PORT, &hints, &servinfo)) != 0) {
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

    // ************  network set up for TMR control End ***********************
    
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {  
        graySurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        SDL_FillRect(graySurface, NULL, SDL_MapRGB(graySurface->format, 128, 128, 128));
        SDL_BlitSurface( graySurface, NULL, gScreenSurface, NULL );
        SDL_UpdateWindowSurface( gWindow );

        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        SDL_Thread* threadID;
        threadID = SDL_CreateThread( threadFunction, "LazyThread", (void*)NULL);

        int xDir;
        int yDir;

        int prev_move;

        int status;

        //While application is running
        while( !quit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                //User presses a key
                
                else if( e.type == SDL_JOYAXISMOTION )
                {
                    if (e.jaxis.axis == 0) {
                        if (e.jaxis.value < -20000) {
                            xDir = -1;
                        } else if (e.jaxis.value > 20000) {
                            xDir = 1;
                        } else {
                            xDir = 0;
                        }
                    } 
                    else if (e.jaxis.axis == 1) {
                        if (e.jaxis.value < -20000) {
                            yDir = -1;
                        } else if (e.jaxis.value > 20000) {
                            yDir = 1;
                        } else {
                            yDir = 0;
                        }
                    }


                    if (xDir == 0 && yDir == 0 ) {
                        vv = stop(vv);
                        if (prev_move == STOP) {
                            continue;
                        }
                        prev_move = STOP;

                        printf("stop\n");
                    }

                    else if ( xDir == 1 && yDir == 0 ) {
                        vv = right(vv);
                        if (prev_move == RIGHT) {
                            continue;
                        }

                        prev_move = RIGHT;

                        printf("go right\n");
                    } else if ( xDir == 0 && yDir == 1 ) {
                        vv = backwards(vv);
                        if (prev_move == DOWN) {
                            continue;
                        }

                        prev_move = DOWN;
                        printf("go back\n");
                        
                    } else if ( xDir == -1 && yDir == 0) {
                        vv = left(vv);
                        if (prev_move == LEFT) {
                            continue;
                        }

                        prev_move = LEFT;
                        printf("go left\n");

                    } else if ( xDir == 0 && yDir == -1) {
                        vv = forwards(vv);
                        if (prev_move == UP) {
                            continue;
                        }

                        prev_move = UP;
                        printf("go forward\n");
                    }

                    gain = 1;

                    for (i = 0; i < 3; i++) {
                        f_vv[i] = gain * vv[i];
                    }

                    char message[50];
                    sprintf(message, "%d ", id);
                    

                    gettimeofday(&tvEnd, NULL);

                    char *temp2 = (char *) malloc(30*sizeof(char *));
                    char *temp3 = (char *) malloc(30*sizeof(char *));

                    memset(temp2, 0, sizeof(temp2));
                    memset(temp3, 0, sizeof(temp3));

                    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
                    sprintf(temp3, "%ld.%06ld ", tvDiff.tv_sec, tvDiff.tv_usec);
                    strncpy(temp2, temp3, strlen(temp3)-4);
                    strcat(temp2, " ");
                    strcat(message, temp2);


                    sprintf(temp2, "%.2f ", f_vv[0]);
                    strcat(message, temp2);
                    sprintf(temp2, "%.2f ", f_vv[1]);
                    strcat(message, temp2);
                    sprintf(temp2, "%.2f ", f_vv[2]);
                    strcat(message, temp2);
                    sprintf(temp2, "%d ", ll);
                    strcat(message, temp2);
                    sprintf(temp2, "%d", lr);
                    strcat(message, temp2);
            

                    printf("\nKey '%c'. cmd: %s\n", e.key.keysym.sym, message);

                    id++;

                    message[strlen(message)] = '\0';
                    
                    if ((numbytes = sendto(sockfd, message, sizeof(message), 0,
                             p->ai_addr, p->ai_addrlen)) == -1) {
                        perror("talker: sendto");
                        exit(1);
                    }

                    //printf("x: %d y: %d\n", xDir, yDir);
                    
                } else if ( e.type == SDL_JOYBUTTONDOWN ) {
                    // LB
                    printf("%d\n", e.jbutton.button);
                    if ( e.jbutton.button == 4 ) { 
                        if ( ll == 0 ) {
                            ll = 1;
                        } else {
                            ll = 0;
                        }
                        TMR_TOG = 1;
                        printf("left light toggle\n");
                    } else if ( e.jbutton.button == 5 ) {
                        if ( lr == 0 ) {
                            lr = 1;
                        } else {
                            lr = 0;
                        }
                        TMR_TOG = 1;
                        printf("right light toggle\n");
                    } else if ( e.jbutton.button == 6 ) {
                        vv = rotatecw(vv);
                        TMR_TOG = 1;
                        printf("rotate cw\n");
                    } else if ( e.jbutton.button == 7 ) {
                        vv = rotateccw(vv);
                        TMR_TOG = 1;
                        printf("rotate ccw\n");
                    } else if ( e.jbutton.button == 1 ) {
                        vv = stop(vv);
                        TMR_TOG = 1;
                        printf("stopped\n");
                    } else if ( e.jbutton.button == 0 ) {
                        capture = 1;
                        cam_tog = 1;
                        printf("take pic\n");
                    } else if ( e.jbutton.button == 3 ) {
                        video_toggle = 1;
                        cam_tog = 1;
                        printf("video toggle\n");
                    }

                    char command[30];
                    int TMR_TOG2 = 0;

                    if (cam_tog == 1) {
                        if (capture == 1) { 
                            memset(command, 0, sizeof(command));
                            
                            snprintf(command, sizeof(command), "mv rcv.jpg img/image%d.jpg", count);

                            status = system(command);
                            
                            count++;
                        } 
                        if (video_toggle == 1) {
                            while(1) {
                                loadMedia(1);

                                if (SDL_PollEvent( &e ) != 0) {
                                    if ( e.type == SDL_JOYAXISMOTION ) {
                                        if (e.jaxis.axis == 0) {
                                            if (e.jaxis.value < -20000) {
                                                xDir = -1;
                                            } else if (e.jaxis.value > 20000) {
                                                xDir = 1;
                                            } else {
                                                xDir = 0;
                                            }
                                        } 
                                        else if (e.jaxis.axis == 1) {
                                            if (e.jaxis.value < -20000) {
                                                yDir = -1;
                                            } else if (e.jaxis.value > 20000) {
                                                yDir = 1;
                                            } else {
                                                yDir = 0;
                                            }
                                        }


                                        if (xDir == 0 && yDir == 0 ) {
                                            vv = stop(vv);
                                            if (prev_move == STOP) {
                                                continue;
                                            }
                                            prev_move = STOP;
                                            TMR_TOG2 = 1;
                                            printf("stop\n");
                                        }

                                        else if ( xDir == 1 && yDir == 0 ) {
                                            vv = right(vv);
                                            if (prev_move == RIGHT) {
                                                continue;
                                            }

                                            prev_move = RIGHT;
                                            TMR_TOG2 = 1;
                                            printf("go right\n");
                                        } else if ( xDir == 0 && yDir == 1 ) {
                                            vv = backwards(vv);
                                            if (prev_move == DOWN) {
                                                continue;
                                            }

                                            prev_move = DOWN;
                                            TMR_TOG2 = 1;
                                            printf("go back\n");
                                            
                                        } else if ( xDir == -1 && yDir == 0) {
                                            vv = left(vv);
                                            if (prev_move == LEFT) {
                                                continue;
                                            }

                                            prev_move = LEFT;
                                            TMR_TOG2 = 1;
                                            printf("go left\n");

                                        } else if ( xDir == 0 && yDir == -1) {
                                            vv = forwards(vv);
                                            if (prev_move == UP) {
                                                continue;
                                            }

                                            prev_move = UP;
                                            TMR_TOG2 = 1;
                                            printf("go forward\n");
                                        }
                                    } else if (e.type == SDL_JOYBUTTONDOWN) {
                                        if ( e.jbutton.button == 4 ) { 
                                            if ( ll == 0 ) {
                                                ll = 1;
                                            } else {
                                                ll = 0;
                                            }
                                            TMR_TOG2 = 1;
                                            printf("left light toggle\n");
                                        } else if ( e.jbutton.button == 5 ) {
                                            if ( lr == 0 ) {
                                                lr = 1;
                                            } else {
                                                lr = 0;
                                            }
                                            TMR_TOG2 = 1;
                                            printf("right light toggle\n");
                                        } else if ( e.jbutton.button == 6 ) {
                                            vv = rotatecw(vv);
                                            TMR_TOG2 = 1;
                                            printf("rotate cw\n");
                                        } else if ( e.jbutton.button == 7 ) {
                                            vv = rotateccw(vv);
                                            TMR_TOG2 = 1;
                                            printf("rotate ccw\n");
                                        } else if ( e.jbutton.button == 1 ) {
                                            vv = stop(vv);
                                            TMR_TOG2 = 1;
                                            printf("stopped\n");
                                        } else if ( e.jbutton.button == 0 ) {
                                            memset(command, 0, sizeof(command));
                                
                                            snprintf(command, sizeof(command), "mv rcv.jpg img/image%d.jpg", count);

                                            status = system(command);
                                            
                                            count++; 
                                        } else if ( e.jbutton.button == 3 ) {
                                            loadMedia(0);
                                            video_toggle = 0;
                                        }

                                        

                                    }
                                    if (video_toggle == 0) {
                                        break;
                                    }

                                    if (TMR_TOG2 == 0) {
                                        continue;
                                    }

                                    gain = 1;

                                    for (i = 0; i < 3; i++) {
                                        f_vv[i] = gain * vv[i];
                                    }

                                    char message[50];
                                    sprintf(message, "%d ", id);
                                    

                                    gettimeofday(&tvEnd, NULL);

                                    char *temp2 = (char *) malloc(30*sizeof(char *));
                                    char *temp3 = (char *) malloc(30*sizeof(char *));

                                    memset(temp2, 0, sizeof(temp2));
                                    memset(temp3, 0, sizeof(temp3));

                                    timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
                                    sprintf(temp3, "%ld.%06ld ", tvDiff.tv_sec, tvDiff.tv_usec);
                                    strncpy(temp2, temp3, strlen(temp3)-4);
                                    strcat(temp2, " ");
                                    strcat(message, temp2);


                                    sprintf(temp2, "%.2f ", f_vv[0]);
                                    strcat(message, temp2);
                                    sprintf(temp2, "%.2f ", f_vv[1]);
                                    strcat(message, temp2);
                                    sprintf(temp2, "%.2f ", f_vv[2]);
                                    strcat(message, temp2);
                                    sprintf(temp2, "%d ", ll);
                                    strcat(message, temp2);
                                    sprintf(temp2, "%d", lr);
                                    strcat(message, temp2);
                            

                                    printf("\nKey '%c'. cmd: %s\n", e.key.keysym.sym, message);

                                    id++;

                                    message[strlen(message)] = '\0';
                                    
                                    if ((numbytes = sendto(sockfd, message, sizeof(message), 0,
                                             p->ai_addr, p->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }

                                    TMR_TOG2 = 0;                                    

                                }         
                                
                            }

                        } else {
                            loadMedia(0);
                        }

                        cam_tog = 0;
                        capture = 0;
                        continue;
                    }

                    if (TMR_TOG) {
                        gain = 1;

                        for (i = 0; i < 3; i++) {
                            f_vv[i] = gain * vv[i];
                        }

                        char message[50];
                        sprintf(message, "%d ", id);
                        

                        gettimeofday(&tvEnd, NULL);

                        char *temp2 = (char *) malloc(30*sizeof(char *));
                        char *temp3 = (char *) malloc(30*sizeof(char *));

                        memset(temp2, 0, sizeof(temp2));
                        memset(temp3, 0, sizeof(temp3));

                        timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
                        sprintf(temp3, "%ld.%06ld ", tvDiff.tv_sec, tvDiff.tv_usec);
                        strncpy(temp2, temp3, strlen(temp3)-4);
                        strcat(temp2, " ");
                        strcat(message, temp2);


                        sprintf(temp2, "%.2f ", f_vv[0]);
                        strcat(message, temp2);
                        sprintf(temp2, "%.2f ", f_vv[1]);
                        strcat(message, temp2);
                        sprintf(temp2, "%.2f ", f_vv[2]);
                        strcat(message, temp2);
                        sprintf(temp2, "%d ", ll);
                        strcat(message, temp2);
                        sprintf(temp2, "%d", lr);
                        strcat(message, temp2);
                

                        printf("\nKey '%c'. cmd: %s\n", e.key.keysym.sym, message);

                        id++;

                        message[strlen(message)] = '\0';
                        
                        if ((numbytes = sendto(sockfd, message, sizeof(message), 0,
                                 p->ai_addr, p->ai_addrlen)) == -1) {
                            perror("talker: sendto");
                            exit(1);
                        }
                        TMR_TOG = 0;
                        continue;
                    }


                    

                }
            }
        }
        
    }

    //Free resources and close SDL
    close();
    fclose(image);



    

    return 0;
}