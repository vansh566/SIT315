#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <iomanip>

using namespace std;

bool isPrime(long long n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;

    long long limit = sqrt(n);

    for (long long i = 3; i <= limit; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            cout << "Usage: mpirun -np <processes> ./hybrid_prime <max_number>" << endl;
        }

        MPI_Finalize();
        return 1;
    }

    long long maxNumber = atoll(argv[1]);

    long long numbersPerProcess = maxNumber / size;
    long long startRange = rank * numbersPerProcess + 1;
    long long endRange;

    if (rank == size - 1) {
        endRange = maxNumber;
    } else {
        endRange = (rank + 1) * numbersPerProcess;
    }

    double startTime = MPI_Wtime();

    long long localPrimeCount = 0;

    #pragma omp parallel for reduction(+:localPrimeCount) schedule(dynamic)
    for (long long i = startRange; i <= endRange; i++) {
        if (isPrime(i)) {
            localPrimeCount++;
        }
    }

    long long totalPrimeCount = 0;

    MPI_Reduce(
        &localPrimeCount,
        &totalPrimeCount,
        1,
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD
    );

    double endTime = MPI_Wtime();
    double executionTime = endTime - startTime;

    if (rank == 0) {
        cout << fixed << setprecision(6);
        cout << "==============================================" << endl;
        cout << "   Hybrid MPI + OpenMP Prime Detection Result  " << endl;
        cout << "==============================================" << endl;
        cout << "Input Range              : 1 to " << maxNumber << endl;
        cout << "MPI Processes Used       : " << size << endl;
        cout << "OpenMP Threads per Rank  : " << omp_get_max_threads() << endl;
        cout << "Total Prime Numbers      : " << totalPrimeCount << endl;
        cout << "Execution Time           : " << executionTime << " seconds" << endl;
        cout << "==============================================" << endl;
    }

    MPI_Finalize();

    return 0;
}
