#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "pgm_IO.h"

/*
 
    //ALGORITMUL DE MAI JOS ESTE IN SERIE

int main(int argc, char **argv) {
    char *str = "image_640x480.pgm";
    char *str_output = "output_image_640x480.pgm";
    int n, m;
    pgm_size(str, &m, &n);
    float data[m][n];
    pgm_read(str, data, m, n);
    float plim[m+2][n+2], pold[m+2][n+2], pnew[m+2][n+2];
    for(int i = 0 ; i <= m+1 ; i++)
        for(int j = 0 ; j <= n+1 ; j++) {
            pold[i][j] = 255;
            pnew[i][j] = 255;
            plim[i][j] = 255;
        }
    for(int i = 1 ; i <= m ; i++)
        for(int j = 1 ; j <= n ; j++)
            plim[i][j] = data[i-1][j-1];

    for(int k = 1 ; k <= atoi(argv[1]) ; k++) {
        for(int i = 1 ; i <= m ; i++)
            for(int j = 1 ; j <= n ; j++)
                pnew[i][j] = 0.25*(pold[i-1][j]+pold[i+1][j]+pold[i][j-1]+pold[i][j+1]-plim[i][j]);
        for(int i = 1 ; i <= m ; i++)
            for(int j = 1 ; j <= n ; j++)
                pold[i][j] = pnew[i][j];
    }
    for(int i = 1 ; i <= m ; i++)
        for(int j = 1 ; j <= n ; j++)
            data[i-1][j-1] = pold[i][j];
    pgm_write(str_output, data, m, n);
}
*/

int main(int argc, char **argv) {
    char *str = "image_640x480.pgm";
    int n, m;
    pgm_size(str, &m, &n);
    MPI_Status status;
    float masterdata[m][n];
 
    int numprocs, rank;
    int root=0;

    int nriter = atoi(argv[1]);

    // initializare MPI
    MPI_Init ( &argc, &argv);
    MPI_Comm_size ( MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank);

    int mp = m / numprocs;
    int np = n;

    // alocare memorie si pgm_read
    if (rank ==root) {
        pgm_read(str, masterdata, m, n);
    }
    
    float data[mp][np], pold[mp+2][np+2], pnew[mp+2][np+2], plim[mp+2][np+2];

    MPI_Scatter(masterdata, mp*np, MPI_FLOAT, data, mp*np, MPI_FLOAT, root, MPI_COMM_WORLD);

    for(int i = 0 ; i <= mp+1 ; i++)
        for(int j = 0 ; j <= np+1 ; j++) {
            pold[i][j] = 255;
            pnew[i][j] = 255;
            plim[i][j] = 255;
    }

    for(int i = 1 ; i <= mp ; i++)
        for(int j = 1 ; j <= np ; j++)
            plim[i][j] = data[i-1][j-1];

    for(int k = 1 ; k <= nriter ; k++) {
        int rankupper = rank+1;
        int ranklower = rank-1;
        if(rankupper == numprocs)
            rankupper = MPI_PROC_NULL;
        if(ranklower == -1)
            ranklower = MPI_PROC_NULL;
        for(int j = 1 ; j <= np ; j++){
            MPI_Sendrecv(&pold[mp][j], np, MPI_FLOAT, rankupper, 0, &pold[0][j], np, MPI_FLOAT, ranklower, 0, MPI_COMM_WORLD, &status);
            MPI_Sendrecv(&pold[1][j], np, MPI_FLOAT, ranklower, 1, &pold[mp+1][j], np, MPI_FLOAT, rankupper, 1, MPI_COMM_WORLD, &status);
        }
        for(int i = 1 ; i <= mp ; i++)
            for(int j = 1 ; j <= np ; j++)
                pnew[i][j] = 0.25*(pold[i-1][j]+pold[i+1][j]+pold[i][j-1]+pold[i][j+1]-plim[i][j]);
        for(int i = 1 ; i <= mp ; i++)
            for(int j = 1 ; j <= np ; j++)
                pold[i][j] = pnew[i][j];
    }
    for(int i = 1 ; i <= mp ; i++)
        for(int j = 1 ; j <= np ; j++)
            data[i-1][j-1] = pold[i][j];
    MPI_Gather(data, mp*np, MPI_FLOAT, masterdata, mp*np, MPI_FLOAT, root, MPI_COMM_WORLD);
    if(rank == root){
        char str_output[255];
        char nriterstr[255];
        strcpy(str_output, "output_image_640x480_");
        sprintf(nriterstr, "%d", nriter);
        strcat(str_output, nriterstr);
        strcat(str_output, ".pgm");
        pgm_write(str_output, masterdata, m, n);
    }
    MPI_Finalize();
}