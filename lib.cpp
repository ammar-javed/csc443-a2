#include <vector>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "string.h"
#include "cmnhdr.hpp"


/*****************************************************************
 *
 * HEAP INITIALIZATION
 *
 *****************************************************************/

/*
 * Initalize a heapfile to use the file and page size given.
 * precondition: the file is already opened with r+ or w+ permissions
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){

    heapfile->page_size = page_size;
    heapfile->file_ptr = file;
    heapfile->last_directory_offset = 0;

    fseek(file, 0, SEEK_SET);

    append_empty_directory_to_file(file, page_size);

    // Set the file pointer to the beginning of the page
    fseek(file, 0, SEEK_SET);
}

/*****************************************************************
 *
 * HEAP HELPERS
 *
 *****************************************************************/

/*
 * Returns the total number of directory entries in a directory
 */
int get_total_directory_entries(int page_size){

    // In a Directory, the first 8 bytes is the offset of the next
    // directory. The rest of the page is divided in to Directory
    // Entries
    return (page_size - OFFSET_SIZE)/sizeof(DirectoryEntry);
}

/**
 * Appends an empty directory with initialized directory entries
 * to the end of the file
 */
void append_empty_directory_to_file(FILE *file, int page_size){

    // The first 8 bytes in the file show indicate what the next
    // page offset is. Since there is no next directory yet, we
    // set it to be 0
    Offset next_directory_offset = 0;
    fwrite(&next_directory_offset, OFFSET_SIZE, 1, file);

    // Next we will initialize all the directory entries
    // A directory entry will consist of a list of DirectoryEntries
    int total_dir_entries = get_total_directory_entries(page_size);
    Offset initial_dirEntry_offset = 0;
    Offset initial_dirEntry_size = 0;
    for (int i=0; i < total_dir_entries; i++){
        fwrite(&initial_dirEntry_offset, OFFSET_SIZE, 1, file);
        fwrite(&initial_dirEntry_size, OFFSET_SIZE, 1, file);
    }

    int padding = page_size - (OFFSET_SIZE * total_dir_entries * 2) - OFFSET_SIZE;
    char *padding_str = new char[padding];
    memset(padding_str, '0', padding);
    fwrite(padding_str, padding, 1, file);

    delete[] padding_str;

}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){

    // The tail of the directory linked list
    Offset dir_offset = heapfile->last_directory_offset;

    // Forward the file to the directory and read the directory entries
    fseek(heapfile->file_ptr, dir_offset*heapfile->page_size, SEEK_SET);
    fpos_t directory_pos;
    fgetpos(heapfile->file_ptr, &directory_pos);

    // Skip the pointer reserved for the next directory
    fseek(heapfile->file_ptr, sizeof(Offset), SEEK_CUR);

    int total_dir_entries = get_total_directory_entries(heapfile->page_size);

    Offset page_offset;
    fread(&page_offset, OFFSET_SIZE, 1, heapfile->file_ptr);

    for (int i = 0; i < total_dir_entries; i++){
        if (page_offset == 0){
            // Create a new page and write its initialized data in to
            // the end of the file.
            Offset new_page_offset = append_empty_page_to_file(
                         heapfile->file_ptr, heapfile->page_size);

            // We don't need to cahnge the directory_pos because we are
            // always starting with the very last directory and creating
            // a new dir if there's no space
            fsetpos(heapfile->file_ptr, &directory_pos);

            // Skip to the current directory entry
            fseek(heapfile->file_ptr, OFFSET_SIZE + (i * OFFSET_SIZE * 2), SEEK_CUR);

            // Write the new page offset and set free space
            fwrite(&new_page_offset, OFFSET_SIZE, 1, heapfile->file_ptr);
            fwrite(&heapfile->page_size, OFFSET_SIZE, 1, heapfile->file_ptr);

            return new_page_offset;
        }
        // Skip the free size offset and read in the next page_offset
        fseek(heapfile->file_ptr, OFFSET_SIZE, SEEK_CUR);
        fread(&page_offset, OFFSET_SIZE, 1, heapfile->file_ptr);
    }

    // This means the last directory is full so we allocate a new one.
    // allocate_new_directory
    fseek(heapfile->file_ptr, 0, SEEK_END);
    Offset new_dir_offset = ftell(heapfile->file_ptr)/ heapfile->page_size;
    append_empty_directory_to_file(heapfile->file_ptr, heapfile->page_size);

    // Nowe we also append a new page. Since we know that the directory is
    // empty, there is no need to traverse through directory entries.
    Offset new_page_offset = append_empty_page_to_file(
                    heapfile->file_ptr, heapfile->page_size);

    // To the beginning of the new directory
    fseek(heapfile->file_ptr, new_dir_offset*heapfile->page_size, SEEK_SET);
    // Skip the next directory pointer
    fseek(heapfile->file_ptr, OFFSET_SIZE, SEEK_CUR);

    // Write to the first directory entry
    fwrite(&new_page_offset, OFFSET_SIZE, 1, heapfile->file_ptr);
    fwrite(&heapfile->page_size, OFFSET_SIZE, 1, heapfile->file_ptr);

    // Forward the file to the old directory and link the new directory
    fseek(heapfile->file_ptr, dir_offset*heapfile->page_size, SEEK_SET);
    fwrite(&new_dir_offset, OFFSET_SIZE, 1, heapfile->file_ptr);

    // Set the last_directory_offset of heapfile to point to the new directory.
    heapfile->last_directory_offset = new_dir_offset;
    return new_page_offset;
}

