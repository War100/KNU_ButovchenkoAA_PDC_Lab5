#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 24

int main(int argc, char *argv[]) {
        int rank, size;    // Поточний процес, загальна кількість використовуваних процесів

        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        srand(time(NULL) + rank);  // Ініціалізація генератора випадкових чисел
        int random_number = rand() % 256;  // Випадкове число від 0 до 255

        // Графова топологія - кожен процес зв'язаний зі своїми двома сусідами у кільці
        int neighbors[2] = {(rank + 1) % size, (rank - 1 + size) % size};
        MPI_Comm graph_comm;
        int indices[SIZE], edges[SIZE*2];

        for (int i = 0; i < size; i++) {
                indices[i] = 2 * i + 2;
                edges[2 * i] = (i + 1) % size;
                edges[2 * i + 1] = (i - 1 + size) % size;
        }
        MPI_Graph_create(MPI_COMM_WORLD, size, indices, edges, 1, &graph_comm);

        int graph_rank;
        MPI_Comm_rank(graph_comm, &graph_rank);

        for(int i = 0; i < SIZE; i++){
                if(rank == i)
                        // Вивід до надсилання в графовій топології
                        printf("[bs] Process %d with random number %d\n", rank, random_number);
                MPI_Barrier(graph_comm);
        }

        int graph_received_numbers[2];  // Змінна отриманих значень
        MPI_Request graph_requests[4];
        int graph_index = 0;

        if(rank == 0)
                printf("\n");

        // Надсилання/отримання даних у графовій топології
        MPI_Isend(&random_number, 1, MPI_INT, neighbors[0], 0, graph_comm, &graph_requests[graph_index++]);
        MPI_Irecv(&graph_received_numbers[0], 1, MPI_INT, neighbors[0], 0, graph_comm, &graph_requests[graph_index++]);
        MPI_Isend(&random_number, 1, MPI_INT, neighbors[1], 0, graph_comm, &graph_requests[graph_index++]);
        MPI_Irecv(&graph_received_numbers[1], 1, MPI_INT, neighbors[1], 0, graph_comm, &graph_requests[graph_index++]);

        MPI_Waitall(graph_index, graph_requests, MPI_STATUSES_IGNORE);

        for(int i = 0; i < SIZE; i++){
                if(rank == i)
                        // Вивід після надсилання в графовій топології
                        printf("[as] Process %d received numbers from neighbors: %d, %d\n",
                                        rank, graph_received_numbers[0], graph_received_numbers[1]);
                MPI_Barrier(graph_comm);
        }

        MPI_Comm_free(&graph_comm);
        MPI_Finalize();
        return 0;
}
