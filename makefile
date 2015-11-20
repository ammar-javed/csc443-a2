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

c2hf.o: csv2heapfile.cpp
	$(CC) -o c2hf.o -c csv2heapfile.cpp

csv2heapfile: csv2heapfile.cpp c2hf.o
	$(CC) -o bin/$@ c2hf.o

scan.o: scan.cpp
	$(CC) -o scan.o -c scan.cpp

scan: scan.cpp scan.o
	$(CC) -o bin/$@ scan.o

insert.o: insert.cpp
	$(CC) -o insert.o -c insert.cpp

insert: insert.cpp insert.o
	$(CC) -o bin/$@ insert.o

select.o: select.cpp
	$(CC) -o select.o -c select.cpp

select: select.cpp select.o
	$(CC) -o bin/$@ select.o

delete.o: delete.cpp
	$(CC) -o delete.o -c delete.cpp

delete: delete.cpp delete.o
	$(CC) -o bin/$@ delete.o

update.o: update.cpp
	$(CC) -o update.o -c update.cpp

update: update.cpp update.o
	$(CC) -o bin/$@ update.o

all: write_fixed_len_pages heapman_tester read_fixed_len_page csv2heapfile scan insert delete update

clean:
	rm *.o bin/*
