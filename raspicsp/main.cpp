#include <iostream>
#include <fstream>
#include <iomanip>
#include "HAL.h"
#include "PIC24.h"
#include "Logger.h"

#define MCRL_PIN 0
#define PGC_PIN 1
#define PGD_PIN 4

using namespace std;

HAL hal(MCRL_PIN, PGD_PIN, PGC_PIN);

/**
 * Tries to find a device with the given name
 */
int findDevice(char *name, DEVICE &dev) {
    for (int i = 0; i < NUM_DEVICES; i++) {
        if (!strcmp(name, DEVICES[i].NAME)) {
            dev = DEVICES[i];
            return 1;
        }
    }
    return 0;
}

void readHexFile(const char *name, std::list<MemoryWord> &mem) {
    HexFile file;
    std::fstream in;
    in.open(name, std::fstream::in);
    file.parse(in);
    file.compileTo16BitWords(mem);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: raspicsp <device> <hexfile>\n");
        return 1;
    }

    DEVICE dev;
    if (!findDevice(argv[1], dev)) {
        printf("Unknown device: %s\n\nKnown devices:\n", argv[1]);
        for (int i = 0; i < NUM_DEVICES; i++) {
            printf(" * %s\n", DEVICES[i].NAME);
        }
        return 2;
    }

    PIC24 pgm(hal, dev);
    std::list<MemoryWord> mem;
    readHexFile(argv[2], mem);

    Logger::log("main", "DEVICE is %s", dev.NAME);
    Logger::log("main", "TBLPAG is %04x", dev.TBLPAG_ADDR);


    uint16_t lo, hi;
    pgm.read_device_id(lo, hi);
    Logger::log("main", "Device ID is: 0x%04x 0x%04x", lo, hi);

    for (int i = 0; i < 16; i += 2) {
        Logger::log("main", "PGM[%d] is 0x%08x", i, pgm.read_word(i));
    }

    Logger::log("main", "PGM[0xFF0000] is 0x%08x", pgm.read_word(0xFF0000));

//    Logger::log("main", "Erasing page of memory");
//    pgm.erase_page(0x3400);

	
    Logger::log("main", "Erasing all program memory...");
    pgm.erase_chip();

    Logger::log("main", "Programming device...");
    pgm.program(mem);

    Logger::log("main", "Verifying memory...");
    pgm.verify(mem);

    return 0;
}
