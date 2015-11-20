#include "lib.cpp"

int print_out_selected_attributes(Page *page, char *start, char *end, int attribute_id){
    vector<Record*> records = (*(page->records));
    int atts = 0;
    for (int i=0; i < records.size(); i++) {
        if (records[i]->size() != 0) {
            for ( attr = records[i]->begin(); attr != records[i]->end(); ++attr) {
                if (strcmp(*attr, start) >= 0 && strcmp(*attr, end) <= 0){
                    char *substring = new char[6];
                    memcpy( substring, *attr, 5);
                    substring[5] = '\0';
                    cout << substring << endl;
                    delete[] substring;
                }

                atts++;
            }
        }
    }

    return atts;
}

int main(int argc, char** argv) {
    if(argc < 6) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "select2 <colstore_name> <attribute_id> <start> <end> <page_size>[-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 7 && strcmp(argv[6], "-v") == 0) {
       verbose = true;
    }

    // Setting up heap file pointer
    FILE *file_ptr = fopen(argv[1], "r+");
    int64 attribute_id = atoi(argv[2]);
    char *start_str = argv[3];
    char *end_str = argv[4];
    int64 page_size = atoi(argv[5]);

    // Structs for timing
    struct timeval start, end, result;

    // Files are open, begin timing
    gettimeofday(&start, NULL);

    // Initialize and read in the heapfile.
    Heapfile *heapfile = new Heapfile();

    init_existing_heapfile(heapfile, page_size, file_ptr, 1);

    // Find out how many blocks of 100 pages is written
    int64 num_blocks;
    fseek(file_ptr, 0, SEEK_SET);
    fread(&num_blocks, OFFSET_SIZE, 1, file_ptr);

    int64 curr_block = 0;
    Page *page;

    while(curr_block < num_blocks){

        init_page(&page);
        Offset page_offset = (curr_block * NUM_ATTRIBUTES) + attribute_id + 1;
        read_page(heapfile, page_offset, page);

        print_out_selected_attributes(page, start_str, end_str, attribute_id);

        curr_block ++; 

    }

    //Finished writing last page
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);
    double total_millisec = (result.tv_sec * 1000.0) + (result.tv_usec / 1000.0);

    if (verbose) {
        // Outputting the relevant metrics 
        cout << "Time: " << total_millisec << " MS" << endl;
    }

    // Memory Cleaning up
    free_page(&page, page->slots_used);
    delete heapfile;
    fclose(file_ptr);
}


