#! /bin/bash

FILE_TO_USE="heap.bin"
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

    (cd ../ && ./bin/csv2colstore ./csv/"$NUM_RECORDS.csv" ./scripts/$FILE_TO_USE ${i})  

	echo -e "===== A D Block Size: ${block_names[$b]} =====\n"
    ../bin/select2 $FILE_TO_USE 0 A D ${i} -v
    echo -e "\n------------------------------------------\n"

    echo -e "===== A Z Block Size: ${block_names[$b]} =====\n"
    ../bin/select2 $FILE_TO_USE 0 A Z ${i} -v
    echo -e "\n------------------------------------------\n"

    echo -e "===== A B Block Size: ${block_names[$b]} =====\n"
    ../bin/select2 $FILE_TO_USE 0 A B ${i} -v
    echo -e "\n------------------------------------------\n"

    b=`expr ${b} + 1`


done

(cd ../ && rm ./csv/"$NUM_RECORDS.csv")
(cd ../ && make clean)
