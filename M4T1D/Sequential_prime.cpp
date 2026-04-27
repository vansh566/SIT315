#include <iostream>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <iomanip>

using namespace std;
using namespace std::chrono;

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
    if (argc < 2) {
        cout << "Usage: ./sequential_prime <max_number>" << endl;
        return 1;
    }

    long long maxNumber = atoll(argv[1]);
    long long primeCount = 0;

    auto start = high_resolution_clock::now();

    for (long long i = 1; i <= maxNumber; i++) {
        if (isPrime(i)) {
            primeCount++;
        }
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << fixed << setprecision(6);
    cout << "==============================================" << endl;
    cout << "       Sequential Prime Detection Result       " << endl;
    cout << "==============================================" << endl;
    cout << "Input Range              : 1 to " << maxNumber << endl;
    cout << "Processes Used           : 1" << endl;
    cout << "Threads Used             : 1" << endl;
    cout << "Total Prime Numbers      : " << primeCount << endl;
    cout << "Execution Time           : " << elapsed.count() << " seconds" << endl;
    cout << "==============================================" << endl;

    return 0;
}
