#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		  1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			      4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;
pthread_t threadCorbeau;
pthread_t threadCroco;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  positioncage = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;

typedef struct
{
  int type;//le champ type contient le type du sprite.
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;
// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	ouvrirFenetreGraphique();
	afficherCage(1);
	afficherCage(2);
	afficherCage(3);
	afficherCage(4);
	initGrilleJeu();

	int evt;
		
	
  // --------------- Etape 2 + 5 armement des signaux -------

	struct sigaction sigAct;

	sigemptyset(&sigAct.sa_mask); 
  sigAct.sa_handler = HandlerSIGQUIT; 
  sigAct.sa_flags = 0;
  sigaction(SIGQUIT, &sigAct, NULL);


  struct sigaction sigA;

	sigemptyset(&sigA.sa_mask); 
  sigA.sa_handler = HandlerSIGALRM; 
  sigA.sa_flags = 0;
  sigaction(SIGALRM, &sigA, NULL);

  struct sigaction sig;

	sigemptyset(&sig.sa_mask); 
  sig.sa_handler = HandlerSIGUSR1;
  sig.sa_flags = 0;
  sigaction(SIGUSR1, &sig, NULL);

  struct sigaction si;

	sigemptyset(&si.sa_mask); 
  si.sa_handler = HandlerSIGINT;
  si.sa_flags = 0;
  sigaction(SIGINT, &si, NULL);

  struct sigaction si1;

	sigemptyset(&si.sa_mask); 
  si1.sa_handler = HandlerSIGUSR2;
  si1.sa_flags = 0;
  sigaction(SIGUSR2, &si1, NULL);

  struct sigaction si2;

	sigemptyset(&si.sa_mask); 
  si2.sa_handler = HandlerSIGHUP;
  si2.sa_flags = 0;
  sigaction(SIGHUP, &si2, NULL);

  struct sigaction si3;

	sigemptyset(&si3.sa_mask); 
  si3.sa_handler = HandlerSIGCHLD;
  si3.sa_flags = 0;
  sigaction(SIGCHLD, &si3, NULL);

	//afficherRireDK();

	// afficherCle(3);
	// afficherCroco(11, 2);
	// afficherCroco(17, 1);
	// afficherCroco(0, 3);
	// afficherCroco(12, 5);
	// afficherCroco(18, 4);

	// afficherDKJr(11, 9, 1);
	// afficherDKJr(6, 19, 7);
	// afficherDKJr(0, 0, 9);
	
	// afficherCorbeau(10, 2);
	// afficherCorbeau(16, 1);
	
	
	// effacerCarres(9, 10, 2, 1);

	// afficherEchec(1);
	// afficherScore(1999);

	//------------------------initialissation des mutex------------------------- 
	pthread_mutex_init(&mutexEvenement, NULL);
	pthread_mutex_init(&mutexGrilleJeu, NULL);
	pthread_mutex_init(&mutexDK,  NULL);
	pthread_mutex_init(&mutexScore,  NULL);
	pthread_cond_init(&condScore, NULL);
	pthread_key_create(&keySpec, DestructeurVS); 

	//------------------------------Creation des threads --------------------------------------------------------

	pthread_create(&threadCle, NULL, (void *(*) (void *))FctThreadCle, NULL); 
	
	pthread_create(&threadEvenements, NULL, (void *(*) (void *))FctThreadEvenements , NULL);  

	pthread_create(&threadDK, NULL, (void *(*) (void *))FctThreadDK , NULL); 

	pthread_create(&threadScore, NULL, (void *(*) (void *))FctThreadScore , NULL); 

	pthread_create(&threadEnnemis, NULL, (void *(*) (void *))FctThreadEnnemis , NULL);

	printf("\n affiche getpid thread principale  =%d",getpid());

	printf("\n affiche thread principale=%d ",pthread_self());

	//----------------------Masquage du SIGNAL du thread Principale--------------------------------------------

	  //------------- Etape2----------- 
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);

	sigaddset(&mask,SIGALRM);

	sigaddset(&mask,SIGUSR1);

	sigaddset(&mask,SIGINT);

	sigaddset(&mask,SIGUSR2);

	sigaddset(&mask,SIGHUP);

	sigaddset(&mask,SIGCHLD);

	sigprocmask(SIG_SETMASK, &mask, NULL);

	
	//-------------------------------------------------------------------------------------------------------------

 
