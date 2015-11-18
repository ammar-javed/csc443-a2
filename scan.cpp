#include "lib.cpp"

int main(int argc, char** argv) {
    if(argc < 3) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "scan <heapfile> <page_size>" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 4 && strcmp(argv[3], "-v") == 0) {
       verbose = true;
    }

    FILE *file_ptr = fopen(argv[1], "r+");
    int page_size = atoi(argv[2]);
    Heapfile *heapfile = new Heapfile();
    init_existing_heapfile(heapfile, page_size, file_ptr);
	if (verbose){
        cout << "Page size of heapfile: " << heapfile->page_size << endl;
        cout << "Last directory offset: " << heapfile->last_directory_offset << endl;
    }



    Offset current_dir_offset = 0;
    Offset page_offset;
    Page *p;

    int total_directory_entries = get_total_directory_entries(page_size);

    Offset next_dir = 1;
    while (current_dir_offset != 0 || next_dir != 0){

    	if (verbose){
	        cout << "Reading Directory: " << current_dir_offset << endl;
    	}

    	fseek(file_ptr, page_size * current_dir_offset, SEEK_SET);
    	fread(&next_dir, OFFSET_SIZE, 1, file_ptr);
    	fpos_t dir_entry_start_pos;
    	fgetpos(heapfile->file_ptr, &dir_entry_start_pos);

    	for (int slot = 0; slot < total_directory_entries; slot++){
    		if (verbose){
	        	cout << "Reading metadata in directory entry slot: " << slot << endl;
    		}
    		fsetpos(file_ptr, &dir_entry_start_pos);
    		fseek(file_ptr, slot * sizeof(DirectoryEntry), SEEK_CUR);
	    	if (verbose){
        		cout << "Start position of the current directory entry: " << ftell(file_ptr) << endl;
    		}
    		
    		fread(&page_offset, OFFSET_SIZE, 1, file_ptr);
    		if (verbose){
        		cout << "Page offset read : " << page_offset << endl;
    		}

    		if (page_offset != 0){
    			if (verbose){
	        		cout << "    The slot has a page with content. So outputting data.... " << endl;
    			}
    			
    			// This page is not empty, so initialize page struct,
    			// read the data in to the page struct and print it out.
    			init_page(&p);
    			if (verbose){
        			cout << "Page Initialized: " << endl;
    			}
    			read_page(heapfile, page_offset, p);
    			if (verbose){
	        		cout << "    Initialized page's page size " << p->page_size << endl;
	        		cout << "    Initialized page's total slots " << p->total_slots << endl;
	        		cout << "    Initialized page's used slots " << p->slots_used<< endl;
	        		cout << "    Initialized page's slot size " << p->slot_size << endl;
    			}

    			dump_page_records(p);
    		}
    	}
    	current_dir_offset = next_dir;
    }

    return EXIT_SUCCESS;
}