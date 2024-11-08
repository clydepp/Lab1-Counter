#include "Vcounter.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
// ignore error signs


int main(int argc, char **argv, char **env){
    int i;
    int clk;

    Verilated::commandArgs(argc, argv);

    Vcounter* top = new Vcounter;

    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace (tfp, 99);
    tfp->open ("counter.vcd");

    top->clk = 1;
    top->rst = 1;
    top->en = 0;

    bool nbetween = true;
    for (i=0; i<300; i++){
        if (i == 26) top->rst = 1; // cancels value 14
        for (clk=0; clk<2; clk++){
            tfp->dump (2*i + clk);
            top->clk = !top->clk;
            top->eval ();
        }
        top->rst = (i<2);
        if (i > 10 && i < 14) nbetween = false;
        else nbetween = true; // to reset when not in range
        // i is 11, 12, 13 - this is synchronous so delay by one cycle
        top->en = (nbetween);
        // putting line 26 here cancels value 15       
        if (Verilated::gotFinish()) exit(0);
    }
    tfp->close();
    exit(0);
}