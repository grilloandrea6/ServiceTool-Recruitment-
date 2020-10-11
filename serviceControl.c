/*
 *  gestione servizi
 *	reclutamento team DIANA
 *
 *	Autore: Andrea Grillo
 *	Ottobre 2020
 *
 * comandi:
 * 	start
 * 	stop
 * 	force-stop
 * 	status
 * 	help
 * 
 * utilizza i file <service>.lock 
 * per salvare lo stato dei servizi
 * 	
 */
 
 //macro per il debug di informazioni utili
#define DEBUGGING
#ifdef DEBUGGING 
#define DEBUG(arg ...) fprintf(stderr,arg);
#else 
#define DEBUG(arg ...) {}
#endif
 
 //include delle librerie
 #include <stdio.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <sys/types.h>
 #include <sys/wait.h>

 // costanti dei nomi dei file in uso dal programma
 #define SERVICEFILE	"services.txt"
 
 // costanti degli stati di un servizio
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
  
 //prototipi delle funzioni 
 bool serviceExists();
 
 void checkStatus();
 
 void checkRealStatus();
	
 void writeStatus();
 
 void startService();
 
 void stopService();
 
 void forceStopService();
 
 //struct globale servizio
 service servizio;
 
 //main
 int main(int argc, char *argv[])
 {
		DEBUG("start main\n");
		
		// gli argomenti devono essere due, nome del servizio e comando da eseguire
		if( argc != 3 )
		{
			fprintf(stderr,"ERRORE: numero di argomenti non valido.\n");
			exit(-1);
		}	
		
		//char * serviceName = argv[1];
		servizio.nome = argv[1];
		
		char * command = argv[2];
		
		DEBUG("	servizio: %s\n	comando: %s\n",servizio.nome,command);
		
		
		if(!serviceExists(servizio) && strcmp(command,"help")!=0)
		{
			fprintf(stderr,"ERRORE: il servizio non è presente nel file services.txt\n");
			exit(-1);
		}	
		
		DEBUG("after serviceExists control\n");
		
		// controllo status su file
		checkStatus();
		
		// controllo degli stati di STARTING e STOPPING
		if(servizio.stato == STARTING)
		{
			checkRealStatus();	//controllo sul pid del processo
				
			if(servizio.stato != STARTED)	// il processo non è partito, è fallito
			{
				servizio.stato = FAILED;
			}
		}
		else if(servizio.stato == STOPPING)
		{
			checkRealStatus();	//controllo sul pid del processo
			
			if(servizio.stato != STOPPED)	// il processo esiste ancora, non si è ancora fermato
			{
				servizio.stato = STOPPING;
			}
		}
		
		
		// parsing del comando da tastiera
		if(strcmp(command,"start")==0)
		{
			DEBUG("start\n");
			if(servizio.stato == STARTED)
			{
				printf("Servizio già in esecuzione");
			}
			else if(servizio.stato == STARTING)
			{
				printf("servizio già in fase di starting");
			} else if(servizio.stato == STOPPING)
			{
				printf("servizio in stato di stopping, riprova tra poco");
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
				printf("Servizio già fermato");
			} else if(servizio.stato == STOPPING)
			{
				printf("Servizio già in fase di stopping");
			} else if(servizio.stato == FAILED)
			{
				printf("Il servizio è fallito");
			} else if (servizio.stato == STARTING)
			{
				printf("il servizio è in stato di starting, riprova tra poco");
			} else
			{
				stopService();
			}
			
		} else if(strcmp(command,"force-stop")==0)
		{
			DEBUG("force-stop\n");
			if(servizio.stato == STOPPED)
			{
				printf("Servizio già stopped");
			} else
			{
				forceStopService(servizio.nome);
				servizio.stato = STOPPING;
			}if(servizio.stato == STOPPED)
			{
				printf("Servizio già fermato");
			} else if(servizio.stato == STOPPING)
			{
				printf("Servizio già in fase di stopping");
			} else if(servizio.stato == FAILED)
			{
				printf("Il servizio è fallito");
			} else if (servizio.stato == STARTING)
			{
				printf("il servizio è in stato di starting, riprova tra poco");
			} else
			{
				forceStopService();
			}
			
		} else if(strcmp(command,"status")==0)
		{
			DEBUG("status\n");
			printf("Lo stato del servizio é ");
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
					printf("ERRORE: status undefined");
					break;
			}
			
		} else if(strcmp(command,"help")==0)
		{
			DEBUG("help\n");
			printf("usage: %s SERVICENAME COMMAND\n\ncommands:\n - start\n - stop\n - force-stop\n - status\n - help\n",argv[0]);
		} else
		{
			fprintf(stderr,"ERRORE: comando non riconosciuto.\n");
			exit(-1);
		}
		
		DEBUG("before calling writeStatus\n");
		// scrivo lo stato attuale su file
		writeStatus(servizio.nome,servizio.stato);
		
		return 0;
 }

 
 bool serviceExists()
 {
	// controlla se serviceName è presente nel file SERVICEFILE
	
	DEBUG("inside serviceExists()\n");
	
	FILE *fdService;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	
	fdService = fopen(SERVICEFILE,"r");
	
	if(fdService == NULL)
	{
		perror("Errore apertura file servizi.");
		exit(-1);
	}

	DEBUG("serviceExists: file opened\n");

	while ((read = getline(&line, &len, fdService)) != -1) 
	{
		DEBUG("serviceExists: inside while\n");
		// se l'ultimo carattere della linea letta è un 'a capo' lo elimino dalla stringa
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
	DEBUG("inside checkStatus\n");

	// TODO read file file exists read status
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
			perror("Errore nell'apertura nel file lock");
			exit(-1);
		}
		
		DEBUG("checkStatus: before fscanf\n");

		//fscanf(fd,"%s %d %d",&servizio.nome, &servizio.pid, &servizio.stato);
		DEBUG("checkStatus: after fscanf\n");

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
	// scrivi status nel file
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
		perror("Errore apertura file lock.");
		exit(-1);
	}
	
	fprintf(fd,"%s %d %d", servizio.nome, servizio.pid, servizio.stato);
	
	fclose(fd);

	return;
}
 
void startService()
{
	// fai partire servizio
	DEBUG("inside startService\n");
	int pid = fork();
	if (pid == 0)
	{
		DEBUG("inside forked process, ready to start -%s-\n",servizio.nome);
		execl("/bin/sh", "sh", "-c", servizio.nome, (char *) NULL);
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
	// manda segnale SIGTERM al processo
	kill(servizio.pid,SIGTERM); 
	servizio.stato = STOPPING;

	return;
}
 
 void forceStopService()
 {
	 // manda segnale SIGKILL al processo
	 kill(servizio.pid,SIGKILL); 
	 servizio.stato = STOPPING;

	 return;
}
