file_name="result.txt"
rm ${file_name}
for i in 256 512 1024 2048 4096
do
	./gemm $i | grep "\[Intel\|\[Student\|\[Ref\|Total squared error student sol" >> ${file_name}
	echo "finished $i"
done
