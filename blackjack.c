#import <stdio.h>    // printf

/* Define a MAX macro if we don't already have one */
#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

void calcDealerProbabilityTable();
void printDealerProbabilityTable();
void calcExpectedProfitTables();
void printExpectedProfitTable();

char calcBest(int pCount, int dCount, float * ep);
char calcBestSoft(int otherCard, int dCount, float * ep);
char calcBestSplitable(int pCard, int dCount, float * ep);
void calcOverallExpectedProfit();

// probability of [card showing][(final count - 17),(last column bust)]
static float dProb[11][6];

// ep = expected profit = wager * (probability_win - probability_lose)

// ep if player stands at [x] and dealer shows [y]
static float epStand[22][11];

// ep if player hits (with a hard hand)
static float epHit[22][11];

// ep if player hits (with a soft hand)
static float epSoftHit[22][11];

// ep if player doubles down (with a hard hand)
static float epDoubleDown[22][11];

// ep if player doubles down (with a soft hand)
static float epSoftDoubleDown[22][11];

// ep if player splits with [x]'s and dealer shows [y]
static float epSplit[11][11];

int main (int argc, const char * argv[]) {
	calcDealerProbabilityTable();
	printDealerProbabilityTable();
	
	calcExpectedProfitTables();
	printExpectedProfitTable();
	
	calcOverallExpectedProfit(); // prints it also
	return 0;
}

char calcBest(int pCount, int dCount, float * ep)
{
	float stand = epStand[pCount][dCount];
	float hit = epHit[pCount][dCount];
	float doubleDown = epDoubleDown[pCount][dCount];
	char result;
	
	if (stand > hit && stand > doubleDown) {
		*ep = stand;
		result = 'S';
	} else if (hit > doubleDown) {
		*ep = hit;
		result = 'H';
	} else if (hit > stand) {
		*ep = doubleDown;
		result = 'D';
	} else {
        *ep = doubleDown;
        result = 'd';
    }
	if (stand < -0.5 && hit < -0.5 && doubleDown < -0.5) {
		*ep = -0.5;
		return result - ('A' - 'a');
	}
	return result;
}

char calcBestSoft(int otherCard, int dCount, float * ep)
{
	int pCount = otherCard + 11;
	float stand = epStand[pCount][dCount];
	float hit = epSoftHit[pCount][dCount];
	float doubleDown = epSoftDoubleDown[pCount][dCount];
	char result;
	
	if (stand > hit && stand > doubleDown) {
		*ep = stand;
		result = 'S';
	} else if (hit > doubleDown) {
		*ep = hit;
		result = 'H';
	} else {
		*ep = doubleDown;
		result = 'D';
	}
	if (stand < -0.5 && hit < -0.5 && doubleDown < -0.5) {
		*ep = -0.5;
		return result - ('A' - 'a');
	}
	return result;
}

char calcBestSplitable(int pCard, int dCount, float * ep)
{
	int pCount = 2 * pCard + (pCard==1?10:0);
	float split = epSplit[pCard][dCount];
	float stand = epStand[pCount][dCount];
	float hit = pCard==1? epSoftHit[pCount][dCount] : epHit[pCount][dCount];
	float doubleDown = pCard==1? epSoftDoubleDown[pCount][dCount] : epDoubleDown[pCount][dCount];
	char result;
	
	if (split > stand && split > hit && split > doubleDown) {
		*ep = split;
		result = 'P';
	} else if (stand > hit && stand > doubleDown) {
		*ep = stand;
		result = 'S';
	} else if (hit > doubleDown) {
		*ep = hit;
		result = 'H';
	} else {
		*ep = doubleDown;
		result = 'D';
	}
	if (split < -0.5 && stand < -0.5 && hit < -0.5 && doubleDown < -0.5) {
		*ep = -0.5;
		return result - ('A' - 'a');
	}
	return result;
}

