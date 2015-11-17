CC = g++

wflp.o: write_fixed_len_pages.cpp
	$(CC) -o .wflp.o -c write_fixed_len_pages.cpp

write_fixed_len_pages: write_fixed_len_pages.cpp wflp.o
	$(CC) -o bin/$@ .wflp.o

rflp.o: read_fixed_len_page.cpp
	$(CC) -o .rflp.o -c read_fixed_len_page.cpp

read_fixed_len_page: read_fixed_len_page.cpp rflp.o
	$(CC) -o bin/$@ .rflp.o

hm.o: heapfile_manager.cpp
	$(CC) -o hm.o -c heapfile_manager.cpp

heapman_tester: heapfile_manager.cpp hm.o
	$(CC) -o bin/$@ hm.o

all: write_fixed_len_pages heapman_tester read_fixed_len_page

clean:
	rm *.o bin/*
