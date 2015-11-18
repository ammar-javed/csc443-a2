#! /bin/bash

FILE_TO_WRITE="test.bin"
NUM_RECORDS=1000

block_names=("1KB" "2KB" "4KB" "8KB" "16KB" "32KB" "128KB" "512KB" "1MB")
block_sizes=(1000 2000 4000 8000 16000 32000 128000 512000 1000000)

(cd ../ && make clean)
(cd ../ && make all)
(cd ../ && python gencsv.py ./csv/"$NUM_RECORDS.csv" $NUM_RECORDS)
echo -e "\n"

b=0
for i in ${block_sizes[@]}
do

    ../bin/write_fixed_len_pages ../csv/$NUM_RECORDS.csv $FILE_TO_WRITE ${i} > /dev/null
	echo -e "===== Block Size: ${block_names[$b]} =====\n"
    ../bin/read_fixed_len_page $FILE_TO_WRITE ${i}
    echo -e "\n------------------------------------------\n"

    rm $FILE_TO_WRITE
    b=`expr ${b} + 1`

done

(cd ../ && rm ./csv/"$NUM_RECORDS.csv")
(cd ../ && make clean)
