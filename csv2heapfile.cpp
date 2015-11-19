#include "lib.cpp"


int main(int argc, char** argv) {
    if(argc < 4) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "csv2heapfile <csv_file> <heapfile> <page_size> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
       verbose = true;
    }

    // Setting up heap file pointer
    FILE *file_ptr = fopen(argv[2], "w+");

    // Page Size
    int64 page_size = atoi(argv[3]);

    int64 record_size = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;

    int64 record_size_csv = (NUM_ATTRIBUTES * ATTRIBUTE_SIZE) + (NUM_ATTRIBUTES);

    // Total Records and page count
    int64 total_records = 0;
    int64 total_pages = 0;

    // Structs for timing
    struct timeval start, end, result;

    // Page pointer
    Page* page;

    // Initialize it for first use
    init_page(&page);
    page->page_size = page_size;
    page->slot_size = record_size;
    page->total_slots = page_size / record_size;

    // Will be reading into this buffer from the csv file
    char* buffer = new char[record_size_csv * page->total_slots];
    memset(buffer, '\0', record_size_csv * page->total_slots);

    // Init the heapfile
    Heapfile *hf = new Heapfile();

    init_heapfile(hf, page_size, file_ptr);

    // Opening the CSV file stream
    ifstream csv_in(argv[1]);

    // Files are open, begin timing
    gettimeofday(&start, NULL);

    if ( csv_in.is_open() ) {
        // Initial read
        csv_in.read(buffer, record_size_csv * page->total_slots);

        while ( csv_in.gcount() ) {
            // For every 1000 byte record we read, create a Record
            // and add it to the page
            for (int i = 0; i < csv_in.gcount(); i += record_size_csv) {
                //New Record
                Record *record = new Record;

                fixed_len_read(&(buffer[i]), record_size_csv, record, 1); 

                append_record(page, record);
                page->slots_used++;
                total_records++;
            }

            // If we didn't read in enough records to fill up the page
            // fill the rest of the slots with empty records
            if (page->records->size() != page->total_slots) {
                int64 left = page->total_slots - page->records->size();
                for (int j = 0; j < left; j++) {
                    append_record(page, new Record());
                }
            }
 
            // Allocate a page in the heapfile
            Offset page_offset = alloc_page(hf);

            // Write the page into the heapfile at the appropriate offset
            write_page(page, hf, page_offset);
            total_pages++;

            // Free this page, and init a new one
            free_page(&page, page->slots_used);
            init_page(&page);
            page->page_size = page_size;
            page->slot_size = record_size;
            page->total_slots = page_size / record_size;

            memset(buffer, '\0', record_size_csv * page->total_slots);

            // Try reading again
            csv_in.read(buffer, record_size_csv * page->total_slots);
        }

    }

    //Finished writing last page
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);
    double total_millisec = (result.tv_sec * 1000.0) + (result.tv_usec / 1000.0);

    if (verbose) {
        // Outputting the relevant metrics 
        cout << "NUMBER OF RECORDS: " << total_records << endl;
        cout << "NUMBER OF PAGES: " << total_pages << endl;
        cout << "Time: " << total_millisec << " MS" << endl;
    }

    // Memory Cleaning up
    free_page(&page, page->slots_used);
    delete[] buffer;
    delete hf;
    fclose(file_ptr);

    return EXIT_SUCCESS;
}