//---------------------------Gestion des 3 vies Etape 2-------------------
	int vie = 0;
  int i=0;
  while (vie < 3) 
  {
    // Commencer le thread DKJr
    pthread_create(&threadDKJr, NULL, (void *(*) (void *))FctThreadDKJr, NULL);
    pthread_join(threadDKJr, NULL);
    for(i = 0; i < 4; i++) 
 		{
 			/// tue les eventuelle croco a la position en bas 
 			if(grilleJeu[1][i].type == CROCO && i > 0) 
 			{
        // suppprime les croco aux alentours
		  	pthread_kill(threadCroco, SIGUSR2);
		  }

 			if(grilleJeu[2][i].type == CORBEAU) 
 			{
        // supprime les corbeaux aus alentours
		  	pthread_kill(threadCorbeau, SIGUSR1);
		  }

		  /// tue les eventuelle croco a la position en bas 
 			if(grilleJeu[3][i].type == CROCO && i > 0) 
 			{
        //suppprime les croco aux alentours
		  	pthread_kill(threadCroco, SIGUSR2);
		  }
 		}

    afficherEchec(vie + 1); // Afficher l'echec pour la vie actuelle, en ajoutant 1 car on commence a partir de 0

    vie++;
  }
  pause();
}
void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}
////---------------------------------- ETAPE 1------------------------------------------------------------------------------------------------------------
void* FctThreadCle(void *)
{
	sigset_t mask;

	sigemptyset(&mask);

	sigaddset(&mask,SIGQUIT);

	sigaddset(&mask,SIGALRM);

	sigaddset(&mask,SIGUSR1);

	sigaddset(&mask,SIGINT);

	sigaddset(&mask,SIGUSR2);

	sigaddset(&mask,SIGHUP);

	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);
  printf("\n affiche thread cle=%d ",pthread_self());

	struct timespec temps;
	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;

	int indiceCle=1; 

	while(1)
	{
		//-------verification de l'indices de la cle 
		if(indiceCle==5)
		{
			indiceCle=4;
			int i=0;
			  for(i=1;i<4;i++) 
			  {
					effacerCarres(3, 12, 2, 3); // Cela permet de rafraichir la grille de jeu  a chaque iteration de la boucle.

				 	afficherCle(indiceCle);

				 	pthread_mutex_lock(&mutexGrilleJeu);

					if(indiceCle == 1) // la cle est assesible pour dkjr
					{
					 	setGrilleJeu(0, 1, CLE);
					}

					else
					{
						setGrilleJeu(0, 1, VIDE);
					}

					pthread_mutex_unlock(&mutexGrilleJeu);

				 	nanosleep(&temps, NULL);	

				  indiceCle--;

			  }

		}
		else
		{
			effacerCarres(3, 12, 2, 3); // Cela permet de rafraichir la grille de jeu  a chaque iteration de la boucle.

		 	afficherCle(indiceCle);

		 	pthread_mutex_lock(&mutexGrilleJeu);

			if(indiceCle == 1) // la cle est assesible pour dkjr
			{
			 	setGrilleJeu(0, 1, CLE);
			}

			else
			{
				setGrilleJeu(0, 1, VIDE);
			}

			pthread_mutex_unlock(&mutexGrilleJeu);

		 	nanosleep(&temps, NULL);	

		  indiceCle++;

		}
		
		

	}
 	pthread_exit(0); //fonction execute par le thread se termine se termine
}

