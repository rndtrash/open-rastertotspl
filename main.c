#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <cups/raster.h>
#include <cups/ppd.h> // TODO: replace with the Job Ticket API
#include <signal.h>
#include <stdbool.h>

#include "c23_compat.h"

#define SCHEDULER_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define SCHEDULER_ALERT(...) SCHEDULER_PRINTF("ALERT: " __VA_ARGS__)
#define SCHEDULER_ATTR(...) SCHEDULER_PRINTF("ATTR: " __VA_ARGS__)
#define SCHEDULER_CRIT(...) SCHEDULER_PRINTF("CRIT: " __VA_ARGS__)
#define SCHEDULER_DEBUG(...) SCHEDULER_PRINTF("DEBUG: " __VA_ARGS__)
#define SCHEDULER_DEBUG2(...) SCHEDULER_PRINTF("DEBUG2: " __VA_ARGS__)
#define SCHEDULER_EMERG(...) SCHEDULER_PRINTF("EMERG: " __VA_ARGS__)
#define SCHEDULER_ERROR(...) SCHEDULER_PRINTF("ERROR: " __VA_ARGS__)
#define SCHEDULER_INFO(...) SCHEDULER_PRINTF("INFO: " __VA_ARGS__)
#define SCHEDULER_NOTICE(...) SCHEDULER_PRINTF("NOTICE: " __VA_ARGS__)
#define SCHEDULER_PAGE(...) SCHEDULER_PRINTF("PAGE: " __VA_ARGS__)
#define SCHEDULER_PPD(...) SCHEDULER_PRINTF("PPD: " __VA_ARGS__)
#define SCHEDULER_STATE(...) SCHEDULER_PRINTF("STATE: " __VA_ARGS__)
#define SCHEDULER_WARNING(...) SCHEDULER_PRINTF("WARNING: " __VA_ARGS__)

#define PRINTER_PRINTF(...) fprintf(stdout, __VA_ARGS__)

inline void printer_cls() {
    PRINTER_PRINTF("CLS\r\n");
}

inline void printer_reverse(int x, int y, int width, int height) {
    PRINTER_PRINTF("REVERSE %d,%d,%d,%d\r\n", x, y, width, height);
}

inline void printer_print(int sets, int copies) {
    PRINTER_PRINTF("PRINT %d,%d\r\n", sets, copies);
}

inline void printer_write(const void *buffer, ssize_t length) {
    write(fileno(stdout), buffer, length);
}

typedef enum printer_mode {
    PRINTER_MODE_OVERWRITE = 0,
    PRINTER_MODE_OR = 1,
    PRINTER_MODE_XOR = 2
} printer_mode_t;

inline void printer_bitmap_begin(int x, int y, int width, int height, printer_mode_t mode) {
    PRINTER_PRINTF("BITMAP %d,%d,%d,%d,%d,", x, y, width, height, mode);
}

inline void printer_bitmap_end() {
    PRINTER_PRINTF("\r\n");
}

_Atomic bool g_cancelled = false;

void cancel(int) {
    g_cancelled = true;
}

int main(int argc, char **argv, char **env) {
    setbuf(stderr, nullptr);

    if (argc > 7 || argc < 6) {
        SCHEDULER_ERROR("rastertotspl job-id user title copies options [file]\n");
        exit(EXIT_FAILURE);
    }

    int fd = 0;
    if (argc == 7 && (fd = open(argv[6], O_RDONLY)) == -1) {
        SCHEDULER_ERROR("Unable to open raster file");
        sleep(1); // TODO: rastertotspl does that, as well as rastertoepson, why?
        exit(EXIT_FAILURE);
    }

    //TODO: load settings

    cups_raster_t *ras = cupsRasterOpen(fd, CUPS_RASTER_READ);

    signal(SIGPIPE, cancel);

    ppd_file_t *ppd = ppdOpenFile(getenv("PRINTER"));
    if (!ppd) {
        int lineNum;
        ppd_status_t status = ppdLastError(&lineNum);

        SCHEDULER_ERROR("The PPD file could not be opened.");
        SCHEDULER_DEBUG("%s on line %d.\n", ppdErrorString(status), lineNum);

        exit(EXIT_FAILURE);
    }

    int page = 0;
    cups_page_header2_t header;
    while (!g_cancelled && cupsRasterReadHeader2(ras, &header)) {
        page++;

        SCHEDULER_PAGE("PAGE: %d %d\n", page, header.NumCopies);
        SCHEDULER_INFO("Starting page %d.", page);

        //TODO: start page
        //StartPage(ppd, &header);

        //TODO: push the bitmap to printer using BITMAP and PRINT commands

        SCHEDULER_INFO("Finished page %d.", page);

        //TODO: afaik the printer moves to the next sticker automatically, but I might be wrong

        if (g_cancelled)
            break;
    }

    // TODO: shutdown

    ppdClose(ppd);

    cupsRasterClose(ras);
    if (fd != 0)
        close(fd);

    if (page == 0) {
        SCHEDULER_ERROR("No pages were found.");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
    return 0;
}
