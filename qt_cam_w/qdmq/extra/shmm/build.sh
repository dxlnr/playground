clang++ -O2 -std=c++11 ../../qdmq/shmm/shmm.cc ../../qdmq/shmm/shmm_rw.cc w.cc -DDEBUG -o writer -pthread
clang++ -O2 -std=c++11 ../../qdmq/shmm/shmm.cc r.cc -o reader -pthread