////---------------------------------- ETAPE 2------------------------------------------------------------------------------------------------------------
void* FctThreadEvenements(void *)
{
	printf("\n affiche thread evenement=%d ",pthread_self());
	struct timespec temps;
	temps.tv_sec = 0;
	temps.tv_nsec = 100000000;

	sigset_t mask;

	sigemptyset(&mask);

	sigaddset(&mask,SIGQUIT);

	sigaddset(&mask,SIGALRM);

	sigaddset(&mask,SIGUSR1);

	sigaddset(&mask,SIGINT);

	sigaddset(&mask,SIGUSR2);

	sigaddset(&mask,SIGHUP);

	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);

	int evt;

	while(1)
  {
  	evt = lireEvenement(); //Attend que le joueur appuie  sur une touche ou clique x 
   	pthread_mutex_lock(&mutexEvenement); 

	    switch (evt)
	    {
				case SDL_QUIT:
					evenement = SDL_QUIT;
					printf("~~~~Pourquoi tu quitte~~~\n");
					exit(0);

				case SDLK_UP:
					printf("KEY_UP\n");
					evenement = SDLK_UP;
					pthread_kill(threadDKJr, SIGQUIT);

				break;
				case SDLK_DOWN:
					printf("KEY_DOWN\n");
					evenement = SDLK_DOWN;
					pthread_kill(threadDKJr, SIGQUIT);

				break;
				case SDLK_LEFT:
					printf("KEY_LEFT\n");
					evenement = SDLK_LEFT;
					pthread_kill(threadDKJr, SIGQUIT);

				break;
				case SDLK_RIGHT:
					printf("KEY_RIGHT\n");
					evenement = SDLK_RIGHT;
					pthread_kill(threadDKJr, SIGQUIT);

				break;
			}
			nanosleep(&temps,NULL);
			pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}



