#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
#include "cmnhdr.hpp"
#include "stdint.h"

// Global verbose setting
bool verbose = false;

/**
 * Initalize a heapfile to use the file and page size given.
 * precondition: the file is already opened with r+ or w+ permissions
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){

	heapfile->page_size = page_size;
	heapfile->file_ptr = file;

	// The first 8 bytes in the file show indicate what the next
	// page offset is. Since there is no next directory yet, we 
	// set it to be 0
	Offset next_directory_offset = 0;
	fwrite(&next_directory_offset, OFFSET_SIZE, 1, file);

	
	// Next we will initialize all the directory entries
	// A directory entry will consist of a list of DirectoryEntries
	int total_dir_entries = get_total_directory_entries(heapfile);
	Offset initial_dirEntry_offset = 0;
	Offset initial_dirEntry_size = page_size;
	for (int i=0; i < total_dir_entries; i++){
		fwrite(&initial_dirEntry_offset, OFFSET_SIZE, 1, file);
		fwrite(&initial_dirEntry_size, OFFSET_SIZE, 1, file);
	}

	// Set the file pointer to the beginning of the page
	fseek(file, 0, SEEK_SET);	
}

/**
 * Returns the total number of directory entries in a directory
 */
int get_total_directory_entries(Heapfile *heapfile){

	// In a Directory, the first 8 bytes is the offset of the next 
	// directory. The rest of the page is divided in to Directory 
	// Entries
	return (heapfile->page_size - OFFSET_SIZE)/sizeof(DirectoryEntry);
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

int main(int argc, char** argv) {

	return EXIT_SUCCESS;
}