void calcDealerProbabilityTable()
{
	float pt[22][6]; //  probability table
	float spt[22][6]; // soft probability table (soft 12 - 21)
	int i,j,jmax,cnt,k,si;
	float p;
	
	for (i=0;i<22;i++) // initialize tables to zero
		for (j=0;j<6;j++)
			spt[i][j] = pt[i][j] = 0.0;
	for (i=17;i<=21;i++) // dealer stands on hard 17+
		pt[i][i-17] = 1.0;
	for (i=18;i<=21;i++) // dealer stands on soft 18+
		spt[i][i-17] = 1.0;
	
	for (i=16; i>0; i--) {
		if (i>11) {
			// probability of busting
			pt[i][5] = (i - 8) / 13.0;
			jmax = 21 - i;
		} else {
			jmax = 10;
		}
		for (j=jmax; j>=1; j--) {
			p = j==10 ? 4/13.0 : 1/13.0;
			cnt = i + j;
			if (cnt > 16) {
				pt[i][cnt-17] += p;
			} else if (i==1 || (j==1 && i<=10)) {
				cnt += 10; // soft hand
				for (k=0;k<6;k++)
					pt[i][k] += p * spt[cnt][k];
			} else {
				for (k=0;k<6;k++)
					pt[i][k] += p * pt[cnt][k];
			}
		}
		
		if (i==12) { // solve the spt after solving pt[12...21][]
			for (si=17; si>=12; si--) {
				for (j=10; j>=1; j--) {
					p = j==10 ? 4/13.0 : 1/13.0;
					cnt = si + j;
					if (cnt > 21) {
						cnt -= 10;
						for (k=0;k<6;k++)
							spt[si][k] += p * pt[cnt][k];
					} else {
						for (k=0;k<6;k++)
							spt[si][k] += p * spt[cnt][k];
					}
				}
			}
		}
	}
	// adjusted for the fact that if dealer has blackjack, you don't get to play
	// if dealer is showing a 10
	for (k=0;k<6;k++)
		pt[10][k] = 0.0; // reinitialize 10 row
	for (j=10; j>=2; j--) { // 2 - 10 is possible (but not 1)
		p = j==10 ? 4/12.0 : 1/12.0; // out of 12, not 13 because it can't be an ace
		cnt = 10 + j;
		if (cnt > 16) {
			pt[10][cnt-17] += p;
		} else {
			for (k=0;k<6;k++)
				pt[10][k] += p * pt[cnt][k];
		}
	}
	// if dealer is showing an ace
	for (k=0;k<6;k++)
		pt[1][k] = 0.0; // reinitialize 1 row
	p = 1/9.0; // out of 9, not 13 because it can't be a 10
	for (j=9; j>=1; j--) { // 1 - 9 is possible (but not 10)
		cnt = 11 + j;
		for (k=0;k<6;k++)
			pt[1][k] += p * spt[cnt][k];
	}
	
	// copy results to dProb
	for (i=0;i<11;i++)
		for (j=0;j<6;j++)
			dProb[i][j] = pt[i][j];
}

void printDealerProbabilityTable()
{
	int i,j;
	
	printf("Dealer Probability Table:\n");
	printf("    ");
	for (j=17;j<=21;j++)
		printf("%7d",j);
	printf("  BUST\n");
	for (i=10;i>0;i--) {
		printf("%4d",i);
		for (j=0;j<6;j++)
			printf("%7.3f",dProb[i][j]);
		printf("\n");
	}
	printf("\n");
}

void printExpectedProfitTable()
{
	int pCount; // player count
	int dCount; // dealer count
	float ep;

	printf("Best strategy:\n");
	printf("      ");
	for (dCount = 2; dCount <= 10; dCount++) 
		printf("%2d ",dCount);
	printf(" A ");
	for (pCount = 21; pCount >= 5; pCount--) {
		printf("\n %3d  ",pCount);
		for (dCount = 2; dCount <= 10; dCount++) {
			printf(" %c ", calcBest(pCount, dCount, &ep));
		}
		printf(" %c ", calcBest(pCount, 1, &ep));
	}
	for (pCount = 10; pCount >= 2; pCount--) {
		printf("\n A,%2d ",pCount);
		for (dCount = 2; dCount <= 10; dCount++) {
			printf(" %c ", calcBestSoft(pCount, dCount, &ep));
		}
		printf(" %c ", calcBestSoft(pCount, 1, &ep));
	}
	printf("\n A,A  ");
	for (dCount = 2; dCount <= 10; dCount++) {
		printf(" %c ", calcBestSplitable(1, dCount, &ep));
	}
	printf(" %c ", calcBestSplitable(1, 1, &ep));
	for (pCount = 10; pCount >= 2; pCount--) {
		printf("\n%2d,%2d ",pCount,pCount);
		for (dCount = 2; dCount <= 10; dCount++) {
			printf(" %c ", calcBestSplitable(pCount, dCount, &ep));
		}
		printf(" %c ", calcBestSplitable(pCount, 1, &ep));
	}
	printf("\n\n");
	printf("H = hit\n");
	printf("S = stand\n");
	printf("D = double down\n");
	printf("P = split\n");
	printf("h = surrender if possible, otherwise hit\n");
	printf("s = surrender if possible, otherwise stand\n\n");
}