////---------------------------------- ETAPE 3--------------------------------------------------------------------------------------------------------
void* FctThreadDKJr(void* p)
{
	/// Structur pour collision-----------------
	struct timespec tempss;
	tempss.tv_sec = 1;
	tempss.tv_nsec = 500000000;
	sigset_t mask;

	sigemptyset(&mask);

	sigaddset(&mask,SIGALRM);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);


	printf("\n affiche thread dkjr =%d ",pthread_self());

	bool on = true; 

	pthread_mutex_lock(&mutexGrilleJeu);

	setGrilleJeu(3, 1, DKJR); 

  afficherDKJr(11, 9, 1); 

  etatDKJr = LIBRE_BAS; 

  positionDKJr = 1;

 pthread_mutex_unlock(&mutexGrilleJeu);
 while (on)
 {
	 	pause();
		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);
		switch (etatDKJr)
		{
			
			case LIBRE_BAS:
			
				switch (evenement)
			  {
					case SDLK_LEFT:
					
						if (positionDKJr >1)
					 	{
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr--;
							setGrilleJeu(3, positionDKJr, DKJR);
							afficherDKJr(11, (positionDKJr * 2) + 7, 
							((positionDKJr - 1) % 4) + 1);
					  }
			 			break;
					case SDLK_RIGHT:

						if (positionDKJr < 7)
					 	{
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr++;
							setGrilleJeu(3, positionDKJr, DKJR);
							afficherDKJr(11, (positionDKJr * 2) + 7, 
							((positionDKJr -1) % 4) + 1);
							
					  }
			 			break;
				  case SDLK_UP:

					 if (positionDKJr == 2 ||  positionDKJr == 3 || positionDKJr == 4 || positionDKJr == 6)
					 {
						 	if(grilleJeu[2][positionDKJr].type == CORBEAU)
						 	{
					 		// 	setGrilleJeu(3, positionDKJr, DKJR);
					 		// 	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					 		// 	setGrilleJeu(2, positionDKJr, DKJR);
							
								// afficherDKJr(10, (positionDKJr * 2) + 7, 8);
								// nanosleep(&tempss,NULL);

								// effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

								pthread_kill(threadCorbeau,SIGUSR1);
							 		
							 	pthread_mutex_unlock(&mutexGrilleJeu);
							 	on=false;

							 	/*
							 	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							 	setGrilleJeu(3, positionDKJr, DKJR);
								afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
								*/

					 		}
					 		else if(grilleJeu[3][positionDKJr].type == CROCO)
					 		{
				 			
				 				pthread_kill(threadCorbeau,SIGUSR1);
						 		
						 		pthread_mutex_unlock(&mutexGrilleJeu);
						 		on=false;

					 			

					 		}
					 		else///---------------------------Etape 3----------------
					 		{
					 			struct timespec temps;
								temps.tv_sec = 1;
								temps.tv_nsec = 400000000;

					 			setGrilleJeu(3, positionDKJr);
					 			effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

					 			setGrilleJeu(2, positionDKJr, DKJR);
							
								afficherDKJr(10, (positionDKJr * 2) + 7, 8);
							 		
							 	pthread_mutex_unlock(&mutexGrilleJeu);
							 	nanosleep(&temps,NULL);
							 	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							 	setGrilleJeu(3, positionDKJr, DKJR);
								afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
								
					 		}
					 	}
					 else if(positionDKJr == 1 ||  positionDKJr == 5)
					 {

					 	setGrilleJeu(3, positionDKJr);
					 	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(2, positionDKJr, DKJR);
						afficherDKJr(10, (positionDKJr * 2) + 7, 7);
						 	
						etatDKJr = LIANE_BAS;

					 }
					 else if(positionDKJr == 7)
					 {
					 		setGrilleJeu(3,positionDKJr);
					 		effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

					 		setGrilleJeu(2, positionDKJr, DKJR);
						 	afficherDKJr(10, (positionDKJr * 2) + 7, 5);
						 	etatDKJr = DOUBLE_LIANE_BAS;
					 }
					 break;
				break;
			  }
		  case LIANE_BAS:
			  if(evenement==SDLK_DOWN)
			  {
          if(grilleJeu[3][positionDKJr].type ==CROCO)
          {
          	pthread_mutex_unlock(&mutexGrilleJeu);
			      pthread_mutex_unlock(&mutexEvenement);
            pthread_kill(threadCroco,SIGUSR2);
            on=false;
          }

          setGrilleJeu(2, positionDKJr);
					effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
					etatDKJr = LIBRE_BAS;
				}					 
				break;
		  case DOUBLE_LIANE_BAS:
			  if(evenement==SDLK_DOWN)
			  {
			  	if(positionDKJr == 7)
					{
	          if(grilleJeu[3][positionDKJr].type ==CROCO)
	          {
	          	pthread_mutex_unlock(&mutexGrilleJeu);
				      pthread_mutex_unlock(&mutexEvenement);
	            pthread_kill(threadCroco,SIGUSR2);
	            on=false;
	          }

	          setGrilleJeu(2, positionDKJr);
				    effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_BAS;
					}

			  }
			  else if (evenement=SDLK_UP)
			  {
			  	if(positionDKJr == 7)
					{
						setGrilleJeu(2, positionDKJr);
				    effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(8, (positionDKJr * 2) + 7, 6);
						etatDKJr = LIBRE_HAUT;
					}

			  }

					 
				break;
		  case LIBRE_HAUT:
			struct timespec temps;
			temps.tv_sec = 0;
			temps.tv_nsec = 500000000; 

				switch (evenement)
			  {
					case SDLK_LEFT:
					
						if (positionDKJr >3)
					 	{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr--;
							setGrilleJeu(1, positionDKJr, DKJR);
							int val = 0;
							switch(positionDKJr) 
							{
							  case 4:
							    val = 3;
							    break;
							  case 5:
							    val = 2;
							    break;
							  case 6:
							    val = 1;
							    break;
						    default:
							    val=4;
							}
							afficherDKJr(7, (positionDKJr * 2) + 7, val);
						}
			 			else if(positionDKJr==3)
			 			{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

							afficherDKJr(5, (positionDKJr * 2) + 7, 9);	//
							if(grilleJeu[0][1].type == CLE) 
							{

						 	  nanosleep(&temps, NULL);
					 			//efface jr en train de sauter pour la cle
							 	setGrilleJeu(0, positionDKJr);
							 	//effacerCarres(3, 12, 2, 4);////////
							 	effacerCarres(5, (2 * 2) + 7, 3, 3);// 12
							 	afficherCage(4);////

							 	//afficher jr avec le cle
							 	setGrilleJeu(0, positionDKJr, DKJR);
						 		afficherDKJr(3, 10, 10);/////

//--------------------etape4 Paradime de reveille-------------------
								pthread_mutex_lock(&mutexDK);

								MAJDK=true;
								pthread_mutex_unlock(&mutexDK);
								pthread_cond_signal(&condDK);
//--------------------etape4 Paradime de reveille-------------------


						 		nanosleep(&temps, NULL);

//---------------------------etape5 score Paradime reveille---------
						 		pthread_mutex_lock(&mutexScore);
						 		score=score+10;
						 		MAJScore=true;
						 		pthread_mutex_unlock(&mutexScore);
						 		pthread_cond_signal(&condScore);
//------------------------------------------------------------------


						 		// on efface 
						 		setGrilleJeu(0, positionDKJr);
						 		effacerCarres(3, 11, 3, 2);
						 		afficherCage(4);

						 		//afficher jr tombant 
					 			setGrilleJeu(1, positionDKJr, DKJR);
				 				afficherDKJr(7, (positionDKJr * 2) + 7, 11);
				 				afficherCage(4);//

				 				nanosleep(&temps, NULL);

				 				setGrilleJeu(1, positionDKJr);
				 				effacerCarres(6, 10, 2, 3);

						 		//afficher sur le sol
								setGrilleJeu(3, 1, DKJR); 
								afficherDKJr(11, 9, 1); 
								etatDKJr = LIBRE_BAS; 
								positionDKJr = 1;

							}
				 			else 
				 			{

			 					nanosleep(&temps, NULL);

			 					//efface jr en train de sauter pour le cle
					 			setGrilleJeu(0, positionDKJr);
					 			effacerCarres(5, (2 * 2) + 7, 3, 3);
					 			afficherCage(4);

					 			//afficher jr en train de tomber
					 			setGrilleJeu(0, positionDKJr, DKJR);
				 				afficherDKJr(6, (positionDKJr * 2) + 7, 12);
				 				nanosleep(&temps, NULL);

				 				//efface jr entreint de tomber
					 			setGrilleJeu(0, positionDKJr);
					 			effacerCarres(6, (2 * 2) + 7, 2, 2);

					 			//afficher jr dans les buissons
					 			setGrilleJeu(3, 1, DKJR);
				 				afficherDKJr(11, 8, 13);

				 				nanosleep(&temps, NULL);

				 				//efface jr dans les buissons
					 			setGrilleJeu(3, positionDKJr);
					 			effacerCarres(11, 7, 2, 2);
					 			
					 			on = false;

							}

			 			}
			 			break;
					case SDLK_RIGHT:
					int val;

						if (positionDKJr < 7)
					 	{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr++;
							setGrilleJeu(1, positionDKJr, DKJR);
							switch(positionDKJr) 
							{

							  case 4:
							    val = 3;
							    break;
							  case 5:
							    val = 2;
							    break;
							  case 6:
							    val = 1;
							    break;
							  default:
							    val = 6;
							}
							afficherDKJr(7, (positionDKJr * 2) + 7, val);
							
					  }
			 			break;
				  case SDLK_UP:
					 if (positionDKJr == 3)
					 {
						 	setGrilleJeu(1, positionDKJr);//correction 
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

							setGrilleJeu(0, positionDKJr, DKJR);
							afficherDKJr(6, (positionDKJr * 2) + 7, 8);
							struct timespec temps;
							temps.tv_sec = 1;
							temps.tv_nsec = 400000000; 
						 	nanosleep(&temps, NULL);	
						 	pthread_mutex_unlock(&mutexGrilleJeu);

						 	setGrilleJeu(0, positionDKJr);////correction 
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

						 	setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7, 4);
					 }
					 else if(positionDKJr == 4)
					 {
						 	setGrilleJeu(1, positionDKJr);//correction 
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

							setGrilleJeu(0, positionDKJr, DKJR);
							afficherDKJr(6, (positionDKJr * 2) + 7, 8);

							struct timespec temps;
							temps.tv_sec = 1;
							temps.tv_nsec = 400000000; 
						 	nanosleep(&temps, NULL);	
						 	pthread_mutex_unlock(&mutexGrilleJeu);

						 	setGrilleJeu(0, positionDKJr);////correction 
						 	effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

						 	setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7, 3);
					 }
					 else if(positionDKJr == 6)
					 {
					 		setGrilleJeu(1, positionDKJr);
					 		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

					 		setGrilleJeu(0, positionDKJr, DKJR);
						 	afficherDKJr(6, (positionDKJr * 2) + 7, 7);
						 	etatDKJr = LIANE_HAUT;
					 }
					 break;
					case SDLK_DOWN:
						if(positionDKJr == 7)
					  {
					 		setGrilleJeu(1, positionDKJr);
					 		effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					 		if(grilleJeu[1][positionDKJr].type ==CROCO)
			        {
		            pthread_mutex_unlock(&mutexGrilleJeu);
		          	pthread_mutex_unlock(&mutexEvenement);
		            pthread_kill(threadCroco,SIGUSR2);
		            on=false;

			        }


					 		setGrilleJeu(2, positionDKJr, DKJR);////
						 	afficherDKJr(10, (positionDKJr * 2) + 7, 5);
						 	etatDKJr = DOUBLE_LIANE_BAS;
					  }
						break;
				break;
			  }

					 
			break;
		  case LIANE_HAUT:
			  if(evenement==SDLK_DOWN)
			  {
			  	if(positionDKJr == 6)
					{
						if(grilleJeu[1][positionDKJr].type ==CROCO)
	          {
	          	pthread_mutex_unlock(&mutexGrilleJeu);
				      pthread_mutex_unlock(&mutexEvenement);
	            pthread_kill(threadCroco,SIGUSR2);
	            on=false;
	          }
						setGrilleJeu(0, positionDKJr);
						effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);

						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4));
						etatDKJr = LIBRE_HAUT;
					}

			  }				
		  	break;
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);
		afficherGrilleJeu();
	}
	pthread_exit(0);
}

