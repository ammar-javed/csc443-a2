#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
#include "wflp.hpp"

// Global verbose setting
bool verbose = false;

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record){
    int size = 0;
    for( attr = (*record).begin(); attr != (*record).end(); ++attr) {
        size += strlen(*attr);
    }
    return size;    
}

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf) {
    for( attr = (*record).begin(); attr != (*record).end(); ++attr) {
        strcat((char *)buf, (*attr));
    } 
}

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record) {
    char* attribute;
    int read = 0;
    
    while (read != size) {
        attribute = new char[11];
        attribute[10] = '\0';
        memcpy( attribute, &((char *) buf)[read], ATTRIBUTE_SIZE );
        read += ATTRIBUTE_SIZE;
        (*record).push_back(attribute);
    }
}
 
/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page **page, int page_size, int slot_size){
    *page = new Page;
    (*page)->page_size = page_size;
    (*page)->slot_size = slot_size;
    (*page)->slots_used = 0;
    (*page)->total_slots = fixed_len_page_capacity(*page);
    (*page)->records = new vector<Record*>;
  
   
    for (int i = 0; i < (*page)->total_slots; i++) {
        (*page)->records->push_back( new Record(NUM_ATTRIBUTES, "\0")); 
    }	
}
 
/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page) {
    return page->page_size / page->slot_size;
}

/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page) {
    return page->total_slots - page->slots_used; 
}
 
/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r) {
    if (fixed_len_page_freeslots(page) <= 0){
	return -1; 	// No more free slots available
    }
    
    // Assuming that first slot available is at index page->slots_used
    return page->slots_used;
}
 
/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){
    
    vector<Record*> records = *(page->records);
    records[slot] = r;
}
 
/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    r = (*(page->records))[slot];   
}

/**
 * Run unit tests of functions
 *
 */
void run_tests(int page_size, int slot_size) {

    if (verbose) {
		cout << "\n== Testing fixed_len_sizeof ==" << endl;
    }

    Record testRec (10, "0123456789");
    int size = fixed_len_sizeof(&testRec);
    if (verbose)
      cout << "The size of testRec(should be 100): " << size << endl;

    if (verbose) {
		cout << "\n== Testing fixed_len_write ==" << endl;
        cout << "The buffer should contain 10 repetitions of 0123456789" << endl;
    }

    char buf[100] = {'\0'};
    fixed_len_write(&testRec, buf);
    if (verbose)
      cout << "Contents of the serialized buffer: " << buf << endl;

    if (verbose) {
		cout << "\n== Testing fixed_len_write ==" << endl;
        cout << "Size should be 50 bytes" << endl;
    }
    Record* testReadBuf = new Record ;
    fixed_len_read(buf, 50, testReadBuf);
    if (verbose)
      cout << "Size of record: " << fixed_len_sizeof(testReadBuf) << endl;
    delete testReadBuf; 

    if (verbose) {
		cout << "\n== Testing init_fixed_len_page ==" <<endl;
        cout << "We will be initializing a new page and its internal directory." << endl;
    }
    Page* page;
    init_fixed_len_page(&page, page_size, slot_size);

    if (verbose) {
       cout << "\n==New Page==" << endl;
       cout << "page_size: " << page->page_size << endl;
       cout << "slot_size: " << page->slot_size << endl;
       cout << "slots_used: " << page->slots_used << endl;
       cout << "total_slots: " << page->total_slots << endl;
       cout << "records: " << page->records->size() << endl;

       cout << "    == Testing First Record ==" << endl;
    }

    Record *first_record = (*(page->records))[0];
    
    if (verbose) {
        cout << "    Size of first record: " << first_record->size() << endl;
        cout << "\n    Inserting 'Yoo!' into first record..." << endl;
    }
    
    (*first_record)[0] = "Yoo!";
    
    if (verbose) {
        cout << "    First attribute in First record now points to: " << first_record->front() << endl;
        cout << "    Size of First record variable is: " << fixed_len_sizeof(first_record) << endl;
        cout << "    Size of First record through registry is: ";
        cout << fixed_len_sizeof(page->records->operator[](0)) << endl;
    }

    for (int i = 0; i < page->total_slots; i++) {
        if (verbose)
			cout << "Freeing record: " << i+1 << endl;
        delete page->records->at(i);
    }

    if (verbose)
        cout << "Freeing Directory" << endl;
    delete page->records;

    if (verbose)
        cout << "Freeing Page" << endl;
    delete page;
}

int main(int argc, char** argv) {
    
    if(argc < 4) {
	    cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "write_fixed_len_pages <csv_file> <page_file> <page_size> [-v verbose]" << endl;
	    return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
       verbose = true;
    }	

	run_tests(atoi(argv[3]), NUM_ATTRIBUTES * ATTRIBUTE_SIZE );

    return EXIT_SUCCESS;

}

