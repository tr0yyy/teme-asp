#include "pgm_IO.h"
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int source, my_rank, nproc, tag = 1;
    int NX, NY, i, j, dims[NDIM], GSIZE = XSIZE * YSIZE;
    int reorder = 0, periods[NDIM] = {TRUE, TRUE};
    int dX, dY, localSize;
    float *masterdata, *data;
    int coords[NDIM];
    MPI_Comm wcomm = MPI_COMM_WORLD, comm_2D;
    MPI_Status status;
    MPI_Datatype blockType;
    char fname[32];
    strcpy(fname, "chessy_struct.pgm");

    /**
     * pasul 1
     * 
     */

    for(i = 0 ; i < NDIM ; i++) {
        dims[i] = 0;
    }

    MPI_Init ( &argc, &argv);
    MPI_Comm_size ( MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank);
    MPI_Dims_create(nproc, NDIM, dims);
    NX = dims[0];
    NY = dims[1];
    MPI_Cart_create(wcomm, NDIM, dims, periods, reorder, &comm_2D);

    /**
     * pasul 2
     * 
     */

    if(my_rank == 0) {
        masterdata = (float*) malloc (GSIZE * sizeof(float));
        for(i = 0 ; i < GSIZE ; i++) {
            masterdata[i] = 0.75 * CONTRAST;
        }
    } 
    data = (float*) malloc ((XSIZE / NX) * (YSIZE / NY) * sizeof(float));
    for(i = 0 ; i < (XSIZE / NX) * (YSIZE / NY) ; i++) {
        data[i] = 0.75 * CONTRAST;
    }

    /**
     * pasul 3
     * 
     */

    MPI_Datatype lineType, colType;

    MPI_Cart_coords(comm_2D, my_rank, NDIM, coords);
    if((coords[0] + coords[1] + 1) % 2 == 1) {
        for(i = 0 ; i < (XSIZE / NX) * (YSIZE / NY) ; i++) {
            data[i] = 0;
        }
    } else {
        for(i = 0 ; i < (XSIZE / NX) * (YSIZE / NY) ; i++) {
            data[i] = CONTRAST;
        }
    }

    dX = XSIZE / NX;
    dY = YSIZE / NY;

    MPI_Type_vector(1, dY, 1, MPI_FLOAT, &lineType);
    MPI_Type_commit(&lineType);

    MPI_Type_vector(dX, 1, dY, MPI_FLOAT, &colType);
    MPI_Type_commit(&colType);

    int up, down, left, right;

    MPI_Cart_shift(comm_2D, 0, 1, &up, &down);
    MPI_Cart_shift(comm_2D, 1, 1, &left, &right);

    MPI_Sendrecv(data + dY, 1, lineType, up, 3, data + (dX - 1) * dY, 1, lineType, down, 3, comm_2D, &status);
    MPI_Sendrecv(data + (dX - 1) * dY, 1, lineType, down, 3, data + dY, 1, lineType, up, 3, comm_2D, &status);



    MPI_Type_vector(dX, dY, YSIZE, MPI_FLOAT, &blockType);
    MPI_Type_commit(&blockType);

    /**
     * pasul 4
     * 
     */

    if(my_rank == 0) {
        for(i = 0 ; i < dX ; i++) {
            for(j = 0 ; j < dY ; j++) {
                *(masterdata + i*YSIZE + j) = *(data + i * dY + j);
            }
        }
    } else {
        MPI_Send(data, dX * dY, MPI_FLOAT, MASTER, tag, wcomm);
    }

    if(my_rank == 0) {
        for(source = 1 ; source < nproc ; source++) {
            MPI_Cart_coords(comm_2D, source, NDIM, coords);
            MPI_Recv((masterdata + coords[1] * dY + coords[0] * dX * YSIZE), 1, blockType, source, tag, wcomm, &status);
        }
        pgm_write(fname, masterdata, XSIZE, YSIZE);
    }
    MPI_Finalize();
}