void HandlerSIGQUIT (int sig) 
{  
	printf("\n je recois le signal SIGQUIT de thread =%d \n",pthread_self());

} 


//------------------------------------------------Etape4 -----------------------------------------------------------------------------------------------


void* FctThreadDK(void *p)
{
	printf("\n affiche thread dk=%d ",pthread_self());
	struct timespec temps;
	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGALRM);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);

	while(1)
	{
//~~~~~~~~~~~~~~Paradime attente ~~~~~~~~~~~~
		pthread_mutex_lock(&mutexDK);
		while(MAJDK==false)
		{

			pthread_cond_wait(&condDK,&mutexDK);


		}
		pthread_mutex_lock(&mutexGrilleJeu);
//-----------------------------------------------
		switch(positioncage)
		{
			case 1: 
			 effacerCarres(2, 7, 2, 2);
			 positioncage++;
			break;
			case 2:
			 effacerCarres(2, 9, 2 , 2);	
			 positioncage++;
			break;g
			case 3:
			 effacerCarres(4, 7, 2 , 2);	
			 positioncage++;
			break;
			case 4:
			 effacerCarres(4, 9, 2 , 3);	
			 positioncage++;
			break;
		}
		MAJDK=false;
		pthread_mutex_unlock(&mutexDK);
		pthread_mutex_unlock(&mutexGrilleJeu);

		if(positioncage>4)
		{
			pthread_mutex_lock(&mutexGrilleJeu);

			afficherRireDK();
//-------------etape 5 paradime de reveille ------------------
			pthread_mutex_lock(&mutexScore);
	 		score=score+10;
	 		MAJScore=true;
	 		pthread_mutex_unlock(&mutexScore);
	 		pthread_cond_signal(&condScore);

//------------------------------------------------------------
	 		nanosleep(&temps,NULL);
			effacerCarres(3 , 8, 2 , 2);

			afficherCage(1);
			afficherCage(2);
			afficherCage(3);
			afficherCage(4);

			positioncage=1;
			pthread_mutex_unlock(&mutexGrilleJeu);

		}


	}
	pthread_exit(0);

}
//------------------------------------------------Etape5 -----------------------------------------------------------------------------------------------

