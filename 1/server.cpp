//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sem.h>
#include <semaphore.h>

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"

#define STR_UP "UP"
#define STR_UP_OK "UP OK\n"
#define STR_ERR "ERR\n"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int g_debug = LOG_INFO;

sem_t* g_mutex = nullptr;

void log_msg( int t_log_level, const char *t_form, ... )
{
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n" };

    if ( t_log_level && t_log_level > g_debug ) return;

    char l_buf[ 1024 ];
    va_list l_arg;
    va_start( l_arg, t_form );
    vsprintf( l_buf, t_form, l_arg );
    va_end( l_arg );

    switch ( t_log_level )
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf( stdout, out_fmt[ t_log_level ], l_buf );
        break;

    case LOG_ERROR:
        fprintf( stderr, out_fmt[ t_log_level ], errno, strerror( errno ), l_buf );
        break;
    }
}

//***************************************************************************
// help

void help( int t_narg, char **t_args )
{
    if ( t_narg <= 1 || !strcmp( t_args[ 1 ], "-h" ) )
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n", t_args[ 0 ] );

        exit( 0 );
    }

    if ( !strcmp( t_args[ 1 ], "-d" ) )
        g_debug = LOG_DEBUG;
}

//***************************************************************************

int up()
{
    sem_post(g_mutex);
    return 1;
}

int down()
{
    sem_wait(g_mutex);
    return 1;
}

void new_client(int l_sock_client, int l_sock_listen)
{
    pollfd l_read_poll[ 2 ];

    l_read_poll[ 0 ].fd = STDIN_FILENO;
    l_read_poll[ 0 ].events = POLLIN;
    l_read_poll[ 1 ].fd = l_sock_listen;
    l_read_poll[ 1 ].events = POLLIN;

    int pid;
    if((pid = fork()) == 0)
    {
        while(1)
        {
            //comunication
            char l_buf[ 256 ];

            // select from fds
            int l_poll = poll( l_read_poll, 2, -1 );

            if(l_poll<0)
            {
                log_msg( LOG_ERROR, "Function poll failed!" );
                exit( 1 );
            }
            /*
            // data on stdin?
            if ( l_read_poll[ 0 ].revents & POLLIN )
            {
                // read data from stdin
                int l_len = read( STDIN_FILENO, l_buf, sizeof( l_buf ) );
                if ( l_len < 0 )
                        log_msg( LOG_ERROR, "Unable to read data from stdin." );
                else
                        log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l_len );

                // send data to client
                l_len = write( l_sock_client, l_buf, l_len );
                if ( l_len < 0 )
                        log_msg( LOG_ERROR, "Unable to send data to client." );
                else
                        log_msg( LOG_DEBUG, "Sent %d bytes to client.", l_len );
            }
            */
           //data from client?
           if(l_read_poll[1].revents & POLLIN)
           {
                //read data from socket
                int l_len = read( l_sock_client, l_buf, sizeof( l_buf ) );
                if (!l_len)
                {
                    log_msg( LOG_DEBUG, "Client closed socket!" );
                    close( l_sock_client );
                    break;
                }
                else if (l_len < 0)
                {
                    log_msg( LOG_ERROR, "Unable to read data from client." );
                }
                else
                {
                    log_msg( LOG_DEBUG, "Read %d bytes from client.", l_len );
                }
                if(!strncasecmp(l_buf, STR_UP, strlen(STR_UP)))
                {
                    printf("%s", STR_UP);
                    up();
                    send(l_sock_client, STR_UP_OK, strlen(STR_UP_OK), 0);
                }    
           }
        }
    }
}