/*
 * Adding on a page block of page size to the end of file
 */
Offset append_empty_page_to_file(FILE *file, int page_size){

    fseek(file, 0, SEEK_END);

    Offset new_page_offset = ftell(file)/ page_size;

    char *initializer = new char[page_size];
    memset(initializer, '0', page_size);
    size_t out= fwrite(initializer, 1, page_size, file);


    delete[] initializer;
    return new_page_offset;

}

/*****************************************************************
 *
 * PAGE INITIALIZATION
 *
 *****************************************************************/

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

/*****************************************************************
 *
 * RECORD SERIALIZATION AND DESERIALIZATION
 *
 *****************************************************************/

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

/*****************************************************************
 *
 * WRITTING
 *
 *****************************************************************/

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
 * Write the record at the end of page this function assumes you 
 * will ALWAYS be able to fit a record at the end.
 */
void append_record(Page *page, Record *r){

    page->records->push_back(r);
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){

    int page_size = heapfile->page_size;

    // Go to the directory entry to insert the metadata
    int total_dir_entries =  get_total_directory_entries(page_size);
    Offset directory_num = pid / total_dir_entries;
    fseek(heapfile->file_ptr, page_size * (total_dir_entries + 1) * directory_num, SEEK_SET);

    Offset directory_offset = (total_dir_entries + 1) * directory_num;
    Offset directory_entry_slot = pid - directory_offset - 1;

    // Skip the next directory pointer and the directory entries before the entry
    // of the page
    fseek(heapfile->file_ptr, (directory_entry_slot * 2) + OFFSET_SIZE, SEEK_CUR);


    // Write the page offset
    fwrite(&pid, OFFSET_SIZE, 1, heapfile->file_ptr);
    Offset free_space = page->page_size - (page->slots_used * page->slot_size); 
    fwrite(&free_space, OFFSET_SIZE, 1, heapfile->file_ptr);
    

    // TODO: Assume that this buffer contains all the data from the page file
    char *buf = new char[heapfile->page_size];
    memset(buf, '0', heapfile->page_size);

    for (int s = 0; s < page->slots_used; s++) {
        fixed_len_write((*(page->records))[s], buf);
    }

    fseek(heapfile->file_ptr, pid * heapfile->page_size, SEEK_SET);
    fwrite(buf, page_size, 1, heapfile->file_ptr);

    delete[] buf;
}

/*****************************************************************
 *
 * READING
 *
 *****************************************************************/

/*
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    r = (*(page->records))[slot];
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){

    int64 page_size = heapfile->page_size;

    // Go to the directory entry to extract the metadata
    int total_dir_entries =  get_total_directory_entries(page_size);
    Offset directory_num = pid / total_dir_entries;
    fseek(heapfile->file_ptr, page_size * (total_dir_entries + 1) * directory_num, SEEK_SET);

    Offset directory_offset = (total_dir_entries + 1) * directory_num;
    Offset directory_entry_offset = pid - directory_offset - 1;

    // Skip the next directory pointer and the directory entries before the entry
    // of the page and the page_offset of the current directory entry
    fseek(heapfile->file_ptr, (directory_entry_offset * 2) + (2 * OFFSET_SIZE), SEEK_CUR);

    Offset free_space;
    fread(&free_space, OFFSET_SIZE, 1, heapfile->file_ptr);

    // from the heapfile, go to the pageID and then
    // read it's contents in to a char buf of page_size
    // and then write the buffer to the page.
    fseek(heapfile->file_ptr, pid * heapfile->page_size, SEEK_SET);

    // Calculate slot size
    int64 record_size = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;

    // Optimal space to read in
    int64 used_space = page_size - free_space;
    char *buf = new char[used_space];

    fread(buf, sizeof(char), used_space, heapfile->file_ptr);

    // For each record, deserialize it and put it in the page
    for (int i = 0; i < used_space; i += record_size) {
        Record *record = new Record();

        fixed_len_read(&(buf[i]), record_size, record);

        append_record(page, record);
    }
    
    // Update page meta data
    page->total_slots = page_size / record_size;
    page->slots_used = used_space / record_size;
    page->page_size = page_size;
    page->slot_size = record_size;
    
}

/*****************************************************************
 *
 * HELPERS
 *
 *****************************************************************/

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

/*****************************************************************
 *
 * CLEAN UP
 *
 *****************************************************************/

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