void* FctThreadScore(void *p)
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGALRM);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);

	printf("\n affiche thread score=%d ",pthread_self());
//-------------------Paradime d'attente ----------------------
	while(1)
	{
		pthread_mutex_lock(&mutexScore);

		while(MAJScore==false)
		{
			pthread_cond_wait(&condScore,&mutexScore);

		}

		MAJScore=false;
		pthread_mutex_unlock(&mutexScore);

		afficherScore(score);
//-------------------Paradime d'attente ----------------------

	}
	pthread_exit(0);

}
//------------------------------------------------Etape6 -----------------------------------------------------------------------------------------------
void* FctThreadEnnemis(void *p)
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);

	printf("\n affiche thread enenie=%d ",pthread_self());

  struct timespec temps;
	temps.tv_sec = delaiEnnemis/1000;
	temps.tv_nsec = 0;
  while(1)
  {


  	nanosleep(&temps,NULL);
		int aleatoire;
		aleatoire=rand()%2;
		if(aleatoire==1)
		{
			pthread_create(&threadCorbeau, NULL, (void *(*) (void *))FctThreadCorbeau , NULL);

			
		}
		else
		{
			pthread_create(&threadCroco, NULL, (void *(*) (void *))FctThreadCroco , NULL);

			
		}
		
	}
	pthread_exit(0);
	

}
void HandlerSIGALRM(int sig) 
{
	printf("\n je recois le signal alarm de thread =%d \n",pthread_self());


	if(delaiEnnemis >(2.5 * 1000))  
	{
		delaiEnnemis = delaiEnnemis- (0.25 * 1000); 
		alarm(15);
	}
}
//------------------------------------------------Etape7-----------------------------------------------------------------------------------------------
void * FctThreadCorbeau (void *p)
{

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGALRM);
	sigaddset(&mask,SIGUSR2);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);

	printf("\n affiche thread CORBEAU=%d ",pthread_self());
	struct timespec temps;
	temps.tv_sec=0;
	temps.tv_nsec= 700000000;
	int * positionCorbeau;

	printf("\n!!!CORBEAU creer ahaha!!!!!\n");

	
	positionCorbeau = (int *)malloc(sizeof(int)); /// on alloue pour chaque corbeau 
	

	(*positionCorbeau)=0;

 
	

	while((*positionCorbeau)<8)
	{
		if(grilleJeu[2][*positionCorbeau].type == DKJR) 
		{
  		pthread_kill(threadDKJr, SIGINT);

  		pthread_exit(0);
  	}  
  	else
		{
			if(((*positionCorbeau)% 2)==0)
			{

				pthread_mutex_lock(&mutexGrilleJeu);

				setGrilleJeu(2, (*positionCorbeau), CORBEAU);
				afficherCorbeau(((*positionCorbeau) * 2 + 8), 2);

				pthread_mutex_unlock(&mutexGrilleJeu);

				pthread_setspecific(keySpec,positionCorbeau);

				nanosleep(&temps,NULL);
		 }
		 else
		 {
		 		pthread_mutex_lock(&mutexGrilleJeu);
				setGrilleJeu(2, (*positionCorbeau), CORBEAU);
				afficherCorbeau(((*positionCorbeau) * 2 + 8), 1);
				pthread_mutex_unlock(&mutexGrilleJeu);

				pthread_setspecific(keySpec,positionCorbeau);

				nanosleep(&temps,NULL);
		  }
		}

		pthread_mutex_lock(&mutexGrilleJeu);

		setGrilleJeu(2, (*positionCorbeau));
		effacerCarres(9,((((*positionCorbeau)) * 2) + 8), 2 ,2);

		*positionCorbeau = (*positionCorbeau) + 1;



		pthread_mutex_unlock(&mutexGrilleJeu);

	}
	pthread_exit(0);
}
void DestructeurVS(void *p)
{
	int *ptr = (int *)p;
  free(ptr);
}
void HandlerSIGUSR1(int)
{
	printf("\nj'ai recue le signal sigusr1=%d",pthread_self());

    // Recuparation de la position horizontale du corbeau

    int *positionCorbeau = (int *)pthread_getspecific(keySpec);
    int positionCorbeauhori = *positionCorbeau;
    
    // Effacement du corbeau dans grilleJeu et Ã  l'Ã©cran
    pthread_mutex_lock(&mutexGrilleJeu);

    setGrilleJeu(2, positionCorbeauhori);
    effacerCarres(9, (positionCorbeauhori * 2) + 8, 2, 2);

    pthread_mutex_unlock(&mutexGrilleJeu);
    pthread_exit(&*positionCorbeau);
	
}
void HandlerSIGINT(int)
{
  printf("\n je recois le signal SIGINT de thread =%d \n",pthread_self());
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(2, positionDKJr);
  effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
  pthread_mutex_unlock(&mutexGrilleJeu);
  

  // si DK Jr est dans l'etat LIBRE_BAS, liberer mutexEvenement
  if (etatDKJr == LIBRE_BAS)
  {
    pthread_mutex_unlock(&mutexEvenement);
  }

  // terminer ThreadDKJr
  pthread_exit(0);
}

