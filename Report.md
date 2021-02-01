#  Submission Report Project 1:

We started the program by expanding on the code provided by the instructor.
Looking at the prompt, we decided to follow the phases that were provides to us.
The first phase required us to implement the fork, exec, wait method, which we
did by using code from the lecture slides. The next phase was about reading and
parsing through arguments given. Following the prompt, a "struct" was made that
iterated through the entire command, seperating based on spaces and NULL
characters. Next, the builtin commands pwd and cd were written. Using some help
from the TA's during office hours, we used getcwd() to return the file path of 
the current working directory for PWD. For cd, we used chdir(), which would
change the current working directory to the one provided by the first argument.
We stored arguments in the array command.args[]. After writing the code for pwd 
and cd, we went back to make some changes that allowed us to recognize and print
errors if either of those commands did not work. Specifically, for pwd, we added
flags that check if there is no error. For cd, we checked for errors returned 
by chdir(). Next up is output redirection, which we actually implemented after
we finished piping, which is the next phase. Piping was implemented by modifying 
and adding code to the fork, exec, wait method that was written earlier. We used
if loops to check if a pipeline was present and also to catch any errors that
may show up. We kept track of the number of pipes present along with managing
actual pipes between forking and redirections. For the redirections part, the
parser was changed to identify the carrot symbol, replace it with a easily
identified character and split the arguements up so that it was possible for the
code handling the output to know where the output began and ended. The last
phase was the extra features. For the redirection of the standard error, we 
modified the output redirection to also allow modification of the stderr file
directory. The piping was also modified to pass through the redirection. The
stack was implemented by a linked list, where every new directory was pushed 
onto the list as a new node and then popped off when the previous directory was
needed. For the actual linked list, we used an implementation of it found on the
 [website](https://www.sanfoundry.com/c-program-stack-using-linked-list/) 
 To test our program throughout its developement, we used CLion to compile and
 recieve a quick output to test a newly implemented feature. For more heavy use
 and cases, we turned to CSIF and compiles and ran it on those computers. Using
 the error codes that we recieved, we were able to lookup the meanings and 
 understand both what the error meant and how to solve the issue.
