#include <vector>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "string.h"
#include "cmnhdr.hpp"
/*
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page **page, int page_size, int slot_size){
    *page = new Page();
    (*page)->page_size = page_size;
    (*page)->slot_size = slot_size;
    (*page)->slots_used = 0;
    (*page)->total_slots = fixed_len_page_capacity(*page);
    (*page)->records = new vector<Record*>;


    for (int i = 0; i < (*page)->total_slots; i++) {
        (*page)->records->push_back( new Record(NUM_ATTRIBUTES, "0000000000"));
    }
}

/*
 * Initializes a page
 */
void init_page(Page **page){
    *page = new Page();
    (*page)->records = new vector<Record*>;
}

/*
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record){
    int size = 0;
    for( attr = (*record).begin(); attr != (*record).end(); ++attr) {
        size += strlen(*attr);
    }
    return size;
}

/*
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, char *buf) {
    for( attr = (*record).begin(); attr != (*record).end(); ++attr) {
        strcat(buf, (*attr));
    }
}

/*
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`. If we are reading from
 * a CSV file instead of a page, account for the ',' read
 * in and ignored.
 */
void fixed_len_read(char *buf, int size, Record *record, int csv) {
    char* attribute;
    int read = 0;

    while (read < size) {
        attribute = new char[11];
        memcpy( attribute, &((char *) buf)[read], ATTRIBUTE_SIZE );
        attribute[10] = '\0';
        read += ATTRIBUTE_SIZE;
        if (csv)
            read += sizeof(char);
        (*record).push_back(attribute);
    }
}

/*
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page) {
    return page->page_size / page->slot_size;
}

/*
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page) {
    return page->total_slots - page->slots_used;
}

/*
 * Returns:
 *   record slot offset if space available,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r) {
    if (fixed_len_page_freeslots(page) <= 0){
    return -1;  // No more free slots available
    }

    // Assuming that first slot available is at index page->slots_used
    return page->slots_used;
}

/*
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){

    vector<Record*>* records = page->records;
    delete (*records)[slot];
    (*records)[slot] = r;
    page->slots_used++;

}

/*
 * Write the record at the end of page
 * this function assumes you will ALWAYS be
 * able to fit a record at the end.
 */
void append_record(Page *page, Record *r){

    page->records->push_back(r);
}

/*
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    r = (*(page->records))[slot];
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

/*
 * Delete entire page, including all vectors initialized
 */
void free_page(Page **page, int64_t slots) {
    for (int i = 0; i < slots; i++) {
        (*page)->records->at(i)->clear();
        delete (*page)->records->at(i);
    }

    delete (*page)->records;

    delete (*page);
}

