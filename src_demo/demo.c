#include "sodna.h"

int main(int argc, char* argv[]) {
    sodna_init();
    sodna_set(10, 10, sodna_map_color(255, 0, 0), sodna_map_color(0, 0, 0), '@');
    sodna_flush();
    sodna_wait_event();
    return 0;
}
