all : my_tar

my_tar : my_tar.c my_tar_helper.c
	gcc -Wall -Wextra -Werror -o my_tar my_tar.c my_tar_helper.c -I.

clean:
	rm -f *.o

fclean: clean
	rm -f my_tar

re: fclean all