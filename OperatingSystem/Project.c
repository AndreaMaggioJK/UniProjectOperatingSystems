//////SaLVA	
#define POP_SIZE 8
#define sim_time 5
#define max_reject 2
#define nof_invites 5
#define MSG_LEN 200
//

#include "sem_utils.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <fcntl.h>
int esc;

int preferenze(int p2, int p3, int p4){
	int a[100] = {0};
	int i=0;
	for(i;i<p2;i++){a[i]=2;}		//0-->19
	int j=i;				//j=p2(20)
	for(j;j<(p2+p3);j++){a[j]=3;}		//20-->49
	int k=j;				//k=p2+p3(50)
	for(k;k<(p2+p3+p4);k++){a[k]=4;}	//50-->99
	int n=0;
	int rnd=rand()%100;
	if(a[rnd]==2)
		return 2;
	else if(a[rnd]==3)
		return 3;
	else if(a[rnd]==4)
		return 4;
}

void alarmHandler(int sig ) {
	kill(0,SIGUSR1);
}

void sigurs1Handler(int sig){
	esc =1;
}

struct msgbuf {
	long mtype;             /* message type, must be > 0 */
	int testo;    /* message data */
};

struct student{
	int matricola[POP_SIZE];
	int voto_AdE[POP_SIZE];
	int voto_SO[POP_SIZE];
	int nof_elements[POP_SIZE];
	int turno[POP_SIZE];
	int ruolo[POP_SIZE];
	int rifiuti[POP_SIZE];
	int inviti[POP_SIZE];
	int nAdepti[POP_SIZE];
	int Adepti[POP_SIZE][3];
};

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

