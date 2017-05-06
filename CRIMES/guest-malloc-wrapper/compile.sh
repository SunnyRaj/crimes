gcc -o test_malloc test_malloc.c
gcc -O2 -Wall -Werror -fPIC -o prot_malloc.so -shared prot_malloc.c -I.
