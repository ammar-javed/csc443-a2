#include "lib.cpp"

int main(int argc, char** argv) {
    if(argc < 4) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "csv2heapfile <csv_file> <colstore_name> <page_size> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
       verbose = true;
    }

    // Setting up heap file pointer
    FILE *file_ptr = fopen(argv[2], "w+");
    int64 page_size = atoi(argv[3]);


    // Opening the CSV file stream
    ifstream csv_in(argv[1]);
    if (!csv_in.is_open() ) {
        cout << "ERROR: The csv file cannot be opened." << endl;
        return EXIT_FAILURE;
    }

    int64 record_size_csv = (NUM_ATTRIBUTES * ATTRIBUTE_SIZE) + (NUM_ATTRIBUTES);


    // Initialize heapfile and page
    Heapfile *heapfile = new Heapfile(); 
    init_heapfile(heapfile, page_size, file_ptr);
    // Will be reading into this buffer from the csv file
    //char* buffer = new char[record_size_csv];
    char* buffer = new char[ATTRIBUTE_SIZE + 1];

    //memset(buffer, '\0', record_size_csv);
    memset(buffer, '\0', ATTRIBUTE_SIZE + 1);



    // Initial read
    csv_in.read(buffer, ATTRIBUTE_SIZE + 1);


    int current_block = 0;
    int64 record_offset_buffer = 0;
    while (csv_in.gcount()) {
        for (int i = 0; i < NUM_ATTRIBUTES; i++){
            Offset offs = append_empty_page_to_file(file_ptr, page_size);
        }

        int records_written = 0;
        while(csv_in.gcount() && records_written < (page_size/ATTRIBUTE_SIZE)){

            record_offset_buffer = (current_block * page_size * NUM_ATTRIBUTES) + (records_written * ATTRIBUTE_SIZE) + page_size;

            fseek(file_ptr, record_offset_buffer, SEEK_SET);
            Offset buffer_offset = records_written * ATTRIBUTE_SIZE;

            for (int attr = 0; attr < NUM_ATTRIBUTES ;attr++){
                fwrite(&buffer[0], ATTRIBUTE_SIZE, 1, file_ptr);
                fseek(file_ptr, page_size - ATTRIBUTE_SIZE, SEEK_CUR);
               
                csv_in.read(buffer, ATTRIBUTE_SIZE + 1);
            }
            records_written++;
        }

        current_block ++;
    }

    int64 total_blocks_written = current_block;

    fseek(file_ptr, 0, SEEK_SET);
    fwrite(&total_blocks_written, OFFSET_SIZE, 1, file_ptr);

    delete[] buffer;
    delete heapfile;
    fclose(file_ptr);
    return EXIT_SUCCESS;

}   