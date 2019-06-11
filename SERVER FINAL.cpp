#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <iostream>
#include <poll.h>
#include <sys/wait.h>
#include <plist.h>
#include <arpa/inet.h>
#define TRUE 1
#define error_handler(msg) \
do \
{  \
    perror(msg);   \
    exit(EXIT_FAILURE);  \
}  \
while (0);
#define TRUE 1

processlist p [100];
int processcount = 0;

conn_list c [100];
int conncount = 0;

using namespace std;




pid_t Run(string data, int msgsock)
{
    char buf2 [1024];
    char c_data [100];
    strcpy(c_data, data.c_str());
    int cnt = sprintf(buf2, "Opening %s\n", c_data);
    write(msgsock, buf2, cnt);
    int pid = fork();
    if (pid == 0)
    {
        execlp(c_data, NULL);
        write(msgsock,"No such file or directory\n", 26);
    }
    else if (pid > 0)
    {
        sleep(1);
        return pid;
    }
}


int Mult(string data)
{
    int total = 1;
    int num;
    char c_data [100];

    strcpy(c_data, data.c_str());
    char *token = strtok(c_data, " ");
    while (token != NULL)
    {
        sscanf(token, "%d", &num);
        total = total * num;
        token = strtok(NULL, " ");
    }
    return total;
}


int Sub(string data)
{
    int total;
    int num;
    char c_data [100];

    strcpy(c_data, data.c_str());
    char *token = strtok(c_data, " ");
    sscanf(token, "%d", &num);
    total = num;
    token = strtok(NULL, " ");
    while (token != NULL)
    {
        sscanf(token, "%d", &num);
        total = total - num;
        token = strtok(NULL, " ");
    }
    return total;
}


int Div(string data, int msgsock)
{
    int total;
    int num;
    char c_data [100];

    strcpy(c_data, data.c_str());
    char *token = strtok(c_data, " ");
    sscanf(token, "%d", &num);
    total = num;
    token = strtok(NULL, " ");
    while (token != NULL)
    {
        if (num == 0)
        {
            write(msgsock, "Division by 0 not allowed!\n", 27);
            token = strtok(NULL, " ");
			total = 0;
        }
        else
        {
            sscanf(token, "%d", &num);
            total = total / num;
            token = strtok(NULL, " ");
        }
    }
    return total;
}


int Add(string data)
{
    int total = 0;
    int num;
    char c_data [100];

    strcpy(c_data, data.c_str());
    char *token = strtok(c_data, " ");
    while (token != NULL)
    {
        sscanf(token, "%d", &num);
        total = total + num;
        token = strtok(NULL, " ");
    }
    return total;
}


void List(int msgsock)
{
    int i = 0;
    string plist;
    char c_list [1024];
    bzero(c_list, sizeof(c_list));

    for (i = 0; i < processcount; i++)
    {
        if (p[i].status == "ACTIVE")
        {
            string id = to_string(p[i].processid);
            string start = ctime(&p[i].start_time);
            plist = id + " " + p[i].processname + " " + start + " " + p[i].status + "\n";
            strcpy(c_list, plist.c_str());
            write(msgsock, c_list, sizeof(c_list));
        }
        else
        {
            string id = to_string(p[i].processid);
            string start = ctime(&p[i].start_time);
            string finish = ctime(&p[i].end_time);
            string elap = to_string(p[i].elap_time);
            plist = id + " " + p[i].processname + " " + start + " " + p[i].status + " " + finish + " " + elap + "\n";
            strcpy(c_list, plist.c_str());

            write(msgsock, c_list, sizeof(c_list));
        }
    }
}

void ConnList()
{
    int i = 0;
    string clist;
    char c_clist [1024];
    bzero(c_clist, sizeof(c_clist));

    for (i = 0; i < conncount; i++)
    {
        string c_id = to_string(c[i].messagesock);
        string addr(c[i].IP);
        string hst = to_string(c[i].host);
        clist = c_id + "   " + addr + "   " + hst + "\n";
        strcpy(c_clist, clist.c_str());
        write(STDOUT_FILENO, c_clist, sizeof(c_clist));
    }
}


void Help(int msgsock)
{
    char help [1024];
    bzero(help, sizeof(help));
    string h = " ADD <num1> <num2> <num3> … … … <num n> : Returns sum of the specified numbers\n SUM <num1> <num2> <num3> … … … <num n> : Returns difference between the specified numbers\n DIV <num1> <num2> <num3> … … … <num n> : Returns quotient of the specified numbers\n MULT <num1> <num2> <num3> … … … <num n> : Returns product of the specified numbers\n RUN <executable> : Opens an executable file\n LIST : Prints a list of all active and inactive processes\n KILL <process id> : Terminates the process whose process ID is given\n EXIT : Disconnects with the Server\n HELP : Provides a list of valid commands that a client can enter\n";
    strcpy(help, h.c_str());
    write(msgsock, help, sizeof(help));
}


void Help2()
{
    char help [1024];
    bzero(help, sizeof(help));
    string h = " LIST CONN : Returns a list of all connections with the server\n LIST PROC : Returns a complete list of processes of all clients\n PRINT <message> : Sends message to all clients\n PRINT <message> <IP Address> : Sends message to client whose IP is specified\n EXIT <IP Address> : Disconnects client whose IP is specified\n EXIT : Disconnects all clients\n HELP : Provides a list of valid commands that can be entered on the server\n ";
    strcpy(help, h.c_str());
    write(STDOUT_FILENO, help, sizeof(help));
}



