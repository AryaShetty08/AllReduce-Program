main:
	gcc $(CFLAGS) allreduce.c -o allreduce
remove:
	rm -f*.o allreduce 
