# Lab 1 - Counter
## Task 1: Simulating an 8-bit binary counter
### Initial struggles
After forking the GitHub repository onto my local drive, I struggled to open and run the Verilator commands. This is because: I needed to run the code in a WSL Distro. 

My lab folder can be found under: ``\\wsl.localhost\Ubuntu-22.04\home\clydepangi\Documents\iac\Lab1-Counter``. With this, I was able to execute the Verilator commands provided.

### Explaining the testbench ***counter_tb.cpp***
The testbench does several things:
- Traces signals and plots it to ***counter.vcd***
- Runs through a for-loop that toggles through the clock
- Instantiates ***Vcounter***, a DUT (device under test)
#### Tracing signals
Note that the ``top->trace`` parameter ``99`` is related to the verbosity of the program. Having a high value allows high precision capture, and is good for scenarios with constantly changing signals. It allows us to accurately analyse state changes without the worry of losing any data.
```cpp
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace (tfp, 99);
    tfp->open ("counter.vcd");
```
This code turns tracing on at line 1, by setting the parameter to traceEverOn to true. The function of struct ``Verilated`` in the header file ***verilated.h*** is seen below:
```cpp
   static void traceEverOn(bool flag) {
	if (flag) { calcUnusedSigs(flag); }
   }
```
Within the header file ***verilated_vcd_c.h***, the function of class type _VerilatedVcdC_ has a void function that opens the counter VCD (value change dump) file, that displays all the values of the waveforms over time - for plotting and simulation.

#### Implementing a clock
Integer _i_ represents the number of the current executing cycle, where 300 in this case is a chosen upper bound at which to stop the clock. _clk_ as a counter in the for-loop is different from _top->clk_ within the loop. The former is a variable defined at the start of the main() function, whereas the latter is an attribute of the class _top_ - a value either 1 or 0. _top_ is the 'top-level entity'. A top-level entity is the primary inputs and outputs of a design - we instantiate internal variables within the top-level entity.
```cpp
    for (i=0; i<300; i++){
        for (clk=0; clk<2; clk++){
            tfp->dump (2*i + clk);
            top->clk = !top->clk;
            top->eval ();
        }
        top->rst = (i<2) | (i==15);
        top->en = (i>4);
        if (Verilated::gotFinish()) exit(0);
    }
```
The line under the nested for-loop, performs a bitwise OR operation ``|``, on the two numeric boolean results. If ``Verilated::gotFinish()`` returns true, the program calls ``exit(0)``, so that the loop is exited, and this part of the program is terminated.

### My results from GTKWave
Note that the executable file generated in folder ***obj_dir*** is called ***counter.vcd***. The traces obtained are seen below:
<p align="center">
<img src="https://github.com/user-attachments/assets/69a6e95a-46a6-40e9-bbe8-b98a49f5732a" width=50% height=50%>
</p>

*Why are the timings in ps?*
This may be due to the verbosity level as selected earlier in the program. But it also relates to the clock speed of the computer system, most cores run within the GHz range, so having an average clock cycle at a pico-level allows us to run high-speed designs.

### TEST YOURSELF CHALLENGES
1. Modify the testbench so that you stop counting for 3 cycles once the counter reaches 0x9, and then resume counting. You may also need to change the stimulus for _rst_.