//------------------------------------------------Etape8 -----------------------------------------------------------------------------------------------
void * FctThreadCroco (void *p)
{
	struct timespec temps;
	temps.tv_sec=0;
	temps.tv_nsec= 700000000;

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGQUIT);
	sigaddset(&mask,SIGALRM);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGINT);
	sigaddset(&mask,SIGHUP);
	sigaddset(&mask,SIGCHLD);
	//----------------MISE en place du masquage --------------
	sigprocmask(SIG_SETMASK, &mask, NULL);



	printf("\n affiche thread croco =%d ",pthread_self());

	printf("\n!!!CROCO creer ahaha !!!!!\n");


	S_CROCO *positionCroco = (S_CROCO *)malloc(sizeof(S_CROCO)); /// on alloue pour chaque croco 

	//----------------------le Croco est en libre haut -----------------
	positionCroco->haut=true;
  positionCroco->position = 0;
  while(positionCroco->position <=5)
  {

   if(grilleJeu[1][positionCroco->position+2].type==DKJR)
   {

   	 pthread_kill(threadDKJr, SIGHUP);
   	 pthread_exit(0);
   }

   pthread_mutex_lock(&mutexGrilleJeu);
	 setGrilleJeu(1, positionCroco->position + 2, CROCO);
	 afficherCroco((positionCroco->position * 2) + 11, (positionCroco->position % 2 == 0) ? 1 : 2);
	 pthread_mutex_unlock(&mutexGrilleJeu);

	 pthread_setspecific(keySpec, positionCroco);

	 nanosleep(&temps, NULL);	

	 pthread_mutex_lock(&mutexGrilleJeu);
	 setGrilleJeu(1, positionCroco->position + 2);
	 effacerCarres(8, ((positionCroco->position) * 2) + 11, 1, 1);
	 pthread_mutex_unlock(&mutexGrilleJeu);
	 positionCroco->position= positionCroco->position + 1;

  }

  afficherCroco(23,3);// le croco qui tombe 
  nanosleep(&temps, NULL); 
  effacerCarres(9,23,1,1);// efface croco qui tombe

   //----------------------le Croco est en libre bas -----------------

  positionCroco->haut=false;

  while(positionCroco->position >=0  && positionCroco->position<=6)
  {
      
       
   if(grilleJeu[3][positionCroco->position+1].type==DKJR)
   {

	   	pthread_kill(threadDKJr, SIGCHLD);
	   	pthread_exit(&*positionCroco); 
   }
   pthread_mutex_lock(&mutexGrilleJeu);
	 setGrilleJeu(3, positionCroco->position + 1, CROCO);
	 afficherCroco((positionCroco->position * 2) + 10, (positionCroco->position % 2 == 0) ? 4 : 5);
	 pthread_mutex_unlock(&mutexGrilleJeu);

	 pthread_setspecific(keySpec, &*positionCroco);

	 nanosleep(&temps, NULL);


   pthread_mutex_lock(&mutexGrilleJeu);
	 setGrilleJeu(3, positionCroco->position + 1);
	 effacerCarres(12, ((positionCroco->position) * 2) + 10, 1, 1);
	 pthread_mutex_unlock(&mutexGrilleJeu);

   positionCroco->position= positionCroco->position - 1;
  }

	pthread_exit(0); 
}
void HandlerSIGUSR2(int sig) 
{
  printf("\n je recois le signal SIGUSR2 de thread =%d \n",pthread_self());
 	S_CROCO *positionCroco = (S_CROCO*)pthread_getspecific(keySpec);


  if(positionCroco->haut==true)
  {
    pthread_mutex_lock(&mutexGrilleJeu);
    setGrilleJeu(1, positionCroco->position + 2);
    effacerCarres(8, ((positionCroco->position) * 2) + 11, 1, 1);
    pthread_mutex_unlock(&mutexGrilleJeu);
  }
  
  else
  {
    pthread_mutex_lock(&mutexGrilleJeu);
    setGrilleJeu(3, positionCroco->position + 1);
    effacerCarres(12, ((positionCroco->position) * 2) + 10, 1, 1);
    pthread_mutex_unlock(&mutexGrilleJeu);
  }
  

	pthread_exit(0);
}


void HandlerSIGHUP(int sig) 
{
  printf("\n je recois le signal SIGHUP de thread =%d \n",pthread_self());
  pthread_mutex_lock(&mutexGrilleJeu);
  setGrilleJeu(1,positionDKJr);
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);

  pthread_mutex_unlock(&mutexGrilleJeu);

	pthread_exit(0);
}



void HandlerSIGCHLD(int sig) 
{
	printf("\n je recois le signal SIGCHLD de thread =%d \n",pthread_self());
  pthread_mutex_lock(&mutexGrilleJeu);
  setGrilleJeu(3, positionDKJr);
	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
  pthread_mutex_unlock(&mutexGrilleJeu);

	pthread_exit(0);
}




















