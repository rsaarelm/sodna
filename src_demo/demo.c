#include "sodna.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    sodna_Cell* cells;
    sodna_init();
    cells = sodna_cells();
    cells[0].fore_r = 15;
    cells[0].symbol = 'A';
    sodna_flush();
    printf("%d\n", sodna_wait_event());
    return 0;
}
