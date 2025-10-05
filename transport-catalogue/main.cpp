#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;

    {
        InputReader reader;
        reader.ReadBaseRequests(cin, catalogue);
    }

    ProcessStatRequests(cin, cout, catalogue);

    return 0;
}