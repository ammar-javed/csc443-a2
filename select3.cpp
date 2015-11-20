#include "lib.cpp"

int print_out_selected_attributes(Page *page, Page *print_page, char *start, char *end, int attribute_id){
    vector<Record*> records = (*(page->records));
    vector<Record*> printable_records = (*(print_page->records));

    int atts = 0;
    for (int i=0; i < records.size(); i++) {
        if (records[i]->size() != 0) {
            int curr_attr = 0;
            for ( attr = records[i]->begin(); attr != records[i]->end(); ++attr) {
                if (strcmp(*attr, start) >= 0 && strcmp(*attr, end) <= 0){
                    const char *attr_at_index = (*(printable_records[i]))[curr_attr];

                    char *substring = new char[6];
                    memcpy( substring, attr_at_index, 5);
                    substring[5] = '\0';
                    cout << substring << endl;
                    delete[] substring;
                }
                curr_attr ++;
                atts++;
            }
        }
    }

    return atts;
}

int main(int argc, char** argv) {
    if(argc < 7) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "select3 <colstore_name> <attribute_id> <return_attribute_id> <start> <end> <page_size>[-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 8 && strcmp(argv[7], "-v") == 0) {
       verbose = true;
    }

    // Setting up heap file pointer
    FILE *file_ptr = fopen(argv[1], "r+");
    int64 attribute_id = atoi(argv[2]);
    int64 return_attribute_id = atoi(argv[3]);
    char *start_str = argv[4];
    char *end_str = argv[5];
    int64 page_size = atoi(argv[6]);

    // Initialize and read in the heapfile.
    Heapfile *heapfile = new Heapfile();

    init_existing_heapfile(heapfile, page_size, file_ptr, 1);

    // Find out how many blocks of 100 pages is written
    int64 num_blocks;
    fseek(file_ptr, 0, SEEK_SET);
    fread(&num_blocks, OFFSET_SIZE, 1, file_ptr);

    int64 curr_block = 0;
    Page *page;
    Page *print_page;

    while(curr_block < num_blocks){

        init_page(&page);
        Offset page_offset = (curr_block * NUM_ATTRIBUTES) + attribute_id + 1;
        read_page(heapfile, page_offset, page);

        init_page(&print_page);
        Offset print_page_offset = (curr_block * NUM_ATTRIBUTES) + return_attribute_id + 1;
        read_page(heapfile, print_page_offset, print_page);

        print_out_selected_attributes(page, print_page, start_str, end_str, attribute_id);

        curr_block ++; 

    }

    // Memory Cleaning up
    free_page(&page, page->slots_used);
    free_page(&print_page, page->slots_used);
    delete heapfile;
    fclose(file_ptr);
}