int main()
{
	int sock, length, length2;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int msgsock;
	char buf[1024];
	int rval;
    string ins;
    string data;
    string msg;
    string ip;
    char buf2 [1024];
    struct pollfd fds[2];
    int timeout = (-1 * 60 * 1000);

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("opening stream socket");
		exit(1);
	}
	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
	if (bind(sock, (struct sockaddr *) &server, sizeof(server)))
	{
		perror("binding stream socket");
		exit(1);
	}
	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length))
	{
		perror("getting socket name");
		exit(1);
	}
	printf("Socket has port #%d\n", ntohs(server.sin_port));
	fflush(stdout);

	/* Start accepting connections */
	listen(sock, 5);


	fds[0].fd = sock;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while(true)
    {
        int rc = poll(fds, 2, timeout);
        if (rc == 0)
        {
            write(STDOUT_FILENO, "Poll() Timed Out", strlen("Poll() Timed Out"));
        }


        if(fds[0].revents == POLLIN)
        {
                msgsock = accept(sock, 0, 0);
                //(msgsock,(struct sockaddr *)&server)
                c[conncount].IP = std::string(inet_ntoa(client.sin_addr));
                getsockname(sock, (struct sockaddr *) &client, (socklen_t*) &length2);
                c[conncount].messagesock = msgsock;
               // inet_ntop(AF_INET, &(server.sin_addr.s_addr), c[conncount].IP, INET_ADDRSTRLEN);
                //sprintf(c[conncount].IP, "%s", inet_ntoa(server.sin_addr));
                c[conncount].host = ntohs(server.sin_port);
                conncount = conncount + 1;

                    int pid = fork();
                    if (pid == 0)
                    {
                        do
                        {
                            bzero(buf, sizeof(buf));
                            rval = read(msgsock, buf, 1024);
//                          write(STDOUT_FILENO, "Client says: ", 14);
//                          write(STDOUT_FILENO, buf, rval);
                            buf[rval - 1] = '\0';
                            string str(buf);
                            ins = str.substr(0, str.find(' '));
                            data = str.substr(str.find(' ') + 1);

                            if (str == "EXIT")
                            {
//                                close(msgsock);
                                exit(0);
                                //write(msgsock, "Disconnected\n", 13);
                            }
                            else if (ins == "RUN")
                            {
                                int pid = Run(data, msgsock);
                                p[processcount].processid = pid;
                                p[processcount].processname = data;
                                p[processcount].status = "ACTIVE";
                                p[processcount].start_time = time(NULL);
                                p[processcount].end_time = 0;
                                p[processcount].elap_time = 0;
                                processcount = processcount + 1;
                            }
                            else if (ins == "ADD")
                            {
                                int total = Add(data);
                                int count = sprintf(buf2, "%d\n", total);
                                write(msgsock, buf2, count);
                            }
                            else if (ins == "MULT")
                            {
                                int total = Mult(data);
                                int count = sprintf(buf2, "%d\n", total);
                                write(msgsock, buf2, count);
                            }
                            else if (ins == "DIV")
                            {
                                int total = Div(data, msgsock);
                                int count = sprintf(buf2, "%d\n", total);
                                write(msgsock, buf2, count);
                            }
                            else if (ins == "SUB")
                            {
                                int total = Sub(data);
                                int count = sprintf(buf2, "%d\n", total);
                                write(msgsock, buf2, count);
                            }
                            else if (ins == "KILL")
                            {
                                int pid = stoi(data);
                                kill(pid, SIGTERM);
                                int j;
                                int k;
                                for (j = 0; j < processcount; j++)
                                {
                                    if (p[j].processid == pid)
                                    {
                                        k = j;
                                    }
                                }
                                p[k].status = "INACTIVE";
                                p[k].end_time = time(NULL);
                                p[k].elap_time = difftime(p[k].end_time, p[k].start_time);
                            }
                            else if (ins == "LIST")
                            {
                                List(msgsock);
                            }
                            else if (ins == "HELP")
                            {
                                Help(msgsock);
                            }
                            else
                            {
                                write(msgsock, "Invalid Command\n", 16);
                            }
                        }

                    while (rval != 0 || buf != "EXIT");

                    }
                    close(msgsock);
            }



            if(fds[1].revents == POLLIN)
            {
                bzero(buf, sizeof(buf));
                rval = read(STDIN_FILENO, buf, 1024);
                buf[rval - 1] = '\0';
                string str(buf);
                ins = str.substr(0, str.find(' '));
                if (ins == "LIST")
                {
                    ConnList();
                }
                if (ins == "PRINT")
                {
                    int spccount = 0;
                    char message [100];
                    for (int i = 0; i < str.size(); ++i)
                    if (str[i] == ' ')
                    {
                            spccount = spccount + 1;
                    }
                    if (spccount == 1)
                    {
                        msg = str.substr(str.find(' ') + 1);
                        strcpy(message, msg.c_str());
                        for (int l = 0; l < conncount; l++)
                        {
                                //cout << c[l].messagesock;
                            write(c[l].messagesock, message, sizeof(msg));
                            // perror("WRITE");
                        }
                    }
                    else if (spccount == 2)
                    {
                        msg = str.substr(str.find(' ') + 1);
                        string ip = msg.substr(str.find(' ') + 1);
                        int addr = stoi(ip);
                        strcpy(message, msg.c_str());
                    }
                 }
                else if (ins == "HELP")
                {
                    Help2();
                }
            }
        }

    }



	/*
	 * Since this program has an infinite loop, the socket "sock" is
	 * never explicitly closed.  However, all sockets will be closed
	 * automatically when a process is killed or terminates normally.
	 */
