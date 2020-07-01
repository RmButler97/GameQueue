// Ryan Butler
// This project will simulate a game where players battle each other
// and a dragon will eat someone and some surviors will be transported
// to the next continent and players waiting in the queue will replace
// those killed in battle, who would be placed at the end of the queue
// February, 8, 2019

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct player
{
	int num;	// 1-30
	char type;	// 'A', 'H', ' '
	int cont;	// 0,1,2,3
};

struct player globalQ[30], Q[5][4];
int dloc;			// continent Dragon is on
int numP = 0;			// total number of human players
int done = 0;			// number of done threads
int doneT = 0;			// number of done threads
int gi = 20;
int taken;
int putback;
pthread_mutex_t mv = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t condM = PTHREAD_COND_INITIALIZER;

void *playGame(void *x)
{
	struct player tempQ[2][4];
	int me = *((int *) x);
	int start = me * 5;
	int end = start + 5;
	int j;
	int numA = 0;
	int numH = 0;
	int epoch;
	int tempDeadN;
	int tempDeadT;
	int tempEatenN;
	int tempEatenT;
	int index;


	// set up world
	j=0;
	for (int i=start; i<end; i++)
	{	
		Q[j][me].num = globalQ[i].num;
		Q[j][me].type = globalQ[i].type;
		Q[j][me].cont = me;
		j++;
	}


	// play game
	for (epoch=0; epoch<5; epoch++)
	{
		j = 0;
		gi = 20;
		index = gi;
		taken = 0;
		numA = 0;
		numH = 0;

		for (int i=0; i<5; i++)
		{
			if (Q[i][me].type == 'A')
				numA++;
			else if (Q[i][me].type == 'H')
				numH++;
		}
		
		if (numA > numH)
		{
			// delete 1st H in local queue if there are more As
			while (Q[j][me].type == 'A')
				j++;
			if (Q[j][me].type == 'H')
			{
				tempDeadN = Q[j][me].num;
				tempDeadT = Q[j][me].type;
				Q[j][me].num = -1;
				Q[j][me].type = ' ';
			}
		}	
		else if (numH > numA)
		{
			// delete 1st A in local queue if there are more Hs
			while (Q[j][me].type == 'H')
				j++;
			if (Q[j][me].type == 'A')
			{
				tempDeadN = Q[j][me].num;
				tempDeadT = Q[j][me].type;
				Q[j][me].num = -1;
				Q[j][me].type = ' ';
			}
		}

		// dragon eats. Search for A, if none found take first H
		if (dloc == me)
		{
			j = 0;
			while (Q[j][dloc].type != 'A' && j < 5)
			{
				j++;
			}
			if (Q[j][dloc].type == 'A')
			{
				tempEatenN = Q[j][dloc].num;
				tempEatenT = Q[j][dloc].type;
				Q[j][dloc].num = -1;
				Q[j][dloc].type = ' ';
			}
			else	// no A was found
			{
				// delete Q[0][dloc] if isn't killed
				if (Q[0][dloc].num != -1)
				{
					tempEatenN = Q[0][dloc].num;
					tempEatenT = Q[0][dloc].type;
					Q[0][dloc].num = -1;
					Q[0][dloc].type = ' ';
				}
				else
				{
					tempEatenN = Q[1][dloc].num;
					tempEatenT = Q[1][dloc].type;
					Q[1][dloc].num = -1;
					Q[1][dloc].type = ' ';
				}
			}
		}
	
		// remove dead bodies
		// move up and replace where dead were
		for (int i=0; i<5; i++)
		{
			for (j=0; j<4; j++)
			{
				if (Q[j][me].num == -1)
				{
					// move next j up
					Q[j][me].num = Q[j+1][me].num;
					Q[j][me].type = Q[j+1][me].type;
					Q[j+1][me].num = -1;
				}
			}
		}

		// transport player to next continent
		// store players moving in temp queue
		tempQ[0][me].num = Q[0][me].num;
		tempQ[0][me].type = Q[0][me].type;
		Q[0][me].num = -1;
		Q[0][me].type = ' ';

		tempQ[1][me].num = Q[1][me].num;
		tempQ[1][me].type = Q[1][me].type;
		Q[1][me].num = -1;
		Q[1][me].type = ' ';

		// move up
		for (j=0; j<4; j++)
		{
			if (Q[j][me].num == -1)
			{
				Q[j][me].num = Q[j+1][me].num;
				Q[j][me].type = Q[j+1][me].type;
				Q[j+1][me].num = -1;
			}
		}

		// wait
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		// transport players to next continent
		pthread_mutex_lock(&mv);
		if (me == 3)
		{
			Q[3][0].num = tempQ[0][me].num;
			Q[3][0].type = tempQ[0][me].type;
			Q[3][0].cont = 0;

			Q[4][0].num = tempQ[1][me].num;
			Q[4][0].type = tempQ[1][me].type;
			Q[4][0].cont = 0;
		}
		else
		{
			Q[3][me+1].num = tempQ[0][me].num;
			Q[3][me+1].type = tempQ[0][me].type;
			Q[3][me+1].cont = me+1;

			Q[4][me+1].num = tempQ[1][me].num;
			Q[4][me+1].type = tempQ[1][me].type;
			Q[4][me+1].cont = me+1;
		}
		pthread_mutex_unlock(&mv);

		// move up
		for (j=0; j<4; j++)
		{
			if (Q[j][me].num == -1)
			{
				Q[j][me].num = Q[j+1][me].num;
				Q[j][me].type = Q[j+1][me].type;
				Q[j+1][me].num = -1;
			}
		}

		//wait
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		// move up
		for (int i=0; i<5; i++)
		{
			for (j=0; j<4; j++)
			{
				if (Q[j][me].num == -1)
				{
					Q[j][me].num = Q[j+1][me].num;
					Q[j][me].type = Q[j+1][me].type;
					Q[j+1][me].num = -1;
				}
			}
		}

		// it's possible no one died
		// if dragon ate and player killed
		pthread_mutex_lock(&mv);
		for (int i=0; i<5; i++)
		{
			if (Q[i][me].num == -1)
			{
				Q[i][me].num = globalQ[gi].num;
				Q[i][me].type = globalQ[gi].type;
				Q[i][me].cont = me;
				gi++;
				taken++;
			}
		}
		pthread_mutex_unlock(&mv);

		// delete globalQ
		// wait for local queue to take from global queue
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		putback = numP-taken;

		// move up
		for (j=0; j<4; j++)
		{
			if (Q[j][me].num == -1)
			{
				Q[j][me].num = Q[j+1][me].num;
				Q[j][me].type = Q[j+1][me].type;
				Q[j+1][me].num = -1;
			}
		}
	
		// only one thread should delete from globalQ
		if (me == 0)
		{
			for (int i=0; i<taken; i++)
			{
				globalQ[index].num = globalQ[index+taken].num;
				globalQ[index].type = globalQ[index+taken].type;
				index++;
			}
			printf("\n");
		}

		// move up
		for (j=0; j<4; j++)
		{
			if (Q[j][me].num == -1)
			{
				Q[j][me].num = Q[j+1][me].num;
				Q[j][me].type = Q[j+1][me].type;
				Q[j+1][me].num = -1;
			}
		}

		// wait
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		// add killed to end of global queue
		// start at globalQ[numP-taken]
		pthread_mutex_lock(&mv);
		globalQ[putback].num = tempDeadN;
		globalQ[putback].type = tempDeadT;
		putback++;
		if (me == dloc)
		{
			globalQ[putback].num = tempEatenN;
			globalQ[putback].type = tempEatenT;
			putback++;
		}
		pthread_mutex_unlock(&mv);
	
		// wait
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		// transport dragon to prev continent
		if (me == 0)
		{
			if (dloc == 0)
				dloc = 3;
			else
				dloc--;
		}

		// wait for all threads to finish calculating
		pthread_mutex_lock(&mv);
		done++;
		if (done < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_broadcast(&cv);
			done = 0;
		}
		pthread_mutex_unlock(&mv);

		// update the globalQ
		j=0;
		for (int i=start; i<end; i++)
		{
			globalQ[i] = Q[j][me];
			j++;
		}

		// conditional variable to wait for all threads to finish before
		// moving to next epoch
	
		// wait for main thread to print epoch
		pthread_mutex_lock(&mv);
		doneT++;
		if (doneT < 4)
			pthread_cond_wait(&cv, &mv);
		else
		{
			pthread_cond_signal(&condM);
			pthread_cond_wait(&cv, &mv);
		}
		pthread_mutex_unlock(&mv);

	}
}