void calcExpectedProfitTables()
{
	int i,j,k,cnt,jmax,d,si;
	float p,aEpHit;
	
	// calc epStand table
	for (i=1; i<=21; i++) {
		for (j=1; j<=10; j++) {
			epStand[i][j] = 0.0; // init table
			for (k=0;k<5;k++) {
				cnt = 17+k; // dealer's hand
				if (cnt < i) // player beats dealer
					epStand[i][j] += dProb[j][k];
				else if (cnt > i) // dealer beats player
					epStand[i][j] -= dProb[j][k];
			}
			epStand[i][j] += dProb[j][5]; // dealer busts
		}
	}
	
	// calc epHit, epSoftHit, & epDouble tables
	for (d=1; d<=10; d++) { // dealer's card
		for (i=21; i>0; i--) { // i = current count
			epHit[i][d] = 0.0; // init tables
			epSoftDoubleDown[i][d] = epDoubleDown[i][d] = 0.0;
			if (i>11) {
				// probability of player busting
				epHit[i][d] -= (i - 8) / 13.0;
				epDoubleDown[i][d] -= 2 * (i - 8) / 13.0;
				jmax = 21 - i;
			} else {
				jmax = 10;
			}
			for (j=jmax; j>=1; j--) { // j = next card
				p = j==10 ? 4/13.0 : 1/13.0;
				cnt = i + j;
				if (i==1 || (j==1 && i<=10)) {
					cnt += 10; // soft hand
					aEpHit = epSoftHit[cnt][d];
				} else {
					aEpHit = epHit[cnt][d];
				}
				if (epStand[cnt][d] > aEpHit)
					epHit[i][d] += p * epStand[cnt][d];
				else
					epHit[i][d] += p * aEpHit;
				epDoubleDown[i][d] += 2 * p * epStand[cnt][d];
			}
			// solve the epSoftHit after solving epHit[12...21][]
			if (i==12) { 
				for (si=21; si>=12; si--) {
					for (j=10; j>=1; j--) {
						p = j==10 ? 4/13.0 : 1/13.0;
						cnt = si + j;
						if (cnt > 21) {
							cnt -= 10;
							aEpHit = epHit[cnt][d];
						} else {
							aEpHit = epSoftHit[cnt][d];
						}
						if (epStand[cnt][d] > aEpHit)
							epSoftHit[si][d] += p * epStand[cnt][d];
						else
							epSoftHit[si][d] += p * aEpHit;
						epSoftDoubleDown[si][d] += 2 * p * epStand[cnt][d];
					}
				}
			}
			
			if (i<=10) { // calculate epSplit
				epSplit[i][d] = 0.0;
				
				if (i==1) { // aces must double down (soft/hard doesn't matter)
					epSplit[i][d] = 2 * epDoubleDown[11][d];
				} else {
					for(j=1;j<=10;j++) { // next card
						if (j==i)
							continue;
						p = j==10 ? 4/13.0 : 1/13.0;
						cnt = i + j;
						if (j==1)
							epSplit[i][d] += 2*p*MAX(MAX(epSoftHit[cnt][d],epSoftDoubleDown[cnt][d]),epStand[cnt][d]);
						else
							epSplit[i][d] += 2*p*MAX(MAX(epHit[cnt][d],epDoubleDown[cnt][d]),epStand[cnt][d]);
					}
					// account for multiple splits...
					p = j==10 ? 4/13.0 : 1/13.0;
					epSplit[i][d] /= 1-2*p;
				}
			}
		}
	}
}

void calcOverallExpectedProfit()
{
	int i,j,k,f;
	float ep;
	float total = 0.0;
	
	for (i=1;i<=10;i++) { // player's card
		for (j=1;j<=10;j++) { // player's other card
			for (k=1;k<=10;k++) { // dealer's card
				f = (i==10?4:1) * (j==10?4:1) * (k==10?4:1);
				if ((i==1 && j==10) || (i==10 && j==1)) {
					if (k==1)
						ep = 1.5 * 9.0 / 13.0;
					else if (k==10)
						ep = 1.5 * 12.0 / 13.0;
					else
						ep = 1.5;
					total += f * ep;
				} else {
					if (i==j)
						calcBestSplitable(i,k,&ep);
					else if (i==1 || j==1)
						calcBestSoft(i==1?j:i,k,&ep);
					else
						calcBest(i+j,k,&ep);
					if (k==1)
						total += f * (ep * 9.0 / 13.0 - 4.0 / 13.0);
					else if (k==10)
						total += f * (ep * 12.0 / 13.0 - 1.0 / 13.0);
					else
						total += f * ep;
				}
			}
		}
	}
	total = total / 13 / 13 / 13;
	printf("Expected Profit = %f\n",total);
}
