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
    int size = 4000;
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

    fclose(file_ptr);

    if (verbose) {
        cout << "\n== Testing init_existing_heapfile" << endl;
    }

    file_ptr = fopen(argv[1], "r+");

    hf = new Heapfile();
    init_existing_heapfile(hf, size, file_ptr);
    if (verbose){
        cout << "Page size of heapfile should be 4000: " << hf->page_size << endl;
        cout << "Contents of the page size: " << hf->file_ptr << endl;
        cout << "Last directory offset should be 0: " << hf->last_directory_offset << endl;
    }

    if (verbose) {
        cout << "\n== Testing read_page" << endl;
    }

    Page *p;
    init_page(&p);
    read_page(hf, 1, p);

    if (verbose) {
        cout << "Total slots for empty page: " << p->total_slots << endl;
        cout << "Slots used for empty page: " << p->slots_used << endl;
        cout << "Page size for empty page: " << p->page_size << endl;
        cout << "Slot size for empty page: " << p->slot_size << endl;
    }

    delete hf;
    fclose(file_ptr);
	return EXIT_SUCCESS;
}
