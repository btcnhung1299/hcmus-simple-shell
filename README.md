# Simple Shell
This project consists of designing a C/C++ program to serve as a shell interface that accepts user commands  and then executes each command in a separate process.

### Run the program
```terminal
g++ main.cpp -o prog
./prog
```

### Features

1. **Execute a command in child process**:
- Support internal commands such as `ls`, `ps`, `cat`, etc.
```terminal
osh> ls
```
- Make command run in the background by appending '&'.
```terminal
osh> ls &
```

2. **Redirect I/O**
- Input redirection: use '<' as redirect character
```terminal
osh> sort < input.txt
```
- Output redirection: use '>' as redirect character
```terminal
osh> ls -la > output.txt
```

3. **History**
- Use '!!' as signal to retrieve the lastest command. Respond "No command in history" if there is no recent command.
```terminal
osh> ls
osh> !!
```

4. **Piped commands**
- Use '|' as linking character between two commands. No redirection operators supported.
```terminal
osh> ls -la | ps -ael
```

5. **Exit**
```terminal
osh> exit
```
