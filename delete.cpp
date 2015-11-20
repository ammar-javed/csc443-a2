#include <string>
#include "lib.cpp"

int main(int argc, char** argv) {
    if(argc < 4) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "delete <heapfile> <record_id: page_id:slot_number> <page_size> [-v]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
       verbose = true;
    }

    RecordID *record_ID = new RecordID();
 
    // Parse the record_id
    string record_id_to_parse= argv[2];
    size_t pos_delim = record_id_to_parse.find(DELIM);

    if ( pos_delim == string::npos || pos_delim == 0 || pos_delim == record_id_to_parse.size()-1 ) {
        cout << "Error: please use the following format for <record_id>" << endl;
        cout << "<page_id>:<slot_number>" << endl;
        return EXIT_FAILURE;
    } 

    record_ID->page_id = atoi(record_id_to_parse.substr(0, pos_delim).c_str());
    if (verbose)
        cout << "Page ID: " << record_ID->page_id << endl;

    record_ID->slot = atoi(record_id_to_parse.substr(pos_delim+1).c_str());
    if (verbose)
        cout << "Slot: " << record_ID->slot << endl;

    Page* page;

    // Init heap file
    FILE *file_ptr = fopen(argv[1], "r+");
    int page_size = atoi(argv[3]);

    Heapfile *heapfile = new Heapfile();
    init_existing_heapfile(heapfile, page_size, file_ptr);
    if (verbose){
        cout << "Page size of heapfile: " << heapfile->page_size << endl;
        cout << "Last directory offset: " << heapfile->last_directory_offset << endl;
    }

    if (!validate_page_id(heapfile, record_ID->page_id)){
        cout << "The record in page " << record_ID->page_id;
        cout << " at slot " << record_ID->slot;
        cout << " is invalid." << endl;
        return EXIT_FAILURE; 
    }
    // Get the offset of the page
    Offset page_offset = get_page_offset_from_pid(record_ID->page_id, heapfile->page_size);
    if (verbose) {
        cout << "Page offset: " << page_offset << endl;
    }

    // Init a page
    init_page(&page);

    // Read the page from the disk into the struct page
    read_page( heapfile, page_offset, page );

    if (record_ID->slot > page->total_slots - 1) {
        cout << "The record in page " << record_ID->page_id;
        cout << " at slot " << record_ID->slot;
        cout << " is invalid." << endl;
        return EXIT_SUCCESS;
    }
   
    Record *record_to_delete = page->records->at(record_ID->slot);

    if (record_to_delete->size()){
        delete_record_at_slot(record_to_delete);

        write_page(page, heapfile, page_offset);
        update_directory_entry_of_pageID(heapfile->file_ptr, record_ID->page_id, heapfile->page_size, 1);

    } else {
        cout << "The record in page " << record_ID->page_id;
        cout << " at slot " << record_ID->slot;
        cout << " does not exist." << endl;
    }

    fclose(heapfile->file_ptr);
    free_page(&page, page->total_slots);
    delete record_ID;
    return EXIT_SUCCESS;
}