int main(){
    union semun arg;
	pid_t ufficio_studenti;
	//shared memory
	int shid;
  	struct student *shared_data;

  	struct msgbuf messaggio;	//code messaggi
	struct msqid_ds mq_info;
	int codaPr = msgget(IPC_PRIVATE, IPC_CREAT|0600);

  	struct msgbuf messaggioV;	//code messaggi
	struct msqid_ds mq_infoV;
	int voto_key = 	msgget(IPC_PRIVATE, IPC_CREAT|0600);

  	if(signal(SIGUSR1, sigurs1Handler)==SIG_ERR){
    printf("\nErrore della disposizione dell'handler\n");
    exit(EXIT_SUCCESS);
  	}  	

  	if (signal(SIGALRM, alarmHandler)==SIG_ERR){
    printf("\nErrore della disposizione dell'handler\n");
    exit(EXIT_SUCCESS);
  	}

  	shid = shmget(IPC_PRIVATE, sizeof(struct student)*POP_SIZE, S_IRUSR | S_IWUSR);
  	if (shid == -1) {
    	fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    	exit(EXIT_FAILURE);
  	}

  	printf("creata e inizializzata l'area di memoria condivisa con ID = %d\n ", shid);
  	printf("e di grandezza %d\n\n ", (int)sizeof(struct student)*POP_SIZE );


	//semafori
	int semZero = semget(IPC_PRIVATE,POP_SIZE+1,IPC_CREAT|IPC_EXCL|S_IRUSR | S_IWUSR);
	if (semZero == -1) {
      fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    arg.val = POP_SIZE;

    if (semctl(semZero, POP_SIZE, SETVAL, arg) == -1) {
      fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    //semFinale
    union semun arg1;
    int semFinale = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|S_IRUSR | S_IWUSR);
	if (semFinale == -1) {
      fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    arg1.val = 1;

    if (semctl(semFinale, 0, SETVAL, arg1) == -1) {
      fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    //semFinale
    union semun arg2;
    int semEndMsg = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|S_IRUSR | S_IWUSR);
	if (semEndMsg == -1) {
      fprintf(stderr, "%s: %d. Errore in semget #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    arg2.val = POP_SIZE;

    if (semctl(semEndMsg, 0, SETVAL, arg2) == -1) {
      fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    size_t data_size;
    int status;
    int i;
    
	for(i = 0;i < POP_SIZE;i++){
	
		switch(ufficio_studenti = fork()){
			
			case-1:

			//	Errore
		    	fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
		    break;
			case 0:
				//FASE DI INIZIALIZZAZIONE
				srand(time(NULL)+getpid());
				// me accollo alla SM

				shared_data = shmat(shid, NULL, 0);
				
				//aggiorna questi valori nella struttura

				shared_data->matricola[i] = getpid();
				shared_data->turno[i] = shared_data->matricola[i]%2;
				shared_data->voto_AdE[i] = rand()%13+18;
				shared_data->voto_SO[i] = 0;
				shared_data->nof_elements[i] = preferenze(P2,P3,P4);
				shared_data->ruolo[i] = -1;
				shared_data->rifiuti[i] = 0;
				shared_data->inviti[i] =0;
				shared_data->nAdepti[i] = 0;
				int k = 0;
				esc = 0;
				while(k < 3){
					shared_data->Adepti[i][k] = -1;;
					k++;
				}
  				arg.val = 0;
    			if (semctl(semZero, i, SETVAL, arg) == -1) {
      				fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
     				exit(EXIT_FAILURE);
    			}

				reserveSem(semZero,POP_SIZE);
				waitZero_sem(semZero,POP_SIZE);
				//printf("sono il processo %d e sono stato risvegliato\n\n",getpid() );
				//GESTIONE INVITI

		while(shared_data->ruolo[i] == -1 && esc == 0 ){


				do {
				
				data_size =	msgrcv(codaPr, &messaggio, MSG_LEN,shared_data->matricola[i],IPC_NOWAIT);
				msgctl(codaPr, IPC_STAT, &mq_info);

				if (data_size!=-1){
				//printf("sono %d\n",i);
				//printf("sono %dLetto il messaggio: %s\n",i,messaggio.testo);
				//printf("Tipo messaggio: %ld\n",messaggio.mtype);
				int matricola_invito= messaggio.testo;
				//printf("messaggio covertito in int: %d\n",matricola_invito);
				if( shared_data->voto_AdE[matricola_invito] > shared_data->voto_AdE[i] || 
					shared_data->nof_elements[matricola_invito] == shared_data->nof_elements[i] ||
					shared_data->rifiuti[i] == max_reject || 
					shared_data->nAdepti[matricola_invito] > 0)
				{
					// accetto
					//printf("accetto %d <-%d \n",i,matricola_invito );
					if(esc == 0){
						shared_data->ruolo[i] = 0;
						shared_data->ruolo[matricola_invito] = 1;	
					}
					if(esc == 0){
					shared_data->Adepti[matricola_invito][ shared_data->nAdepti[matricola_invito] ] = i;
					shared_data->nAdepti[matricola_invito] = shared_data->nAdepti[matricola_invito]+1; 
					}


					reserveSem(semZero,matricola_invito);
  					// printf("Spedito il messaggio di accettazione %s\n\n",messaggio.testo);

				}else{
					// rifiuto
					shared_data->rifiuti[i] = shared_data->rifiuti[i]+1;
					//printf("rifiuto %d <-%d \n",i,matricola_invito );
					
					reserveSem(semZero,matricola_invito);
  					//printf("Spedito il messaggio di rifiuto\n\n");

				}

		  		}

				} while (data_size!=-1 && shared_data->ruolo[i] == -1 && esc == 0);
				releaseSem(semZero,i);
				releaseSem(semZero,i);
				//INVIO RICHIESTA




  			//	printf("messaggi nella coda matricola %d finiti \n\n",i);


			int matricola_da_invitare;

			do{
				matricola_da_invitare = rand()%POP_SIZE;				
			}while(esc == 0 && (matricola_da_invitare == i || matricola_da_invitare%2 != shared_data->turno[i]));

  			if(	shared_data->inviti[i] <= nof_invites && 
  				shared_data->ruolo[i] == -1 &&
  			 	shared_data->ruolo[matricola_da_invitare] == -1&& 
  			 	matricola_da_invitare != i &&
  			 	esc == 0 && 
  			 	semctl(semZero, matricola_da_invitare, GETVAL) != 2 ){

				shared_data->inviti[i] = shared_data->inviti[i]+1;  				
				//printf("%d  = matricola invitata da %d \n",matricola_da_invitare,i );
  				
				messaggio.mtype = shared_data->matricola[matricola_da_invitare];
				messaggio.testo = i;
				//matricola da inviare da randomizzare
				if (msgsnd(codaPr, &messaggio, MSG_LEN, 0)<0) {
    				fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    				exit(EXIT_FAILURE);
  				}
  				msgctl(codaPr,IPC_STAT,&mq_info);
  				//printf("Spedito il messaggio\n\n");
				reserveSem(semZero,i);
		while(semctl(semZero, i, GETVAL) != 0 && esc == 0){
					////////////////////////////////////////////////////////////////////////
			do {
				data_size =	msgrcv(codaPr, &messaggio, MSG_LEN,shared_data->matricola[i],IPC_NOWAIT);
				msgctl(codaPr, IPC_STAT, &mq_info);

				if (data_size!=-1){
					int matricola_invito= messaggio.testo;
					//printf("Rifiuto %d <-%d \n",i,matricola_invito );
					reserveSem(semZero,matricola_invito);
  					//printf("Spedito il messaggio di rifiuto\n\n");
  				}
  			}while(data_size!=-1);
  		}
						
				//printf("sbloccato %d\n",i);
  				///



  			}else{reserveSem(semZero,i);reserveSem(semZero,i);}
				/////
  			
  			

  			//printf("fine %d\n",i);
  		}
  		while(shared_data->ruolo[i] == 0 && esc == 0){

			do {
				data_size =	msgrcv(codaPr, &messaggio, MSG_LEN,shared_data->matricola[i],IPC_NOWAIT);
				msgctl(codaPr, IPC_STAT, &mq_info);

				if (data_size!=-1){
					int matricola_invito= messaggio.testo;
					//printf("///rifiuto %d <-%d \n",i,matricola_invito );
					reserveSem(semZero,matricola_invito);
  					//printf("Spedito il messaggio di rifiuto\n\n");
  				}
  			}while(data_size!=-1);


  		}

		while(shared_data->ruolo[i] == 1 && esc == 0 ){
/////////////////////////////////////////////////////
			if(shared_data->nAdepti[i] != shared_data->nof_elements[i]-1){

				releaseSem(semZero,i);
				releaseSem(semZero,i);
				//INVIO RICHIESTA




  			//	printf("messaggi nella coda matricola %d finiti \n\n",i);


			int matricola_da_invitare;

			do{
				matricola_da_invitare = rand()%POP_SIZE;				
			}while(esc == 0 && (matricola_da_invitare == i || matricola_da_invitare%2 != shared_data->turno[i]));

  			if(shared_data->inviti[i] <= nof_invites  && 
  				shared_data->ruolo[matricola_da_invitare] == -1 && 
  				matricola_da_invitare != i && 
  				esc == 0 && 
  				semctl(semZero, matricola_da_invitare, GETVAL) != 2 ){

				shared_data->inviti[i] = shared_data->inviti[i]+1;  				
				//printf("%d  = MATRICOLA INVIATA DA %d \n",matricola_da_invitare,i );
  				
				messaggio.mtype = shared_data->matricola[matricola_da_invitare];
				messaggio.testo = i;
				//matricola da inviare da randomizzare

				if (msgsnd(codaPr, &messaggio, MSG_LEN, 0)<0) {
    				fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    				exit(EXIT_FAILURE);
  				}
  				msgctl(codaPr,IPC_STAT,&mq_info);
  				//printf("Spedito il messaggio\n\n");
				reserveSem(semZero,i);
		while(semctl(semZero, i, GETVAL) != 0 && esc == 0){
			do {
				data_size =	msgrcv(codaPr, &messaggio, MSG_LEN,shared_data->matricola[i],IPC_NOWAIT);
				msgctl(codaPr, IPC_STAT, &mq_info);;

				if (data_size!=-1){
					int matricola_invito= messaggio.testo;
					//printf("Rifiuto %d <-%d \n",i,matricola_invito );
					reserveSem(semZero,matricola_invito);
  					//printf("Spedito il messaggio di rifiuto\n\n");
  				}
  			}while(data_size!=-1);
  		}
						
				//printf("sbloccato %d\n",i);
  				///



  			}else{reserveSem(semZero,i);reserveSem(semZero,i);}




  		}
///////////////////////////////////////////////////
  		}
  				//printf("FINE%d\n",i);
  				reserveSem(semEndMsg,0);
  				//printf("Prima%d\n",semctl(semFinale, 0, GETVAL));
  				waitZero_sem(semFinale,0);
  				//printf("DOPO%d\n",semctl(semFinale, 0, GETVAL));

				msgrcv(voto_key, &messaggioV, MSG_LEN,shared_data->matricola[i],IPC_NOWAIT);
				msgctl(voto_key, IPC_STAT, &mq_infoV);
  				printf("Matricola %d , voto SO = %d\n",shared_data->matricola[i],messaggioV.testo );


				if (shmdt(shared_data) == -1) {
    			fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    			exit(EXIT_FAILURE);
  				}


				exit(i);


				//....
			break;
			default:
			if(i == POP_SIZE-1){

  				if(signal(SIGUSR1, SIG_IGN)==SIG_ERR){
    				printf("\nErrore della disposizione dell'handler\n");
    				exit(EXIT_SUCCESS);
  				}  	

				waitZero_sem(semZero,POP_SIZE);
				printf("gli studenti hanno inizializzato i valori\n");

				shared_data = shmat(shid, NULL, 0);
				alarm(sim_time);

				//////////////////////////////////////////////////////
				waitZero_sem(semEndMsg,0);
				printf("CALCOLO VOTI\n");
				int k = 0;
				for(k = 0; k < POP_SIZE;k++){

					if(shared_data->ruolo[k] == 1){


						int cont = 0;
						int Max = shared_data->voto_AdE[k];
						while(cont < shared_data->nAdepti[k]){
							int Adp = shared_data->Adepti[k][cont];
							if(Max < shared_data->voto_AdE[ Adp ]){
								Max = shared_data->voto_AdE[Adp];					
							}
							cont++;
						}
						//printf("Max: %d\n",Max);
						cont = 0;
						if(shared_data->nof_elements[k]-1 == shared_data->nAdepti[k]){
							shared_data->voto_SO[k] = Max;

						}else{ shared_data->voto_SO[k] = Max-3; }
						while(cont < shared_data->nAdepti[k]){
							int Adp = shared_data->Adepti[k][cont];
							if(shared_data->nof_elements[Adp]-1 == shared_data->nAdepti[k]){
								shared_data->voto_SO[Adp] = Max;

							}else{ shared_data->voto_SO[Adp] = Max-3; }
							cont++;
						}


					}
				}
				for(k=0;k<POP_SIZE ;k++){
					messaggioV.mtype = shared_data->matricola[k];
		  			messaggioV.testo = shared_data->voto_SO[k];
					msgsnd(voto_key, &messaggioV, MSG_LEN, 0);
					msgctl(voto_key, IPC_STAT, &mq_infoV);
				}
				printf("UfficioVotiChiuso\n");
				/////////////////////////////////////////////////////
				reserveSem(semFinale,0);

				while(ufficio_studenti = wait(&status) != -1){}
				printf("gli studenti sono terminati\n");


				printf("Numero degli studenti per voto di Architettura = %d\n", POP_SIZE);
				int media_AdE = 0;
				for(int j = 0;POP_SIZE >j ;j++){
					media_AdE = media_AdE + shared_data->voto_AdE[j];
				}
				printf("Media per voto di Architettura = %d\n", media_AdE/POP_SIZE);

				printf("Numero degli studenti per voto di Sistemi operativi = %d\n", POP_SIZE);
				int media_SO = 0;
				for(int j = 0;POP_SIZE >j ;j++){
					media_SO = media_SO + shared_data->voto_SO[j];
				}
				printf("Media per voto di Sistemi operativi = %d\n", media_SO/POP_SIZE);




				for(int j = 0;POP_SIZE >j ;j++){
				//printf("matricola[%d] =%d \n", j,shared_data->matricola[j]);
				//printf("turno[%d] =%d \n", j,shared_data->turno[j]);
				//printf("voto_AdE[%d] =%d \n", j,shared_data->voto_AdE[j]);
				printf("voto_SO[%d] =%d \n", shared_data->matricola[j],shared_data->voto_SO[j]);
				//printf("nof_elements[%d] =%d \n", j,shared_data->nof_elements[j]);
				//printf("ruolo[%d] =%d \n", j,shared_data->ruolo[j]);
				//printf("key_msg[%d] =%d \n",j,shared_data->key_msg[j]);
				//printf("rifiuti[%d] =%d \n",j,shared_data->rifiuti[j]);
				//printf("inviti[%d] =%d \n", j,shared_data->inviti[j]);
				//printf("nAdepti[%d] =%d \n",j,shared_data->nAdepti[j] );
				//printf("Adepti [%d]=",j );
				/*int k = 0;
				while(k < 3){
					printf("%d ", shared_data->Adepti[j][k]);
					k++;
				}*/
				//printf("\n");


				}

				if (msgctl(codaPr, IPC_RMID, NULL)<0) {
    				fprintf(stderr, "%s: %d. Errore in msgctl (IPC_RMID) #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    				exit(EXIT_FAILURE);
  				}

				if (shmdt(shared_data) == -1) {
    			fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    			exit(EXIT_FAILURE);
  				}

				// dealloco shared memory
				if (shmctl(shid, 0, IPC_RMID) == -1) {
    				fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
    				exit(EXIT_FAILURE);
  				}
 				printf("Rimossa l'area di memoria condivisa con ID = %d \n", shid);
 								// dealloco semaforo
				if (semctl(semZero, POP_SIZE, IPC_RMID) == -1) {
      				fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
 					exit(EXIT_FAILURE);
				}

 				printf("Rimosso semaforo con ID = %d \n", semZero);

				if (semctl(semFinale, POP_SIZE, IPC_RMID) == -1) {
      				fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
 					exit(EXIT_FAILURE);
				}

 				printf("Rimosso semaforo con ID = %d \n", semFinale);

				if (semctl(semEndMsg, POP_SIZE, IPC_RMID) == -1) {
      				fprintf(stderr, "%s: %d. Errore in semctl #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
 					exit(EXIT_FAILURE);
				}

 				printf("Rimosso semaforo con ID = %d \n", semEndMsg);
			}


		}



    }

}