#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
#include "stdint.h"
#include "lib.cpp"

/**
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

/**
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

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

int main(int argc, char** argv) {
    if(argc < 2) {
        cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "heap_tester <heap_filee> [-v verbose]" << endl;
        return EXIT_FAILURE;
    }

    // Check if verbose flag is found
    if (argc == 3 && strcmp(argv[2], "-v") == 0) {
       verbose = true;
    }

    if (verbose) {
        cout << "\n== Testing init_heapfile" << endl;
    }

    FILE *file_ptr = fopen(argv[1], "w+");

    Heapfile *hf = new Heapfile();
    int size = 80;
    init_heapfile(hf, size, file_ptr);
	if (verbose){
        cout << "Page size of heapfile: " << hf->page_size << endl;
        cout << "Contents of the page size: " << hf->file_ptr << endl;
    }

    if (verbose) {
        cout << "\n== Testing alloc_page" << endl;
    }

     Offset page_offset = alloc_page(hf);
     if (verbose){
         cout << "Offset of first page allocated should be 1: " << page_offset << endl;
     }
     page_offset = alloc_page(hf);
     if (verbose){
         cout << "Offset of second page allocated should be 2: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of third page allocated should be 3: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of fourth page allocated should be 4: " << page_offset << endl;
    }
    page_offset = alloc_page(hf);
    if (verbose){
        cout << "Offset of fifth page allocated should be 6: " << page_offset << endl;
    }



    delete hf;
    fclose(file_ptr);
	return EXIT_SUCCESS;
}