int main()
{
	pthread_t id[4];	// 4 threads for each continent
	FILE *fp;
	char c;			// what the file reads
	int i = 0;
	int is[4];
	int epoch;


	fp = fopen("players.dat", "r");
	
	// read file and create global queue
	while(!feof(fp))
	{
		fscanf(fp, "%c", &c);
		if (c=='A' || c=='H')
		{
			globalQ[i].num = i+1;
			globalQ[i].type = c;
			i++;
			numP++;
		}
		else if (c=='D')
		{
			// read next char for starting continent for Dragon
			fscanf(fp, "%d", &dloc);
		}
	}

	// print initial world
	printf("Initial Setup:\n\n");
	printf("Player\tType\tContinent\n");
	for (i=0; i<numP; i++)
	{
		if ((dloc * 5) + 5 == i)
			printf("Dragon\t \t%d\n", dloc);
		if (i<20)
			printf("%d\t%c\t%d\n", globalQ[i].num,
					globalQ[i].type, i/5);
		else
			printf("%d\t%c\tqueue\n", globalQ[i].num, globalQ[i].type);
	}
	printf("\n\n");

	// create 4 threads for each continent
	for (i=0; i<4; i++)
	{
		is[i] = i;
		pthread_create(&id[i], NULL, playGame,(void *)&is[i]);
	}

	// play for 5 epochs
	for(epoch=0; epoch<5; epoch++)
	{
		// wait for all threads to finish
		pthread_mutex_lock(&mv);
		while (doneT < 4)
			pthread_cond_wait(&condM, &mv);
		// print initial world
		printf("The World after Epoch %d:\n\n", epoch+1);
		printf("Player\tType\tContinent\n");
		for (i=0; i<numP; i++)
		{
			// Dragon
			if ((dloc * 5) + 5 == i)
				printf("Dragon\t \t%d\n", dloc);
			// players on a continent
			if (i<20)
				printf("%d\t%c\t%d\n", globalQ[i].num,
						globalQ[i].type, globalQ[i].cont);
			// players in queue
			else
				printf("%d\t%c\tqueue\n", globalQ[i].num,
						globalQ[i].type);
		}
		printf("\n\n");
		pthread_cond_broadcast(&cv);
		doneT = 0;
		pthread_mutex_unlock(&mv);
	}

	// join threads
	for (i=0; i<4; i++)
		pthread_join(id[i], NULL);

	// print out final world
	printf("Final World:\n\n");
	printf("Player\tType\tContinent\n");
	for (i=0; i<numP; i++)
	{
		if ((dloc * 5) + 5 == i)
			printf("Dragon\t \t%d\n", dloc);
		if (i<20)
			printf("%d\t%c\t%d\n", globalQ[i].num,
					globalQ[i].type, globalQ[i].cont);
		else
			printf("%d\t%c\tqueue\n", globalQ[i].num, globalQ[i].type);
	}

	return 0;

}
