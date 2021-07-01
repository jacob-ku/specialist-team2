#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"


bool checkCmdArguments(
                    int     argc,
                    char    *argv[],
                    int     &portNum,
                    bool    &bSecureMode
                    )
{
    if (argc < 3) return false;

    // Port Number for Listen Socket
    portNum     = atoi(argv[1]);
    if(portNum < 1024 || portNum > 65535)
        return false;

    // Secure or Not
    if(!strcmp(argv[2], "0")) bSecureMode = false;
    else if(!strcmp(argv[2], "1")) bSecureMode = true;
    else return false;

    return true;
}