
#include "../Test.h"
#include "../common/cephes.h"

#define MAXCYCLES 1000

// random excursion in original version does not work with i=1999 and j=4

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
R A N D O M  E X C U R S I O N S  T E S T
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define MAX(A,B) ((A)>(B)?(A):(B))

void
RandomExcursions(Nist::Test& test)
{
	auto n = test.GetN();
	auto epsilon = test.GetEpsilon();
	auto& R_ = test.GetWriteableResults();

	int		b, i, j, k, J, x;
	int		cycleStart, cycleStop, *cycle = NULL, *S_k = NULL;
	int		stateX[8] = { -4, -3, -2, -1, 1, 2, 3, 4 };
	int		counter[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }, nu[6][8];
	double	p_value, sum, constraint;
	double	pi[5][6] = { { 0.0000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.00000000000, 0.0000000000 },
	{ 0.5000000000, 0.25000000000, 0.12500000000, 0.06250000000, 0.03125000000, 0.0312500000 },
	{ 0.7500000000, 0.06250000000, 0.04687500000, 0.03515625000, 0.02636718750, 0.0791015625 },
	{ 0.8333333333, 0.02777777778, 0.02314814815, 0.01929012346, 0.01607510288, 0.0803755143 },
	{ 0.8750000000, 0.01562500000, 0.01367187500, 0.01196289063, 0.01046752930, 0.0732727051 } };
	//int maxcyc;

	R_.random_excursion.valid = 0;
	//maxcyc=MAX(MAXCYCLES, n/100);

	if (((S_k = (int *)calloc(n, sizeof(int))) == NULL) ||
		((cycle = (int *)calloc(1 + MAX(MAXCYCLES, n / 100), sizeof(int))) == NULL)) {  // fixed bug accessing later one more element :-)
		printf("Random Excursions Test:  Insufficent Work Space Allocated.\n");
		if (S_k != NULL)
			free(S_k);
		if (cycle != NULL)
			free(cycle);
#ifdef VERIFY_RESULTS
		if (RandomExcursions_v1 == RandomExcursions) R1 = R_;
		else R2 = R_;
#endif
		return;
	}

	J = 0; 					/* DETERMINE CYCLES */
	S_k[0] = 2 * (int)epsilon[0] - 1;
	for (i = 1; i<n; i++) {
		S_k[i] = S_k[i - 1] + 2 * epsilon[i] - 1;
		if (S_k[i] == 0) {
			J++;
			if (J >= MAX(MAXCYCLES, n / 100)) {
				printf("ERROR IN FUNCTION randomExcursions:  EXCEEDING THE MAX NUMBER OF CYCLES EXPECTED\n.");
				free(S_k);
				free(cycle);
#ifdef VERIFY_RESULTS
				if (RandomExcursions_v1 == RandomExcursions) R1 = R_;
				else R2 = R_;
#endif
				return;
			}
			//if(J>=maxcyc)printf("STOP1: accesing wrong element (n=%i).\n",n);
			cycle[J] = i;
		}
	}
	if (S_k[n - 1] != 0)
		J++;
	//if(J>=maxcyc)printf("STOP2: accesing wrong element (n=%i) [%i vs %i].\n",n,J,maxcyc);
	cycle[J] = n;

#if defined(FILE_OUTPUT) ||  defined(KS)
	if (cmdFlags.output == 1 || cmdFlags.output == -1) {
		fprintf(stats[TEST_RND_EXCURSION], "\t\t\t  RANDOM EXCURSIONS TEST\n");
		fprintf(stats[TEST_RND_EXCURSION], "\t\t--------------------------------------------\n");
		fprintf(stats[TEST_RND_EXCURSION], "\t\tCOMPUTATIONAL INFORMATION:\n");
		fprintf(stats[TEST_RND_EXCURSION], "\t\t--------------------------------------------\n");
		fprintf(stats[TEST_RND_EXCURSION], "\t\t(a) Number Of Cycles (J) = %04d\n", J);
		fprintf(stats[TEST_RND_EXCURSION], "\t\t(b) Sequence Length (n)  = %d\n", n);
	}
#endif

	constraint = MAX(0.005*pow(n, 0.5), 500);
	if (J < constraint) {
#if defined(FILE_OUTPUT) ||  defined(KS)
		if (cmdFlags.output == 1 || cmdFlags.output == -1) {
			fprintf(stats[TEST_RND_EXCURSION], "\t\t---------------------------------------------\n");
			fprintf(stats[TEST_RND_EXCURSION], "\t\tWARNING:  TEST NOT APPLICABLE.  THERE ARE AN\n");
			fprintf(stats[TEST_RND_EXCURSION], "\t\t\t  INSUFFICIENT NUMBER OF CYCLES.\n");
			fprintf(stats[TEST_RND_EXCURSION], "\t\t---------------------------------------------\n");
			for (i = 0; i < 8; i++)
				fprintf(results[TEST_RND_EXCURSION], "%f\n", 0.0);
		}
#endif
		//printf("WARNING: INSUFFICIENT NUMBER OF CYCLES (n=%i, J=%i, constraint=%i).\n",n,J,(int)constraint);
	}
	else {
#if defined(FILE_OUTPUT) ||  defined(KS)
		if (cmdFlags.output == 1 || cmdFlags.output == -1) {
			fprintf(stats[TEST_RND_EXCURSION], "\t\t(c) Rejection Constraint = %f\n", constraint);
			fprintf(stats[TEST_RND_EXCURSION], "\t\t-------------------------------------------\n");
		}
#endif
		cycleStart = 0;
		//if(1>=maxcyc)printf("STOP3: accesing wrong element (n=%i).\n",n);
		cycleStop = cycle[1];
		for (k = 0; k<6; k++)
			for (i = 0; i<8; i++)
				nu[k][i] = 0;
		for (j = 1; j <= J; j++) {                           /* FOR EACH CYCLE */
			for (i = 0; i<8; i++)
				counter[i] = 0;
			for (i = cycleStart; i<cycleStop; i++) {
				if ((S_k[i] >= 1 && S_k[i] <= 4) || (S_k[i] >= -4 && S_k[i] <= -1)) {
					if (S_k[i] < 0)
						b = 4;
					else
						b = 3;
					counter[S_k[i] + b]++;
				}
			}
			//if(j>=maxcyc)printf("STOP4: accesing wrong element (n=%i)  [%i vs %i].\n",n,j,maxcyc);
			cycleStart = cycle[j] + 1;
			if (j < J)
			{
				//if((j+1)>=maxcyc)printf("STOP5: accesing wrong element (n=%i)  [%i vs %i].\n",n,j+1,maxcyc);
				cycleStop = cycle[j + 1];
			}

			for (i = 0; i<8; i++) {
				if ((counter[i] >= 0) && (counter[i] <= 4))
					nu[counter[i]][i]++;
				else if (counter[i] >= 5)
					nu[5][i]++;
			}
		}

		//PRINT

		/*
		printf("Cycle count = %d \n",J);
		for( i = 0; i < 8; i++)
		{
		for ( k=0; k<6; k++ )
		{
		printf("%5d",nu[k][i]);
		}
		printf("\n");
		}
		*/


		for (i = 0; i < 8; i++) {
			x = stateX[i];
			sum = 0.;
			for (k = 0; k < 6; k++)
				sum += pow(nu[k][i] - J*pi[(int)fabs(x)][k], 2) / (J*pi[(int)fabs(x)][k]);
			p_value = cephes_igamc(2.5, sum / 2.0);
#ifdef SPEED
			dummy_result += p_value;
#endif
#if defined(FILE_OUTPUT) ||  defined(KS)
			if (cmdFlags.output == 1 || cmdFlags.output == -1) {
				if (isNegative(p_value) || isGreaterThanOne(p_value))
					fprintf(stats[TEST_RND_EXCURSION], "WARNING:  P_VALUE IS OUT OF RANGE.\n");

				fprintf(stats[TEST_RND_EXCURSION], "%s\t\tx = %2d chi^2 = %9.6f p_value = %f\n",
					p_value < ALPHA ? "FAILURE" : "SUCCESS", x, sum, p_value);
				fprintf(results[TEST_RND_EXCURSION], "%f\n", p_value); fflush(results[TEST_RND_EXCURSION]);
			}
#endif
#ifdef KS
			pvals.random_excursion_pvals[i][pvals.seq_counter] = p_value;
#endif

			R_.random_excursion.valid = 1;
			R_.random_excursion.J[i] = J;
			R_.random_excursion.x[i] = x;
			R_.random_excursion.p_value[i] = p_value;
			R_.random_excursion.sum[i] = sum;
		}
	}
#if defined(FILE_OUTPUT) ||  defined(KS)
	if (cmdFlags.output == 1 || cmdFlags.output == -1) {
		fprintf(stats[TEST_RND_EXCURSION], "\n"); fflush(stats[TEST_RND_EXCURSION]);
	}
#endif

	free(S_k);
	free(cycle);

#ifdef VERIFY_RESULTS
	if (RandomExcursions_v1 == RandomExcursions) R1 = R_;
	else R2 = R_;
#endif

}