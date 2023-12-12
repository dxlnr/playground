CUCOMP = nvcc
CFLAGS =

run: vecAdd.o
	$(CUCOMP) $(CFLAGS) vecAdd.o -o run

vecAdd.o: vecAdd.cu
	$(CUCOMP) $(CFLAGS) -c vecAdd.cu

.PHONY: clean cleanall

clean:
	rm -f run *.o

cleanall:
	rm -f run *.o add_vec*
