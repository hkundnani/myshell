# myshell
This is the simplified verison of UNIX shell implemented using C. The shell can read the input from stdin and or can be passed (redirect) a file containing the sequence of commands. It either executes the command by itself or may fork and call appropriate functions.
To run the programs
```
# Run make in the directory containing Makefile and then the executable
make
./myshell 
OR 
./myshell < [FILE]
```
#### Functionalities supported by myshell
#### 1. Built-in commands
myshell supports 4 built-in commands: cd, pwd, set and exit.
If no argument is provided to the cd it will set the default to HOME path.
#### 2. External commands
#### 3. Implementation of ls -l (myls)
myls command will display the output similar to ls -l (execpt total). To run the myls one needs to set the MYPATH environemnt variable. The path should be absolute path, without double quotes pointing to the executable of myls present in the code respository. Syntax is shown below
```
set MYPATH=[PATH]/myls
myls
```
#### 4. Input/Output Redirection ( <> )
```
wc < myshell.c
```
The above command will print the newline, word and bytecount of the input file.
```
ls > out.txt
```
The above command will write the output of ls to the out.txt file.
```
wc < myshell.c > out.txt
```
The above command will take input from myshell.c, execute the wc command and write the output to the out.txt file.
#### 5. Pipe ( | )
Output of one command will be chained to the other command as input. Syntax is shown below.
```
ls | wc
```
#### 6. Background Process ( & )
By default the shell will wait for a command to finish but if the command is passed with & then it will let the command execute in background and it will let user run other commands. Syntax is shown below.
```
sleep 5 &
```
#### Note
Space should be given between the argument and command or pipe, redirection and background as shown in the examples.

#### Limitations of myshell
1. May not work properly if space is not given according to the note mentioned above.
2. It doesn't support double quotes hence while setting environment variable using set don't include the value in double quotes.
3. It doesn't support pipe and redirection together.
4. It doesn't support pipe and background process together.
5. It doesn't support redirection and background process together.
6. It doesn't support built-in commands with background process or pipe (except pwd). 
