#include <cstdio>
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
void fixed_len_read(void *buf, int size, Record *record);


int main(int argc, char** argv) {
    
    if(argc < 4) {
	    cout << "ERROR: Invalid number of arguments. Please use the following format:" << endl;
        cout << "write_fixed_len_pages <csv_file> <page_file> <page_size> [-v verbose]" << endl;
	    return EXIT_FAILURE;
    }    

    Record testRec (5, "0123456789");
    int size = fixed_len_sizeof(&testRec);
    cout << size << endl;

    char buf[50] = {'\0'};
    fixed_len_write(&testRec, buf);
    cout << buf << endl;
    return EXIT_SUCCESS;

}

