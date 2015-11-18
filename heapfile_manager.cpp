#include "lib.cpp"

int main(int argc, char** argv) {
    if(argc < 2) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "heap_tester <heap_filee> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 3 && strcmp(argv[2], "-v") == 0) {
       verbose = true;
    }

    if (verbose) {
        cout << "\n== Testing init_heapfile" << endl;
    }

    FILE *file_ptr = fopen(argv[1], "w+");

    Heapfile *hf = new Heapfile();
    int size = 80;
    init_heapfile(hf, size, file_ptr);
	if (verbose){
        cout << "Page size of heapfile: " << hf->page_size << endl;
        cout << "Contents of the page size: " << hf->file_ptr << endl;
    }

    if (verbose) {
        cout << "\n== Testing alloc_page" << endl;
    }

     Offset page_offset = alloc_page(hf);
     if (verbose){
         cout << "Offset of first page allocated should be 1: " << page_offset << endl;
     }
     page_offset = alloc_page(hf);
     if (verbose){
         cout << "Offset of second page allocated should be 2: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of third page allocated should be 3: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of fourth page allocated should be 4: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of fifth page allocated should be 6: " << page_offset << endl;
    }



    delete hf;
    fclose(file_ptr);
	return EXIT_SUCCESS;
}
