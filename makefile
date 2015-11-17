CC = g++

wflp.o: write_fixed_len_pages.cpp
	$(CC) -o .wflp.o -c write_fixed_len_pages.cpp

write_fixed_len_pages: write_fixed_len_pages.cpp wflp.o
	$(CC) -o bin/$@ .wflp.o

rflp.o: read_fixed_len_page.cpp
	$(CC) -o .rflp.o -c read_fixed_len_page.cpp

read_fixed_len_page: read_fixed_len_page.cpp rflp.o
	$(CC) -o bin/$@ .rflp.o

clean:
	-rm *o bin/*