int main( int t_narg, char **t_args )
{
    if ( t_narg <= 1 ) help( t_narg, t_args );

    int l_port = 0;

    // parsing arguments
    for ( int i = 1; i < t_narg; i++ )
    {
        if ( !strcmp( t_args[ i ], "-d" ) )
            g_debug = LOG_DEBUG;

        if ( !strcmp( t_args[ i ], "-h" ) )
            help( t_narg, t_args );

        if ( *t_args[ i ] != '-' && !l_port )
        {
            l_port = atoi( t_args[ i ] );
            break;
        }
    }

    if ( l_port <= 0 )
    {
        log_msg( LOG_INFO, "Bad or missing port number %d!", l_port );
        help( t_narg, t_args );
    }

    log_msg( LOG_INFO, "Server will listen on port: %d.", l_port );

    // socket creation
    int l_sock_listen = socket( AF_INET, SOCK_STREAM, 0 );
    if ( l_sock_listen == -1 )
    {
        log_msg( LOG_ERROR, "Unable to create socket.");
        exit( 1 );
    }

    in_addr l_addr_any = { INADDR_ANY };
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons( l_port );
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if ( setsockopt( l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof( l_opt ) ) < 0 )
      log_msg( LOG_ERROR, "Unable to set socket option!" );

    // assign port number to socket
    if ( bind( l_sock_listen, (const sockaddr * ) &l_srv_addr, sizeof( l_srv_addr ) ) < 0 )
    {
        log_msg( LOG_ERROR, "Bind failed!" );
        close( l_sock_listen );
        exit( 1 );
    }

    // listenig on set port
    if ( listen( l_sock_listen, 1 ) < 0 )
    {
        log_msg( LOG_ERROR, "Unable to listen on given port!" );
        close( l_sock_listen );
        exit( 1 );
    }

    log_msg( LOG_INFO, "Enter 'quit' to quit server." );

    int sem_id = semget( IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        log_msg( LOG_ERROR, "Unable to create semaphore!" );
        exit( 1 );
    }
    if(semctl(sem_id, 0, SETVAL, 1) == -1)
    {
        log_msg( LOG_ERROR, "Unable to set semaphore!" );
        exit( 1 );
    }

    // go!
    while ( 1 )
    {
        int l_sock_client = -1;

        // list of fd sources
        pollfd l_read_poll[ 2 ];

        l_read_poll[ 0 ].fd = STDIN_FILENO;
        l_read_poll[ 0 ].events = POLLIN;
        l_read_poll[ 1 ].fd = l_sock_listen;
        l_read_poll[ 1 ].events = POLLIN;

        while ( 1 ) // wait for new client
        {
            // select from fds
            int l_poll = poll( l_read_poll, 2, -1 );

            if ( l_poll < 0 )
            {
                log_msg( LOG_ERROR, "Function poll failed!" );
                exit( 1 );
            }

            if ( l_read_poll[ 0 ].revents & POLLIN )
            { // data on stdin
                char buf[ 128 ];
                int len = read( STDIN_FILENO, buf, sizeof( buf) );
                if ( len < 0 )
                {
                    log_msg( LOG_DEBUG, "Unable to read from stdin!" );
                    exit( 1 );
                }

                log_msg( LOG_DEBUG, "Read %d bytes from stdin" );
                // request to quit?
                if ( !strncmp( buf, STR_QUIT, strlen( STR_QUIT ) ) )
                {
                    log_msg( LOG_INFO, "Request to 'quit' entered.");
                    close( l_sock_listen );
                    exit( 0 );
                }
            }

            if ( l_read_poll[ 1 ].revents & POLLIN )
            { // new client?
                sockaddr_in l_rsa;
                int l_rsa_size = sizeof( l_rsa );
                // new connection
                l_sock_client = accept( l_sock_listen, ( sockaddr * ) &l_rsa, ( socklen_t * ) &l_rsa_size );
                if ( l_sock_client == -1 )
                {
                    log_msg( LOG_ERROR, "Unable to accept new client." );
                    close( l_sock_listen );
                    exit( 1 );
                }
                uint l_lsa = sizeof( l_srv_addr );
                // my IP
                getsockname( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
                log_msg( LOG_INFO, "My IP: '%s'  port: %d",
                                 inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );
                // client IP
                getpeername( l_sock_client, ( sockaddr * ) &l_srv_addr, &l_lsa );
                log_msg( LOG_INFO, "Client IP: '%s'  port: %d",
                                 inet_ntoa( l_srv_addr.sin_addr ), ntohs( l_srv_addr.sin_port ) );

                break;
            }

        } // while wait for client

        // change source from sock_listen to sock_client
        l_read_poll[ 1 ].fd = l_sock_client;

        while ( 1  )
        { // communication
            char l_buf[ 256 ];

            // select from fds
            int l_poll = poll( l_read_poll, 2, -1 );

            if ( l_poll < 0 )
            {
                log_msg( LOG_ERROR, "Function poll failed!" );
                exit( 1 );
            }

            // data on stdin?
            if ( l_read_poll[ 0 ].revents & POLLIN )
            {
                // read data from stdin
                int l_len = read( STDIN_FILENO, l_buf, sizeof( l_buf ) );
                if ( l_len < 0 )
                    log_msg( LOG_ERROR, "Unable to read data from stdin." );
                else
                    log_msg( LOG_DEBUG, "Read %d bytes from stdin.", l_len );

                // send data to client
                l_len = write( l_sock_client, l_buf, l_len );
                if ( l_len < 0 )
                    log_msg( LOG_ERROR, "Unable to send data to client." );
                else
                    log_msg( LOG_DEBUG, "Sent %d bytes to client.", l_len );
            }
            // data from client?
            if ( l_read_poll[ 1 ].revents & POLLIN )
            {
                // read data from socket
                int l_len = read( l_sock_client, l_buf, sizeof( l_buf ) );
                if ( !l_len )
                {
                    log_msg( LOG_DEBUG, "Client closed socket!" );
                    close( l_sock_client );
                    break;
                }
                else if ( l_len < 0 )
                {
                    log_msg( LOG_ERROR, "Unable to read data from client." );
                    close( l_sock_client );
                    break;
                }
                else
                    log_msg( LOG_DEBUG, "Read %d bytes from client.", l_len );

                // write data to client
                l_len = write( STDOUT_FILENO, l_buf, l_len );
                if ( l_len < 0 )
                    log_msg( LOG_ERROR, "Unable to write data to stdout." );

                // close request?
                if ( !strncasecmp( l_buf, "close", strlen( STR_CLOSE ) ) )
                {
                    log_msg( LOG_INFO, "Client sent 'close' request to close connection." );
                    close( l_sock_client );
                    log_msg( LOG_INFO, "Connection closed. Waiting for new client." );
                    break;
                }

                struct sembuf sem_op;
                //fork a child processs to handle client commads
                pid_t pid = fork();
                if(pid == -1)
                {
                    log_msg( LOG_ERROR, "Unable to fork!" );
                    exit( 1 );
                }
                else if (pid == 0)
                {
                    //child process
                    while(1)
                    {
                        //receive command from client
                        //..
                        //process the command
                        // declare the variable "command"
                        char command[100];
                        if(strcmp(command, "UP\n") == 0)
                        {
                            sem_op.sem_num = 0;
                            sem_op.sem_op = -1;
                            sem_op.sem_flg = 0;
                            if(semop(sem_id, &sem_op, 1) == -1)
                            {
                                log_msg( LOG_ERROR, "Unable to acquire semaphore!" );
                                exit( 1 );
                            }
                            //send "UP OK" response to client
                            const char* response = "UP OK";
                            l_len = write(l_sock_client, response, strlen(response));
                            //release the semaphore
                            sem_op.sem_op = 1;
                            if(semop(sem_id, &sem_op, 1) == -1)
                            {
                                log_msg( LOG_ERROR, "Unable to release semaphore!" );
                                exit( 1 );
                            }
                            else if(strcmp(command, "DOWN\n") == 0)
                            {
                                //acquire the semaphore
                                struct sembuf sem_op;
                                sem_op.sem_num = 0;
                                sem_op.sem_op = -1;
                                sem_op.sem_flg = 0;
                                if(semop(sem_id, &sem_op, 1) == -1)
                                {
                                    log_msg( LOG_ERROR, "Unable to acquire semaphore!" );
                                    exit( 1 );
                                }
                                //send "DOWN OK" response to client
                                const char* response2 = "DOWN OK";
                                l_len = write(l_sock_client, response2, strlen(response));
                                //release the semaphore
                                sem_op.sem_op = 1;
                                if(semop(sem_id, &sem_op, 1) == -1)
                                {
                                    log_msg( LOG_ERROR, "Unable to release semaphore!" );
                                    exit( 1 );
                                }
                            }
            
                            else
                            {
                                //parent process
                                //...
                                close( l_sock_client );
                            }
                        }
                    }
                }
                
            }
            // request for quit
            if ( !strncasecmp( l_buf, "quit", strlen( STR_QUIT ) ) )
            {
                close( l_sock_listen );
                close( l_sock_client );
                log_msg( LOG_INFO, "Request to 'quit' entered" );
                exit( 0 );
            }
        } // while communication
    } // while ( 1 )

    return 0;
}



