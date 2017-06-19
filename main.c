#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi/mpi.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

int rank, size;

float nextTime(float rateParameter)
{
    float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
    //printf("next: %f\n",temp);

    return temp;
}

void *client_func(void* arg){
    int number = -1;
    int thread_id = (int) arg;
    struct timeval afterSleep;
    float nextTx, now;
    double d1, d2;
    srand(clock());
    MPI_Status status;
    while(1){
        now = (nextTime(350) * 1000000);  // speed = 350
        usleep(now);
        gettimeofday(&afterSleep, 0);
        d1 = afterSleep.tv_sec*1000000.0 +  afterSleep.tv_usec;

        MPI_Send(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        gettimeofday(&afterSleep, 0);
        d2 = afterSleep.tv_sec*1000000.0 +  afterSleep.tv_usec;
        //printf("after: %.10f \n", d2);
        printf("rank:%d thread:%d from rank:%d %d\n",rank, thread_id, status.MPI_SOURCE, (int) (d2 - d1) );

        ++number;
        if(number == 10000)
            number = -1;
    }

}

void *server_func(void* arg){
    int number = -1;
    MPI_Status status;
    while(1){
        MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        ++number;
        if(number == 10000)
            number = -1;
        MPI_Send(&number, 1, MPI_INT, status.MPI_SOURCE , 0, MPI_COMM_WORLD);
    }

}


int main(int argc, char **argv)
{
    void *stat;
    pthread_attr_t attr;
    pthread_t *sthread, *cthread;

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // rank -> rank of this processor
    MPI_Comm_size(MPI_COMM_WORLD, &size);   // size -> total number of processors

    //srand((unsigned)time(NULL));

    int servers_count = atoi(argv[1]);
    int clients_count = atoi(argv[2]);

    if(rank==0){
        printf("server ready, code: %d threads: %d \n", rank, servers_count);
        // create threads for event based arch style
        sthread = (pthread_t*) malloc(servers_count * sizeof(pthread_t));
        int i;
        for(i=0;i < servers_count;i++){
            pthread_create(&sthread[i], NULL, server_func, (void *)i);
        }
    }
    else{
        // create sender threads
        printf("client ready, code: %d threads: %d \n", rank, clients_count);
        cthread = (pthread_t*) malloc(clients_count * sizeof(pthread_t));
        int i;
        for(i=0;i < clients_count;i++){
            pthread_create(&cthread[i], NULL, client_func, (void *)i);
        }
    }

    int i;
    if(rank==0){
        // create threads for event based arch style
        for(i=0;i < servers_count;i++){
            pthread_join(sthread[i], &stat);
        }
        free(sthread);
    }
    else{
        // create sender threads
        for(i=0;i < clients_count;i++){
            pthread_join(cthread[i], &stat);
        }
        free(cthread);
    }
    printf("Finished : Proc %d \n", rank);

    MPI_Finalize();
    pthread_exit((void *)NULL);
    return 0;
}
