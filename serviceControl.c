/*
 *  Service Management
 *	Recruiting team DIANA
 *
 *	Author: Andrea Grillo
 *	October 2020
 *
 * commands:
 * 	start
 * 	stop
 * 	force-stop
 * 	status
 * 	help
 * 
 * uses files <service>.lock 
 * to save services' status
 * 	
 * 
 * Compile with:
 * 		cc serviceControl.c -o FILENAME
 * To get help write: ./FILENAME SERVICE help
 * 
 * The program has been tested on Debian 9.
 * 
 * To run the program the services executables
 * must be in the PATH directory.
 * Modify the PATH constant if needed.
 * 
 * To print debug message define constant DEBUGGING.
 */
 
 //macro to print useful information.
//#define DEBUGGING
#ifdef DEBUGGING 
#define DEBUG(arg ...) fprintf(stderr,arg);
#else 
#define DEBUG(arg ...) {}
#endif
 
 //includes
 #include <stdio.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <sys/types.h>
 #include <sys/wait.h>

 // constants
 #define SERVICEFILE	"services.txt"
 #define PATH 			"/bin/"
 
 // service states constants
 #define STARTED	0
 #define STOPPED	1
 #define STARTING	2
 #define STOPPING	3
 #define FAILED		4
 
 typedef struct {
	 char * nome;
	 int pid;
	 int stato;
 } service;
  
// function prototypes
 bool serviceExists();
 
 void checkStatus();
 
 void checkRealStatus();
	
 void writeStatus();
 
 void startService();
 
 void stopService();
 
 void forceStopService();
 
 //global variable servizio
 service servizio;
 
 //main
 int main(int argc, char *argv[])
 {
		DEBUG("start main\n");
		
		// there must be 2 parameters, service name and command
		if( argc != 3 )
		{
			fprintf(stderr,"ERROR: numbers of parameters not valid.\n");
			exit(-1);
		}	
		
		servizio.nome = argv[1];
		
		char * command = argv[2];
		
		DEBUG("	servizio: %s\n	comando: %s\n",servizio.nome,command);
		
		
		if(!serviceExists(servizio) && strcmp(command,"help")!=0) // if command==help don't mind about service existence
		{
			fprintf(stderr,"ERROR: Service not found in services.txt\n");
			exit(-1);
		}	
		
		DEBUG("after serviceExists control\n");
		
		// status check in lock file
		checkStatus();
		
		DEBUG("status checked\n	nome: -%s-\n	pid: %d\n	stato: %d\n",servizio.nome,servizio.pid,servizio.stato);
		
		
		// check of STARTING and STOPPING statuses
		if(servizio.stato == STARTING)
		{
			checkRealStatus();	//check on process PID
				
			if(servizio.stato != STARTED)	// process did not start -> FAILED
			{
				servizio.stato = FAILED;
			}
		}
		else if(servizio.stato == STOPPING)
		{
			checkRealStatus();	//check on process PID
			
			if(servizio.stato != STOPPED)	// process still exists, it hasn't stopped yet
			{
				servizio.stato = STOPPING;
			}
		}
		
		
		// command parsing
		if(strcmp(command,"start")==0)
		{
			DEBUG("start\n");
			if(servizio.stato == STARTED)
			{
				printf("Service already running.\n");
			}
			else if(servizio.stato == STARTING)
			{
				printf("Service already starting.\n");
			} else if(servizio.stato == STOPPING)
			{
				printf("Service stopping, retry.\n");
			} else
			{
				startService();
				DEBUG("start Service ended\n");
			}
		} 
		else if(strcmp(command,"stop")==0)
		{
			DEBUG("stop\n");
			if(servizio.stato == STOPPED)
			{
				printf("Service already stopped.\n");
			} else if(servizio.stato == STOPPING)
			{
				printf("Service already stopping.\n");
			} /*else if(servizio.stato == FAILED)
			{											if it failed, use STOP command and it can be started again
				printf("Il servizio è fallito");
			} */else if (servizio.stato == STARTING)
			{
				printf("Service starting, retry.\n");
			} else
			{
				stopService();
			}
			
		} else if(strcmp(command,"force-stop")==0)
		{
			DEBUG("force-stop\n");
			if(servizio.stato == STOPPED)
			{
				printf("Service already stopped.\n");
			} else if(servizio.stato == STOPPING)
			{
				printf("Service already stopping.\n");
			} /*else if(servizio.stato == FAILED)
			{											if it failed, use STOP command and it can be started again
				printf("Il servizio è fallito");
			} */else if (servizio.stato == STARTING)
			{
				printf("Service starting, retry.\n");
			} else
			{
				forceStopService();
			}
			
		} else if(strcmp(command,"status")==0)
		{
			DEBUG("status\n");
			printf("Service status is ");
			switch(servizio.stato)
			{
				case 0:
					printf("started.\n");
					break;
				case 1:
					printf("stopped.\n");
					break;
				case 2:
					printf("starting.\n");
					break;
				case 3:
					printf("stopping.\n");
					break;
				case 4:
					printf("failed.\n");
					break;
				default:
					printf("ERROR: status undefined");
					break;
			}
			
		} else if(strcmp(command,"help")==0)
		{
			DEBUG("help\n");
			printf("usage: %s SERVICENAME COMMAND\n\ncommands:\n - start\n - stop\n - force-stop\n - status\n - help\n",argv[0]);
		} else
		{
			fprintf(stderr,"ERROR: command not recognized.\n");
			exit(-1);
		}
		
		DEBUG("before calling writeStatus\n");
		
		// write actual status on file
		writeStatus(servizio.nome,servizio.stato);
		
		return 0;
 }

 
 bool serviceExists()
 {
	// check if service is present in file SERVICEFILE
	
	DEBUG("inside serviceExists()\n");
	
	FILE *fdService;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	
	fdService = fopen(SERVICEFILE,"r");
	
	if(fdService == NULL)
	{
		perror("Error opening servicefile.");
		exit(-1);
	}

	DEBUG("serviceExists: file opened\n");

	while ((read = getline(&line, &len, fdService)) != -1) 
	{
		DEBUG("serviceExists: inside while\n");

		// if last character of line is a 'new line' it gets removed from string
		if(line[strlen(line)-1]=='\n') line[strlen(line)-1] = '\0';
		
		if(strcmp(line,servizio.nome)==0)
		{
			DEBUG("service esiste\n");
			fclose(fdService);
			return(true);
		}
			
	}
	
	fclose(fdService);
	return(false);
 }
 