I implemented this by using a 'temp' Boolean condition _nbetween_ that is true iff the _count_ is between 9 and 11 (inclusive). Disabling the ``top->en`` value means that the value is preserved, as the SystemVerilog file works by adding the current number and _en_.
```cpp
    if (i > 10 && i < 14) nbetween = false;
    else nbetween = true; // to reset when not in range
    // i is 11, 12, 13 - this is synchronous so delay by one cycle
    top->en = (nbetween);
```
![image](https://github.com/user-attachments/assets/7885a3b0-3211-482c-9cf6-4ec23338956f)

2. The current counter has a synchronous reset. To implement asynchronous reset, you can change line 11 of counter.sv to detect change in _rst_ signal. (See notes.)

To implement this, I made changes to the ``always_ff`` parameters, adding the _posedge rst_, so that the reset can happen even if the clock is not toggled. I made changes to the counter.sv file, so that the addition of _rst_ makes the function perform like ``always (rst)``.
```sv
    always_ff @(posedge clk, posedge rst)
    if (rst) count <= {WIDTH{1'b0}};
    else count <= count + {{WIDTH-1{1'b0}}, en};
```
![image](https://github.com/user-attachments/assets/2fc4c096-20ae-4005-9e8a-1ee5b917dcb5)
Adding to the program in challenge 1, ``if (i == 26) top->rst = 1;`` was added at the end of the nested for-loop, so that the count resets asynchronously.
## Task 2: Linking Verilator simulation with Vbuddy
### Setting up the Vbuddy interface
Within the bash terminal run this to search for the USB terminal the Vbuddy is connected to, sharing it with WSL. **This must be run every time the Vbuddy is used** - include this line in the doit.sh file. My device name is ``/dev/ttyUSB0``.
```
~/Documents/iac/lab0-devtools/tools/attach_usb.sh
```
In the task2 folder, create a file called vbuddy.cfg and put the name of the device terminated with a backslash. For me, the only line would be ``/dev/ttyUSB0\``.
### Modifying the testbench for Vbuddy
#### Initialising Vbuddy
The function vbdOpen() is from the _vbuddy.cpp_ file (included as header for the testbench). It reads in the port name from the vbuddy.cfg file using the stdlib ``fopen("vbuddy.cfg, "r")`` - with the use of r for read. The error at compilation: ``** Error opening port: /dev/ttyUSB0C`` is due to an error with ``serial.openDevice()``, which sets the return value _errorOpening_ within vbdOpen(). The testbench should not have to return -1, since the error is dealt within vbuddy.cpp. vbdHeader() writes the string into an array, but doesn't print it.
```cpp
   if (vbdOpen() != 1) return(-1);
   vbdHeader("Lab 1: Counter");
```
#### Outputing values to the 7-segment display
```cpp
   vbdHex(4, (int(top->count) >> 16) & 0xF);
   vbdHex(3, (int(top->count) >> 8) & 0xF);
   vbdHex(2, (int(top->count) >> 4) & 0xF);
   vbdHex(1, int(top->count) & 0xF);
   vbdCycle(i+1);
```
This code above, inserted into the for-loop uses the std::sprintf function (like vbdHeader) to write to global function serial. The vbdHex function consists of 6 switch cases, the first parameter. The second parameter is formed by using the count value (see counter.sv) and right shifting ``>>`` it by either 16, 8 or 4 bits. This result is then bitwise ANDed with 0xF.

After compilation: a counter made up of four 7-segments starts to iterate through hex values.
<p align="center">
<img src="https://github.com/user-attachments/assets/f2af5653-37d9-42dd-9a69-18a1b6f288d1" width=40% height=40%>
</p>

### Exploring the flag feature
The Vbuddy has a push-button switch. We can toggle the enable signal through the vbdFlag(), which returns its current value and toggles. The flag state can be observed at the bottom of the display. When the flag state is 0 (top->en = 0), the counter doesn't increment but the cycles still do.
```cpp
   // top->en = (i>4); from before
   top->en = vbdFlag();
```
#### Plotting instead of the 7-seg display
Instead of showing count values on 7-segment displays, you may also plot this on the TFT by replacing the vdbHex() section with the command vbdPlot(). You may want to increase the number of clock cycles to simulate because plotting a dot is much faster than outputting to the 7-segment display. You can start/stop the counter with the flag.
```cpp
   vbdPlot(int(top->count), 0, 255);
```
<p align="center">
<img src="https://cdn.discordapp.com/attachments/1219258758631395388/1297587966771728394/2560F110-A557-417C-A6C5-02666209A862.jpg?ex=67167877&is=671526f7&hm=8ad13ea67f3efeaabeac2e087c90ec546dc80a5d2970dd1275ace959bed2a6a0&" width=20% height=20%>
</p>

The cut at the very start (bottom left corner) is due to the reset being triggered at cycle _i = 4_.
### TEST YOURSELF CHALLENGE
Modify your counter and testbench files so that the en signal controls the direction of counting: ‘1’ for up and ‘0’ for down, via the vbdFlag() function.

I edited the counter.sv file so that there could be synchronous switching if enable was on or off. If enable is active (on), then the count increments. This time not by en, but by 1 - as this will not work otherwise. Seeing the image below, the reset still works as before.
```sv
    if (rst) count <= {WIDTH{1'b0}};
    else if (en) count <= count + {{WIDTH-1{1'b0}}, 0'b1};
    else count <= count - {{WIDTH-1{1'b0}}, 0'b1};
```
<p align="center">
<img src="https://cdn.discordapp.com/attachments/1219258758631395388/1297592587254763550/3F9E11FD-514D-4AB6-9EB0-A5EE830B4E23.jpg?ex=67167cc4&is=67152b44&hm=017cf06dea01f7c58ae34fe49604372cf467092cc1b537a52360a1b4d3507c1e&" width=40% height=40%>
</p>
