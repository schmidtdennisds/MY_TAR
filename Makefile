all : my_tar

my_ls : my_tar.c
	gcc -Wall -Wextra -Werror -o my_tar my_tar.c -I.

clean:
	rm -f *.o

fclean: clean
	rm -f my_tar

re: fclean all