void checkStatus()
{
	// check service status on lock file
	
	DEBUG("inside checkStatus\n");

	char nomeFile[strlen(servizio.nome)+6];
	
	// "servizio.nome".lock
	strcat(strcpy(nomeFile,servizio.nome),".lock");
		
	DEBUG("nome file: %s\n",nomeFile);
	
	if( access(nomeFile, F_OK ) != -1 ) 
	{
		DEBUG("checkStatus: file exists\n");
		
		FILE *fd;
		fd = fopen(nomeFile,"r");
		
		if(fd == NULL)
		{
			perror("Error in lock file opening.");
			exit(-1);
		}
		
		DEBUG("checkStatus: before fscanf\n");

		char nome[100];
		
		fscanf(fd,"%s %d %d",&nome, &servizio.pid, &servizio.stato);
		DEBUG("checkStatus: after fscanf - nome -%s-\n",nome);

		strcpy(servizio.nome,nome);
		DEBUG("checkStatus: after strcpy  - nome -%s-\n",servizio.nome);

		fclose(fd);		
	} else
	{
		servizio.stato = STOPPED;
	}
	DEBUG("end checkStatus\n");
	return;
}

void checkRealStatus()
{
	// check service status using its PID
	
	if (getpgid(servizio.pid) >= 0)
	{
		servizio.stato = STARTED;
	} else
	{
		servizio.stato = STOPPED;
	}
	
	return;
}

void writeStatus()
{
	// write service status on lock file
	
	DEBUG("inside writeStatus\n");
	
	char fileName[strlen(servizio.nome)+6];
	
	DEBUG("before concatenazione stringhe nome servizio: -%s-\n",servizio.nome);
	
	// "servizio.nome".lock
	strcat(strcpy(fileName,servizio.nome),".lock");
		
	DEBUG("nome file: %s\n",fileName);
	
	FILE * fd;
	fd = fopen(fileName,"w");
	
	if(fd == NULL)
	{
		perror("Error in lock file opening.");
		exit(-1);
	}
	
	fprintf(fd,"%s %d %d", servizio.nome, servizio.pid, servizio.stato);
	
	fclose(fd);

	return;
}
 
void startService()
{
	// start service
	
	DEBUG("inside startService\n");
	int pid = fork();
	if (pid == 0)
	{
		DEBUG("inside forked process, ready to start -%s-\n",servizio.nome);

		if(execl("/bin/sh",servizio.nome, (char *) NULL) == -1)
		{
				perror("exec failed");
				exit(-1);
		}
		DEBUG("should never reach there, after exec\n");
		return;
	} else
	{
		servizio.pid = pid;	
		servizio.stato = STARTING;
	}
	DEBUG("end startService\n");
	return;
}
 
void stopService()
{
	// send SIGTERM signal to the process
	
	kill(servizio.pid,SIGTERM); 
	servizio.stato = STOPPING;

	return;
}
 
 void forceStopService()
 {
	// send SIGKILL signal to the process
	
	 kill(servizio.pid,SIGKILL); 
	 servizio.stato = STOPPING;

	 return;
}
