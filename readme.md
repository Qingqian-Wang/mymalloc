# My malloc

- In this project, I designed my own malloc function to support the expansion of the size of the stack in a preassigned way.
For a single block, its structure is like this:


To be specific, I used sbrk() system call to expand the size of the stack, and I used the first and best fit to assign these preassigned memories to the program.




- The folder alloc_policy_tests, alloc_policy_tests_osx, and general_tests contains some method to test the malloc, please have a try if you want.

Thanks for your time and attention.
