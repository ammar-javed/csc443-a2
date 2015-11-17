#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
#include "cmnhdr.hpp"
#include <fstream>

// Verbose status
bool verbose = false;

int main(int argc, char** argv) {

    if(argc < 3) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "read_fixed_len_page <page_file> <page_size> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 4 && strcmp(argv[3], "-v") == 0) {
       verbose = true;
    }

    return EXIT_SUCCESS;
}

