#include "Vcounter.h"
#include "verilated.h"
#include "verliated_vcd_c.h"
// ignore error signs


int main(int argc, char **argv, char **env){
    int i;
    int clk;

    Verilated::commandArgs(argc, argv);

    Vcounter* top = new Vcounter;

    Verilated::traceEverON(true);
    VerilatedVcdC* tcp = new VerilatedVcdC;
    top->trace (tfp, 99);
    tfp->open ("counter.vcd");

    top->clk = 1;
    top->rest = 1;
    top->en = 0;

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
    tfp->close();
    exit(0);
}