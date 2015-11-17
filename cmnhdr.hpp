#include <cstdint>
#include <ctime>

using namespace std;

#define NUM_ATTRIBUTES 100
#define ATTRIBUTE_SIZE 10

typedef uint64_t Offset;
typedef uint64_t int64;

int OFFSET_SIZE = sizeof(Offset);

typedef const char* V;
typedef vector<V> Record;

// Vector Iterator
vector<V>::iterator attr;

// Page Struct
typedef struct {
    int64_t total_slots;
    int64_t slots_used;
    int64_t page_size;
    int64_t slot_size;
    vector<Record*>* records;
} Page;

// Heap file Struct
typedef struct {
    FILE *file_ptr;
    int page_size;
} Heapfile;

// Page ID = page offset
typedef Offset PageID;

// RecordID Struct
typedef struct {
    Offset page_id;
    Offset slot;
} RecordID;

// DirectoryEntry struct
typedef struct {
    Offset page_offset;
    int64 free_space;
} DirectoryEntry;

/*****************************************************************
 *
 * PAGE MANAGER METHODS
 *
 *****************************************************************/

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size);
 
/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page);
 
/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page);
 
/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r);
 
/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r);
 
/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record);


/*****************************************************************
 *
 * HEAP MANAGER METHODS
 *
 ****************************************************************/

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

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

/**
 * Returns the total number of directory entries in a directory
 */
int get_total_directory_entries(Heapfile *heapfile);
