CC=gcc

server: sql
	    $(CC) main.c -lsqlite3 sql_driver.o -o server


sql:  
	  $(CC) sql_driver.c -c -w -o sql_driver.o
	  
	  		
peer:  sql
	   $(CC) -g -Wall peer.c -o peer		

clean:
	   rm *.o

.PHONY: all test clean
