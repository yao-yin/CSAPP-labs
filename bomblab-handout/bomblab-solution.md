
#bomblab solution

In this project, we need to defuse a bomb. Here we use the bomb file from self-study materials.
After reading all document here, We don't have many clues.

First we can take a glance at ```bomb.c``` to see the structure of the bomb.

```
initialize_bomb();
printf("Welcome to my fiendish little bomb. You have 6 phases with\n");
printf("which to blow yourself up. Have a nice day!\n");

/* Hmm...  Six phases must be more secure than one phase! */
input = read_line();             /* Get input                   */
phase_1(input);                  /* Run the phase               */
phase_defused();                 /* Drat!  They figured it out!
                    * Let me know how they did it. */
printf("Phase 1 defused. How about the next one?\n");
```

We can find the structure of bomb is linear. After initialization, the program reads input from stdin and try to solve the current phase and repeat "read stdin -> solve current phase" pattern.

###phase_1

In order to defuse this bomb, we need some tools. The most import tool here is gdb debugger and we will use various features of this debugger.

Use this command to start debugging with gdb.
```
$ gdb bomb
```

Then we can disassemble the ```main()``` function with command:
```
(gdb) disassemble main
```

The interesting part is here.
```
0x0000000000400e19 <+121>:   callq  0x4013a2 <initialize_bomb> 
0x0000000000400e1e <+126>:   mov    $0x402338,%edi
0x0000000000400e23 <+131>:   callq  0x400b10 <puts@plt>     
0x0000000000400e28 <+136>:   mov    $0x402378,%edi
0x0000000000400e2d <+141>:   callq  0x400b10 <puts@plt>     
0x0000000000400e32 <+146>:   callq  0x40149e <read_line>    
0x0000000000400e37 <+151>:   mov    %rax,%rdi // Notice here!
0x0000000000400e3a <+154>:   callq  0x400ee0 <phase_1>      
0x0000000000400e3f <+159>:   callq  0x4015c4 <phase_defused>
```
The return value of ```0x40149e <read_line> ``` which is input string  ```char*``` is stored at ```%rax```. Then it  is moved to ```%rdi```. We can also disassemble ```phase_1``` with command:
```
(gdb) disassemble phase_1
```
and get:
```
Dump of assembler code for function phase_1:
   0x0000000000400ee0 <+0>:     sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:     mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:     callq  0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:    test   %eax,%eax
   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:    callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
   0x0000000000400efb <+27>:    retq
```
Usually, registers ```%rdi``` and ```%rsi``` are used to pass first two parameters to the callee function, which is ```0x401338 <strings_not_equal>``` here. We can guess from the name of the function that we need to compare our input with the value in ```%rsi``` which is ```0x402400```.
So we can check it by first add a breakpoint to ```callq  0x401338 <strings_not_equal>``` position with command:
```
(gdb) break strings_not_equal
```
Then run bomb and give any input.
After it stops at breakpoint, we can check the string to get:
```
(gdb) x /s $rsi     
0x402400:       "Border relations with Canada have never been better."
```
This is exactly the solution to ```phase_1```.

###phase_2

We can use the same method to see what happened in ```phase_2```. Here ```disas``` is an abbreviation for ``` disassemble```, remember the input ```char*``` was in ```%rdi``` and ```%rax```.

```
(gdb) disas phase_2
Dump of assembler code for function phase_2:
   0x0000000000400efc <+0>:     push   %rbp
   0x0000000000400efd <+1>:     push   %rbx
   0x0000000000400efe <+2>:     sub    $0x28,%rsp
   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
   0x0000000000400f05 <+9>:     callq  0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:    callq  0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:    jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:    add    %eax,%eax
   0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:    callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:    add    $0x4,%rbx
   0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:    add    $0x28,%rsp
   0x0000000000400f40 <+68>:    pop    %rbx
   0x0000000000400f41 <+69>:    pop    %rbp
   0x0000000000400f42 <+70>:    retq
End of assembler dump.
```






