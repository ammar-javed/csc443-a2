
#include "lib.cpp"

int main(int argc, char** argv) {
    if(argc < 4) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "insert  <heapfile> <csv_file> <page_size> -v" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
       verbose = true;
    }

    FILE *file_ptr = fopen(argv[1], "r+");
    int page_size = atoi(argv[3]);

    // Opening the CSV file stream
    ifstream csv_in(argv[2]);
    if (!csv_in.is_open() ) {
        cout << "ERROR: The csv file cannot be opened." << endl;
        return EXIT_FAILURE;
    }

    // Initialize heapfile and page
    Heapfile *heapfile = new Heapfile();
    init_existing_heapfile(heapfile, page_size, file_ptr);
    Page *page;
    if (verbose){
        cout << "Page size of heapfile: " << heapfile->page_size << endl;
        cout << "Last directory offset: " << heapfile->last_directory_offset << endl;
    }

    int64 record_size_csv = (NUM_ATTRIBUTES * ATTRIBUTE_SIZE) + (NUM_ATTRIBUTES);
    int64 record_size = (NUM_ATTRIBUTES * ATTRIBUTE_SIZE);
    int total_directory_entries = get_total_directory_entries(page_size);

    Offset current_dir_offset = 0;
    Offset page_offset;
    int64 free_space;
    Record *record;

    // Will be reading into this buffer from the csv file
    char* buffer = new char[record_size_csv];
    memset(buffer, '\0', record_size_csv);

    // Initial read
    csv_in.read(buffer, record_size_csv);

    Offset next_dir = 1;
    while (csv_in.gcount()) {
    
        // For every 1100 byte record we read, create a Record
        // Loop #2
        int i = 0;
        while ((current_dir_offset != 0 || next_dir != 0)){

            if (verbose){
                cout << "Reading Directory: " << current_dir_offset << endl;
            }

            fseek(file_ptr, page_size * current_dir_offset, SEEK_SET);
            fread(&next_dir, OFFSET_SIZE, 1, file_ptr);
            fpos_t dir_entry_start_pos;
            fgetpos(heapfile->file_ptr, &dir_entry_start_pos);

            // Check in all directory entries to see if there is a free page
            for(int slot = 0; (slot < total_directory_entries && csv_in.gcount()); slot++){
                if (verbose){
                    cout << "Reading metadata in directory entry slot: " << slot << endl;
                }

                fsetpos(file_ptr, &dir_entry_start_pos);
                fseek(file_ptr, slot * sizeof(DirectoryEntry), SEEK_CUR);
                if (verbose){
                    cout << "Start position of the current directory entry: " << ftell(file_ptr) << endl;
                }
                
                fread(&page_offset, OFFSET_SIZE, 1, file_ptr);
                fread(&free_space, OFFSET_SIZE, 1, file_ptr);

                if (verbose){
                    cout << "Page offset read : " << page_offset << endl;
                    cout << "Page free space : " << free_space << endl;
                }

                // In the case we have to grow the heap
                if (page_offset == 0){
                    page_offset = alloc_page(heapfile);
                    free_space = page_size;
                }
                
                if (free_space/record_size > 0){ 
                    
                    // Get the data in to a page struct to determine free slots
                    init_page(&page);
                    if (verbose){
                        cout << "Page Initialized: " << endl;
                    }
                    read_page(heapfile, page_offset, page);
                    if (verbose){
                        cout << "    Initialized page's page size " << page->page_size << endl;
                        cout << "    Initialized page's total slots " << page->total_slots << endl;
                        cout << "    Initialized page's used slots " << page->slots_used<< endl;
                        cout << "    Initialized page's slot size " << page->slot_size << endl;
                    }

                    while (fixed_len_page_freeslots(page) > 0  && csv_in.gcount()){

                        //New Record with the csv information in it
                        record = new Record();
                        fixed_len_read(buffer, record_size_csv, record, 1);

                        if (verbose){
                            cout << "    Writting record to page... " << endl;
                        }
                        int free_slot = add_fixed_len_page(page, record);

                        write_fixed_len_page(page, free_slot, record);                        
                        
                        if (verbose){
                            cout << "        Page offset of page with free slot" << page_offset << endl;
                            cout << "        Free slot : " << free_slot << endl;
                            cout << "        New used slots for page " << page->slots_used<< endl;
                        }
                        
                        // Read again from the file
                        csv_in.read(buffer, record_size_csv);                            
                    }
                    write_page(page, heapfile, page_offset);

                }
            }
            current_dir_offset = next_dir;
        }

        // All the directory entries are filled up so we need to alloc_page one more time
        // to allocate a  new directory and one more page
        if (current_dir_offset == 0 && next_dir == 0 && csv_in.gcount()){

            if (verbose){
                cout << "insert ** Allocating new directory" << endl;
            }
            page_offset = alloc_page(heapfile);
            current_dir_offset = heapfile->last_directory_offset;
            next_dir = 0;

            if (verbose){
                cout << "        New directory offset" << current_dir_offset << endl;
                cout << "        New page offset " << page_offset << endl;
            }

        }
    }
    return EXIT_SUCCESS;
}
