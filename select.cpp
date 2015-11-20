#include "lib.cpp"

int print_selected_records(Page *page, char *start, char *end, int attribute_id){
	vector<Record*> records = (*(page->records));
	int atts = 0;
    for (int i=0; i < records.size(); i++) {
        if (records[i]->size() != 0) {
        	const char *attr_at_index = (*(records[i]))[attribute_id];
            if (strcmp(attr_at_index, start) >= 0 && strcmp(attr_at_index, end) <= 0){
            	char *substring = new char[6];
				memcpy( substring, attr_at_index, 5);
				substring[5] = '\0';
				cout << substring << endl;

				delete[] substring;
				atts ++;
            }
        }
    }

    return atts;
}

int main(int argc, char** argv) {
    if(argc < 6) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "select <<heapfile> <attribute_id> <start> <end> <page_size>" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 7 && strcmp(argv[6], "-v") == 0) {
       verbose = true;
    }

    // Convert all the arguments
    FILE *file_ptr= fopen(argv[1], "r+");
    if (file_ptr == NULL){
    	cout << "ERROR: File " << argv[1] << " cannot be opened." << endl;
    	return EXIT_FAILURE;
    }

    // Files are open, begin timing
    struct timeval start, end, result;
    gettimeofday(&start, NULL);

    int attribute_id =  atoi(argv[2]);
    char *start_char = argv[3];
    char *end_char = argv[4];
    int page_size =  atoi(argv[5]);

    // Initialize and read in the heapfile.
    Heapfile *heapfile = new Heapfile();

    init_existing_heapfile(heapfile, page_size, file_ptr);

	if (verbose){
        cout << "Page size of heapfile: " << heapfile->page_size << endl;
        cout << "Last directory offset: " << heapfile->last_directory_offset << endl;
    }

    Offset current_dir_offset = 0;
    Offset page_offset;
    Page *page;

    int total_directory_entries = get_total_directory_entries(page_size);

    int selected_atts = 0;
    Offset next_dir = 1;
    while (current_dir_offset != 0 || next_dir != 0){

    	if (verbose){
	        cout << "Reading Directory: " << current_dir_offset << endl;
    	}

    	forward_file_ptr_to_start_of_directory(file_ptr, current_dir_offset, page_size);
    	fread(&next_dir, OFFSET_SIZE, 1, file_ptr);
    	
    	// Save the position of the current directory entries.
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
	        		cout << "The slot has a page with content. Reading data in to page struct... " << endl;
    			}
    			
    			// This page is not empty, so initialize page struct,
    			// read the data in to the page struct and print it out.
    			init_page(&page);
    			read_page(heapfile, page_offset, page);
    			if (verbose){
	        		cout << "    Initialized page's page size " << page->page_size << endl;
	        		cout << "    Initialized page's total slots " << page->total_slots << endl;
	        		cout << "    Initialized page's used slots " << page->slots_used<< endl;
	        		cout << "    Initialized page's slot size " << page->slot_size << endl;
    			}
                
    			int read_att = print_selected_records(page, start_char, end_char, attribute_id);
    			selected_atts += read_att;
    		} else {
    			if (verbose){
    				cout << "    Done traversing through all the directory entries." << endl;
    			}
    			break;
    		}
    	}
    	current_dir_offset = next_dir;
    }


    //Finished writing last page
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);
    double total_millisec = (result.tv_sec * 1000.0) + (result.tv_usec / 1000.0);

    // Outputting the relevant metrics as per assignment requirements
    cout << endl << "NUMBER OF ATTRIBUTES SELECTED: " << selected_atts << endl;
    cout << "PAGE SIZE: " << page_size << endl;
    cout << "Time: " << total_millisec << " MS" << endl;


    // Memory Cleaning up
    free_page(&page, page->slots_used);
    delete heapfile;
    fclose(file_ptr);
 }