CC = g++
HOME = /usr/cudd-2.5.0
CUDDINCLUDE = $(HOME)/cudd/libcudd.a $(HOME)/util/libutil.a $(HOME)/epd/libepd.a $(HOME)/mtr/libmtr.a $(HOME)/st/libst.a $(HOME)/obj/libobj.a
test: test.c
	$(CC) -L$(CUDDINCLUDE) -I$(HOME)/include/ test.c -o test
