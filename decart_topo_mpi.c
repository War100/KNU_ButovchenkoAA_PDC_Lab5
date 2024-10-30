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

        int dims[2] = {SIZE/6, SIZE/4};     // Розміри топології 4x6
        int periods[2] = {0, 0};  // Без періодичності

        MPI_Comm cart_comm;

        // Створення декартової топології
        MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

        if(size == SIZE) {

                if(rank == 0){
                        printf("\n----------Neighbor table-----------\n\n");
                }

                // Визначення сусідів
                int left, right, up, down;
                MPI_Cart_shift(cart_comm, 1, 1, &left, &right);  // Зліва і справа
                MPI_Cart_shift(cart_comm, 0, 1, &up, &down);     // Зверху і знизу

                for(int current_process = 0; current_process < SIZE; current_process++){
                        if(rank == current_process){
                                printf("Process %d: left = %d, right = %d, up = %d, down = %d\n",
                                                rank, left, right, up, down);
                        }
                        MPI_Barrier(cart_comm);
                }

                int neighbors[4] = {left, right, up, down};     // Сусідні процеси
                int send_message[4] = {-256, -256, -256, -256}; // Повідомлення до відправлення
                int recv_message[4] = {-256, -256, -256, -256}; // Отримане повідомлення

                srand(time(NULL) + rank);  // Ініціалізація генератора випадкових чисел

                MPI_Barrier(cart_comm);

                if(rank == 0){
                        printf("\n----------Content before sending-----------\n\n");
                }
                // Виведення вмісту до надсилання
                for(int current_process = 0; current_process < SIZE; current_process++){
                        if(rank == current_process){
                                printf("Process %d container: ", rank);
                                for(int i = 0; i < 4; i++){
                                        if(neighbors[i] >= 0)
                                                printf("%d, ", recv_message[i]);
                                }
                                printf("\n");
                        }
                        MPI_Barrier(cart_comm);
                }

                if(rank == 0){
                        printf("\n----------Sending-----------\n\n");
                }

                // Надсилання повідомлень всім сусідам
                for(int current_process = 0; current_process < SIZE; current_process++){
                        if(rank == current_process){
                                printf("[s] Process %d\n", rank);

                                // Для всіх сусідів, що існують
                                for(int i = 0; i < 4; i++){
                                        if(neighbors[i] >= 0){
                                                send_message[i] = rand() % 256; // Генерування випадкового числа для передачі

                                                printf("\t%d) Sending: %d to %d process\n", i, send_message[i], neighbors[i]);

                                                // Передача повідомлення сусіду
                                                MPI_Send(
                                                                &send_message[i],
                                                                1,
                                                                MPI_INT,
                                                                neighbors[i],
                                                                0,
                                                                cart_comm
                                                        );
                                        }
                                }
                        }
                        MPI_Barrier(cart_comm);
                }


                if(rank == 0){
                        printf("\n----------Recieving-----------\n\n");
                }

                // Отримання надісланих повідомлень
                for(int current_process = 0; current_process < SIZE; current_process++){
                        if(rank == current_process){
                                printf("[r] Process %d\n", rank);
                                for(int i = 0; i < 4; i++){
                                        if(neighbors[i] >= 0){
                                                // Власне отримання повідомлення від того самого сусіда
                                                MPI_Recv(
                                                                &recv_message[i],
                                                                1,
                                                                MPI_INT,
                                                                neighbors[i],
                                                                0,
                                                                cart_comm,
                                                                MPI_STATUS_IGNORE
                                                        );
                                                printf("\tRecieving from %d...number %d\n", neighbors[i], recv_message[i]);
                                        }
                                }
                        }
                        MPI_Barrier(cart_comm);
                }


                if(rank == 0){
                        printf("\n----------Content after recieving-----------\n\n");
                }

                // Виведення вмісту після надсилання
                for(int current_process = 0; current_process < SIZE; current_process++){
                        if(rank == current_process){
                                printf("Process %d container: ", rank);
                                for(int i = 0; i < 4; i++){
                                        if(neighbors[i] >= 0)
                                                printf("%d, ", recv_message[i]);
                                }
                                printf("\n");
                        }
                        MPI_Barrier(cart_comm);
                }

        }
        else if(rank == 0) {
                // Виведення попередження при неправильній кількості вказаних процесів
                printf("Error !!! \n");
                printf("The number of processes must be = %d \n",SIZE); 
                printf("The number of processes given = %d \n", size);
        }

        if(rank < SIZE)
                MPI_Comm_free(&cart_comm);

        MPI_Finalize();
        return 0;
}
