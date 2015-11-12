CC = g++

wflp.o: write_fixed_len_pages.cpp
	$(CC) -o wflp.o -c write_fixed_len_pages.cpp

write_fixed_len_pages: write_fixed_len_pages.cpp wflp.o
	$(CC) -o bin/$@ wflp.o

clean:
	-rm *o bin/*
