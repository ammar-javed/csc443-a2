#include <vector>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "string.h"
#include "cmnhdr.hpp"
#include "math.h"


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

void init_existing_heapfile(Heapfile *heapfile, int page_size, FILE *file){
    heapfile->page_size = page_size;
    heapfile->file_ptr = file;
    fseek(file, 0, SEEK_SET);
    Offset current_dir = 0;
    Offset next_dir;
    fread(&next_dir, OFFSET_SIZE, 1, file);
    while(next_dir != 0){
        fseek(file, page_size * next_dir, SEEK_SET);
        current_dir = next_dir;
        fread(&next_dir, OFFSET_SIZE, 1, file);
    }
    heapfile->last_directory_offset = current_dir;
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

/*
 * Get page offset on disk from a given PID
 */
Offset get_page_offset_from_pid(PageID pid, int page_size){
    
    int total_dir_entries = get_total_directory_entries(page_size);

    return (pid + ceil ((pid + 1.0)/total_dir_entries));

}  

/*
 * Given the page offset, return the directory offset of the directory
 * in which the page's directory entry is stored in 
 */
// 0 2 4 6 8 10 12 13  <-- memory
// - 0 1 - 2  3  -  4 <-- pid
// 0 1 2 3 4  5  6  7 <-- page_offset

Offset get_dir_offset_of_page_offset(Offset page_offset, int page_size){
    int total_dir_entries = get_total_directory_entries(page_size);
    int page_block_size = total_dir_entries + 1;

    int total_dirs_needed = ceil((float)page_offset / page_block_size);
    int dir_num = total_dirs_needed - 1;
    Offset dir_offset = dir_num * page_block_size;
    return dir_offset;
}

/*
 * Get the slot number of the page's directory entry in it's directory
 */
int get_slot_number_of_page_offset(Offset page_offset, int page_size){
    Offset dir_offset = get_dir_offset_of_page_offset(page_offset, page_size);

    int slot = (page_offset - dir_offset) - 1;
    return slot;

}

/*
 * Forward the pointer in the file to the start of the directory 
 */
void forward_file_ptr_to_start_of_directory(FILE *file, Offset dir_offset, int page_size){
    fseek(file, page_size * dir_offset, SEEK_SET);
}

/*
 * Pre-condition, the file pointer has to be at the beginning of the directory
 */
void forward_file_ptr_to_directory_entry_slot_from_directory(FILE *file,int slot){
    fseek(file, (slot * OFFSET_SIZE * 2) + OFFSET_SIZE, SEEK_CUR);
}

/*
 * Forward the pointer in the file to the start of the Directory Entry
 */
void forward_file_ptr_to_directory_entry(FILE *file, Offset dir_offset, int slot, int page_size){
    forward_file_ptr_to_start_of_directory(file, dir_offset, page_size);
    forward_file_ptr_to_directory_entry_slot_from_directory(file, slot);

}

/*
 * Forward the file pointer to the page offset 
 */
void forward_file_ptr_to_page_offset(FILE *file, Offset page_offset, int page_size){
    fseek(file, page_offset * page_size, SEEK_SET);
}

/*
 * Forward the file pointer to the start of the PID
 */
void forward_file_ptr_to_start_of_PID(FILE *file, PageID pid, int page_size){
    Offset page_offset = get_page_offset_from_pid(page_size, pid);
    forward_file_ptr_to_page_offset(file, page_offset, page_size);
}

/*
 * Given pid and a file, modify the directory entry of the page in the heapfile. 
 * If record deleted is 1, then reduce the free space of the directory entry by
 * a page size, if it is 0, incrememnt the free space.
 */
void update_directory_entry_of_pageID(FILE *file, PageID pid, int page_size, int record_deleted){
    

    Offset page_offset = get_page_offset_from_pid(pid, page_size);

    Offset dir_offset = get_dir_offset_of_page_offset(page_offset, page_size);


    int slot = get_slot_number_of_page_offset(page_offset, page_size);

    forward_file_ptr_to_directory_entry(file, dir_offset, slot, page_size);

    int64 free_space;

    // skip the page offset value;
    Offset page_off;
        fread(&page_off, OFFSET_SIZE, 1, file);

    //fseek(file, OFFSET_SIZE, SEEK_CUR);

    fread(&free_space, OFFSET_SIZE, 1, file);

    if (record_deleted){
        free_space = free_space + (NUM_ATTRIBUTES * ATTRIBUTE_SIZE);

    } else {
        free_space = free_space - page_size;

    }

    page_offset = get_page_offset_from_pid(pid, page_size);
    dir_offset = get_dir_offset_of_page_offset(page_offset, page_size);

    fseek(file, dir_offset * page_size, SEEK_SET);
    fseek(file, (slot * OFFSET_SIZE * 2 ) + OFFSET_SIZE, SEEK_CUR);
    fseek(file,  OFFSET_SIZE, SEEK_CUR);

    fwrite(&free_space, OFFSET_SIZE, 1, file);

}


/*
 * Returns true if the pageID is valid and false otherwise
 */
int validate_page_id(Heapfile *heapfile, PageID pid){

    int page_size = heapfile->page_size;

    Offset page_offset = get_page_offset_from_pid(pid, page_size);

    Offset last_dir_offset = heapfile->last_directory_offset;




    Offset dir_offset = get_dir_offset_of_page_offset(page_offset, page_size);

    if (dir_offset < last_dir_offset){
        // All directories before the last directory should be at its full
        // capacity, so the page offset is valid
        return true;
    } else if (dir_offset == last_dir_offset){
        int slot = get_slot_number_of_page_offset(page_offset, page_size);

        forward_file_ptr_to_directory_entry(heapfile->file_ptr, dir_offset, slot, page_size);

        int recorded_page_offset;
        fread(&recorded_page_offset, OFFSET_SIZE, 1, heapfile->file_ptr);
        if (page_offset == recorded_page_offset){
            // The page offset must be the same when reading the directory entry
            return 1;
        } else {
            // If page_offset == 0, then we haven't written to it yet.
            // if page_offset is something else, then there's a mismatch
            return 0;
        }
    } else {
        return 0;
    }
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
        (*page)->records->push_back( new Record());
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
    if (record->size() > 0 ) {
        for( attr = (*record).begin(); attr != (*record).end(); ++attr) {
            strcat(buf, (*attr));
        }
    } else {
        char *empty = new char[NUM_ATTRIBUTES * ATTRIBUTE_SIZE];
        memset(empty, '0', NUM_ATTRIBUTES * ATTRIBUTE_SIZE);
        strcat(buf, empty);
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

        if (strcmp(attribute, "0000000000") == 0) {
            break;
        }

        read += ATTRIBUTE_SIZE;
        if (csv)
            read += sizeof(char);

        (*record).push_back(attribute);
    }
}

/*****************************************************************
 *
 * ADDING RECORDS
 *
 *****************************************************************/

/*
 * Returns:
 *   record slot offset if space available,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r) {
    if (fixed_len_page_freeslots(page) <= 0){
        return -1;  // No more free slots available
    }

    vector<Record*> records = (*(page->records));
    int ret = 0;
    // Iterate over all the vectors in the Records array in Page,
    // return the index of the first vector whose size() == 0
    for (int slot = 0; slot < records.size(); slot++) {
        if ( records[slot]->size() == 0 ) {
            ret = slot;
            break;
        }
    }
    return ret;
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
    if (r->size())
        page->slots_used++;
}

/*****************************************************************
 *
 * UPDATING RECORDS
 *
 *****************************************************************/

void update_record(Record *r, int64 attr, char* new_value) {

        char* attribute = new char[11];
        memcpy( attribute, new_value, ATTRIBUTE_SIZE );
        attribute[10] = '\0';
        delete[] (*r)[attr];
        (*r)[attr] = attribute;
}

/*****************************************************************
 *
 * DELETING RECORDS
 *
 *****************************************************************/

/*
 * Delete/clear the record
 */
void delete_record_at_slot(Record *r) {
    r->clear();
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

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, Offset pid){

    int page_size = heapfile->page_size;
    // Go to the directory entry to insert the metadata
    int total_dir_entries =  get_total_directory_entries(page_size);
    Offset directory_num = pid / (total_dir_entries + 1);

    fseek(heapfile->file_ptr, page_size * (total_dir_entries + 1) * directory_num, SEEK_SET);

    Offset directory_offset = (total_dir_entries + 1) * directory_num;
    Offset directory_entry_slot = pid - directory_offset - 1;

    // Skip the next directory pointer and the directory entries before the entry
    // of the page
    fseek(heapfile->file_ptr, (directory_entry_slot * 2 * OFFSET_SIZE) + OFFSET_SIZE, SEEK_CUR);

    // Write the page offset
    fwrite(&pid, OFFSET_SIZE, 1, heapfile->file_ptr);
    Offset free_space = page->page_size - (page->slots_used * page->slot_size); 
    fwrite(&free_space, OFFSET_SIZE, 1, heapfile->file_ptr);
    

    // This buffer contains all the data from the page file
    char *buf = new char[heapfile->page_size+1];
   
    memset(buf, '\0', heapfile->page_size);

    for (int s = 0; s < page->records->size(); s++) {
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
void read_page(Heapfile *heapfile, Offset pid, Page *page){

    int64 page_size = heapfile->page_size;

    // Go to the directory entry to extract the metadata
    int total_dir_entries =  get_total_directory_entries(page_size);
    Offset directory_num = pid / total_dir_entries;
    fseek(heapfile->file_ptr, page_size * (total_dir_entries + 1) * directory_num, SEEK_SET);

    Offset directory_offset = (total_dir_entries + 1) * directory_num;
    Offset directory_entry_offset = pid - directory_offset - 1;

    // Skip the next directory pointer and the directory entries before the entry
    // of the page and the page_offset of the current directory entry
    fseek(heapfile->file_ptr, (directory_entry_offset * 2 * OFFSET_SIZE) + (2 * OFFSET_SIZE), SEEK_CUR);

    Offset free_space;
    fread(&free_space, OFFSET_SIZE, 1, heapfile->file_ptr);

    // from the heapfile, go to the pageID and then
    // read it's contents in to a char buf of page_size
    // and then write the buffer to the page.
    fseek(heapfile->file_ptr, pid * heapfile->page_size, SEEK_SET);

    // Calculate slot size
    int64 record_size = NUM_ATTRIBUTES * ATTRIBUTE_SIZE;

    // Read in entire page 
    char *buf = new char[heapfile->page_size];
    memset(buf, '\0', heapfile->page_size);

    fread(buf, sizeof(char), heapfile->page_size, heapfile->file_ptr);

    // We will count how many slots are in use
    page->slots_used = 0;

    // For each record, deserialize it and put it in the page
    for (int i = 0; i < heapfile->page_size; i += record_size) {
        Record *record = new Record();

        fixed_len_read(&(buf[i]), record_size, record);
        append_record(page, record);
    }
    
    // Update page meta data
    page->total_slots = page_size / record_size;
    page->page_size = page_size;
    page->slot_size = record_size;
    delete[] buf;
    
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
    vector<Record*> records = (*(page->records));
    for (int i=0; i < records.size(); i++) {
        atts = 0;
        if (records[i]->size() != 0) {
            for ( attr = records[i]->begin(); attr != records[i]->end(); ++attr) {
                cout << (*attr);
                if ( atts != 99 ) {
                    cout << ",";
                } else if (atts == 99) {
                    cout << endl;
                }
                atts++;
            }
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

