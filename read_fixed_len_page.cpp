#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
#include "cmnhdr.hpp"
#include <fstream>

// Verbose status
bool verbose = false;

/**
 * Initializes a page 
 */
void init_fixed_len_page(Page **page){
    *page = new Page();
    (*page)->records = new vector<Record*>;
}

/*
 * Print all records in CSV format.
 */
void dump_page_records(Page *page) {
    int atts;
    for (int i=0; i < page->slots_used; i++) {
        atts = 0;
        vector<Record*> records = (*(page->records));
        for ( attr = records[i]->begin(); attr != records[i]->end(); ++attr) {
            cout << (*attr);
            if ( atts != 99 ) {
                cout << ",";
            } else {
                cout << endl;
            }
            atts++;
        }
    }
}

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`. If we are reading from
 * a CSV file instead of a page, account for the ',' read
 * in and ignored.
 */
void fixed_len_read(void *buf, int size, Record *record) {
    char* attribute;
    int read = 0;

    while (read < size) {
        attribute = new char[11];
        memcpy( attribute, &((char *) buf)[read], ATTRIBUTE_SIZE );
        attribute[10] = '\0';
        read += ATTRIBUTE_SIZE;
        (*record).push_back(attribute);
    }
}

/**
 * Write a record into the page
 */
void write_fixed_len_page(Page *page, Record *r){

    page->records->push_back(r);
}

void free_page(Page **page) {
    for (int i = 0; i < (*page)->slots_used; i++) {
        (*page)->records->at(i)->clear();
        delete (*page)->records->at(i);
    }

    delete (*page)->records;

    delete (*page);
}

int main(int argc, char** argv) {

    if(argc < 3) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "read_fixed_len_page <page_file> <page_size> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 4 && strcmp(argv[3], "-v") == 0) {
       verbose = true;
    }

    // Total record read from csv
    int total_records = 0;

    // Size of records in page
    int recSize;

    // Total Number of pages allocated
    int total_pages = 0;

    // Page pointer we will be using
    Page* page;

    // Init Page
    init_fixed_len_page(&page);

    // Input stream
    ifstream binary_input (argv[1]);

    // Records buffer
    char* recordsBuffer;

    if ( binary_input.is_open() ) {
        while ( binary_input.read(reinterpret_cast<char *> (&(page->total_slots)), sizeof(int64_t)) ) {
            binary_input.read(reinterpret_cast<char *> (&(page->slots_used)), sizeof(int64_t));
            binary_input.read(reinterpret_cast<char *> (&(page->page_size)), sizeof(int64_t));
            binary_input.read(reinterpret_cast<char *> (&(page->slot_size)), sizeof(int64_t));

            if (page->page_size != atoi(argv[2])) {
                cout << "Error: Please specify the same page_size used to intially create the page_file." << endl;
                cout << "page_file (" << argv[1] << ") was created using a page_size of ";
                cout << page->page_size << "." << endl;
                return EXIT_FAILURE;
            }

            // init a buffer the size of the valid records stored
            recSize = page->slots_used * page->slot_size;
            recordsBuffer = new char[recSize+1];
            recordsBuffer[recSize] = '\0';
 
            binary_input.read(recordsBuffer, recSize);

            // Format and init records from this buffer
            for (int i = 0; i < recSize; i += (NUM_ATTRIBUTES * ATTRIBUTE_SIZE) ) {
                // New record
                Record *record = new Record;

                fixed_len_read(&(recordsBuffer[i]), page->slot_size, record);

                write_fixed_len_page(page, record);
                
            }

            dump_page_records(page);

            // If we had a partially empty page, then skip ahead to next page
            // else this pointer wont move
            binary_input.seekg( binary_input.tellg() + (page->page_size - recSize) ); 

            free_page(&page);

            delete[] recordsBuffer;

            init_fixed_len_page(&page);

        }
    } else {
        cout << "\n\nERROR: Failed to open page_file." << endl;
        return EXIT_FAILURE;
    }

    free_page(&page);

    return EXIT_SUCCESS;
}

