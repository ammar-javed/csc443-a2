#include "stdio.h"
#include <vector>
#include <iostream>
#include "string.h"
using namespace std;

typedef const char* V;
typedef std::vector<V> Record;
#define NUM_ATTRIBUTES 100
#define ATTRIBUTE_SIZE 10

// Vector Iterator
std::vector<V>::iterator attr;

// Page Struct
typedef struct {
    Record *records;
    int total_slots;
    int slots_used;
    int page_size;
    int slot_size;
} Page;

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
 * Run unit tests of functions
 *
 */
void run_tests() {

    Record testRec (10, "0123456789");
    int size = fixed_len_sizeof(&testRec);
    if (verbose)
      cout << "The size of testRec(should be 100): " << size << endl;

    char buf[100] = {'\0'};
    fixed_len_write(&testRec, buf);
    if (verbose)
      cout << "Contents of the serialized buffer: " << buf << endl;

    Record* testReadBuf = new Record ;
    fixed_len_read(buf, 50, testReadBuf);
    if (verbose)
      cout << "Size of record: " << fixed_len_sizeof(testReadBuf) << endl;
    delete testReadBuf; 
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

	run_tests();
    return EXIT_SUCCESS;

}

