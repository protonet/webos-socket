/**

  Copyright (C) 2010 Protonet.info  All rights reserved.
**/

#include <stdio.h>
#include <math.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#include "SDL.h"
#include "PDL.h"

#include <syslog.h>
#define PACKAGEID      "com.protonet.sockettest"

#define MAX_BUFFER_LEN 1024

volatile bool Paused = false;

int sock;                        /* Socket -- GLOBAL for signal handler */

void SIGIOHandler(int, siginfo_t *info, void *uap); /* Function to handle SIGIO */

int doOpenSocket(const char *ip_address, unsigned short port){
    struct sockaddr_in sockaddr;     /* Server address */
    struct sigaction handler;        /* Signal handling action definition */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket() failed");
        return -1;
    }

    /* Set up the server address structure */
    memset(&sockaddr, 0, sizeof(sockaddr));       /* Zero out structure */
    sockaddr.sin_family = AF_INET;                /* Internet family */
    sockaddr.sin_addr.s_addr = inet_addr(ip_address); /* Any incoming interface */
    sockaddr.sin_port = htons(port);              /* Port */

    /* Bind to the local address */
    if (connect(sock, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0){
        printf("connect() failed");
        return -1;
    }

    /* Set signal handler for SIGIO */
    handler.sa_sigaction = SIGIOHandler;
    /* Create mask that mask all signals */
    if (sigfillset(&handler.sa_mask) < 0){
        printf("sigfillset() failed");
        return -1;
    }
    /* No flags */
    handler.sa_flags = 0;

    if (sigaction(SIGIO|SIGHUP, &handler, 0) < 0){
        printf("sigaction() failed for SIGIO");
        return -1;
    }

    /* We must own the socket to receive the SIGIO message */
    if (fcntl(sock, F_SETOWN, getpid()) < 0){
        printf("Unable to set process owner to us");
        return -1;
    }

    /* Arrange for nonblocking I/O and SIGIO delivery */
    if (fcntl(sock, F_SETFL, O_NONBLOCK | FASYNC) < 0){
        printf("Unable to put client sock into non-blocking/async mode");
        return -1;
    }

}

void doCloseSocket(){
    close(sock);
}

void didReceiveData(const char *theData){
    const char *params[1];
    params[0] = theData;
    PDL_Err mjErr = PDL_CallJS("didReceiveData", params, 1);
    if ( mjErr != PDL_NOERROR )
    {
        printf("error: %s\n", PDL_GetError());
    }
}

void SIGIOHandler(int signum, siginfo_t *info, void *uap)
{
    struct sockaddr_in serveraddr;    /* Address of datagram source */
    unsigned int clntLen;             /* Address length */
    int recvMsgSize;                  /* Size of datagram */
    char echoBuffer[MAX_BUFFER_LEN];  /* Datagram buffer */

    /* printf("signal %d si_code: %d error: %d\n", info->si_signo, info->si_code, info->si_errno); */
    do  /* As long as there is input... */
    {
        clntLen = sizeof(serveraddr);

        if ((recvMsgSize = recvfrom(sock, echoBuffer, MAX_BUFFER_LEN, 0,
                                    (struct sockaddr *) &serveraddr, &clntLen)) > 0)
        {
            echoBuffer[recvMsgSize] = '\0';
            didReceiveData(echoBuffer);
            if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *) 
                       &serveraddr, sizeof(serveraddr)) != recvMsgSize){
                printf("sendto() failed");
                return;
            }
        }
        else if (recvMsgSize == 0)
        {
            doCloseSocket();
            didReceiveData("connection closed");
            return;
        }
        else
        {
            /* Only acceptable error: recvfrom() would have blocked */
            if (errno != EWOULDBLOCK){
                printf("recvfrom() failed");
                return;
            }
        }
    }  while (recvMsgSize > 0);
    /* Nothing left to receive */
}

PDL_bool openSocket(PDL_JSParameters *params)
{
    if (PDL_GetNumJSParams(params) < 2 )
    {
        PDL_JSException(params, "You must provide the ip address and port");
        return PDL_TRUE;
    }
    const char *ip_address = PDL_GetJSParamString(params, 0);
    int port               = PDL_GetJSParamInt(params, 1);

    syslog(LOG_INFO, "connecting socket");
    doOpenSocket(ip_address, port);

    return PDL_TRUE;
}

PDL_bool closeSocket(PDL_JSParameters *params)
{
    doCloseSocket();

    return PDL_TRUE;
}


int main(int argc, char** argv)
{
    // open syslog in case we want to print out some debugging
    openlog(PACKAGEID, 0, LOG_USER);

    // Initialize the SDL library
    int result = SDL_Init(SDL_INIT_VIDEO);
    if ( result != 0 )
    {
        printf("Could not init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    PDL_Init(0);

    // register the Mojo callbacks
    PDL_RegisterJSHandler("openSocket", openSocket);
    PDL_RegisterJSHandler("closeSocket", closeSocket);

    PDL_JSRegistrationComplete();
    didReceiveData("initialized");

    // Event descriptor
    SDL_Event Event;

    do {
        if (Paused)
        {
            SDL_WaitEvent(&Event);
            if (Event.type == SDL_ACTIVEEVENT)
            {
                if ((Event.active.state & SDL_APPACTIVE) &&	(Event.active.gain == 1))
                {
                    Paused = false;
                }
            }
        }
        else
        {
            while (SDL_PollEvent(&Event)) {
                switch (Event.type) 
                {
                    // List of keys that have been pressed
                    case SDL_KEYDOWN:
                        switch (Event.key.keysym.sym) 
                        {
                            // Escape forces us to quit the app
                            case SDLK_ESCAPE:
                                Event.type = SDL_QUIT;
                                break;
                        }
                        break;

                    // handle deactivation by pausing our animation
                    case SDL_ACTIVEEVENT:
                        if ((Event.active.state & SDL_APPACTIVE) &&	(Event.active.gain == 0))
                        {
                            Paused = true;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    } while (Event.type != SDL_QUIT);


    PDL_Quit();
    SDL_Quit();

    return 0;
}
