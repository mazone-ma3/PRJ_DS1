#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HEADER_SIZE 688
#define MAX_TRACKS 162  // Fixed size for track table
#define CYLINDERS 40
#define HEADS 2
#define SECTORS_PER_TRACK 16
#define SECTOR_SIZE 256
#define TRACKS (CYLINDERS * HEADS)  // 160
#define MEDIA_TYPE 0x00  // 2D
#define WRITE_PROTECT 0x00
#define DENSITY 0x00  // Double density
#define SECTOR_N 0x01  // 256 bytes (2^(7+N))

typedef struct {
    char name[17];  // 16 chars + terminator
    uint8_t reserved[9];
    uint8_t write_protect;
    uint8_t media_type;
    uint32_t disk_size;
    uint32_t track_offsets[MAX_TRACKS];
} d77_header_t;

// Function to write a binary file to specified starting linear sector (1-based)
void write_binary_to_sector(FILE *fp, const char *bin_file, int start_sector) {
    FILE *bin = fopen(bin_file, "rb");
    if (!bin) {
        fprintf(stderr, "Error opening %s\n", bin_file);
        return;
    }

    fseek(bin, 0, SEEK_END);
    long bin_size = ftell(bin);
    fseek(bin, 0, SEEK_SET);

    int num_sectors_needed = (bin_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int linear_sector = start_sector - 1;  // 0-based

    for (int i = 0; i < num_sectors_needed; i++) {
        // Calculate position
        int track = linear_sector / SECTORS_PER_TRACK;
        int sector_in_track = linear_sector % SECTORS_PER_TRACK;
        uint32_t track_offset = HEADER_SIZE + track * (SECTORS_PER_TRACK * (16 + SECTOR_SIZE));
        uint32_t sector_offset = track_offset + sector_in_track * (16 + SECTOR_SIZE) + 16;  // After sector header

        // Read data from bin
        uint8_t buffer[SECTOR_SIZE] = {0};
        size_t to_read = (bin_size - i * SECTOR_SIZE > SECTOR_SIZE) ? SECTOR_SIZE : (bin_size - i * SECTOR_SIZE);
        fread(buffer, 1, to_read, bin);

        // Write to image
        fseek(fp, sector_offset, SEEK_SET);
        fwrite(buffer, 1, SECTOR_SIZE, fp);

        linear_sector++;
    }

    fclose(bin);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s output.d77 file1.bin start_sector1 file2.bin start_sector2\n", argv[0]);
        fprintf(stderr, "start_sector is 1-based linear sector number.\n");
        return 1;
    }

    const char *output = argv[1];
    const char *file1 = argv[2];
    int sector1 = atoi(argv[3]);
    const char *file2 = argv[4];
    int sector2 = atoi(argv[5]);

    if (sector1 < 1 || sector2 < 1) {
        fprintf(stderr, "Sectors must be positive integers >=1\n");
        return 1;
    }

    // Calculate sizes
    uint32_t track_data_size = SECTORS_PER_TRACK * (16 + SECTOR_SIZE);  // 16 header + 256 data per sector
    uint32_t total_data_size = TRACKS * track_data_size;
    uint32_t disk_size = HEADER_SIZE + total_data_size;

    // Prepare header
    d77_header_t header;
    memset(&header, 0, sizeof(header));
    strcpy(header.name, "CUSTOMDISK");  // 10 chars, pad with spaces or null
    header.name[16] = 0x00;  // Terminator
    header.write_protect = WRITE_PROTECT;
    header.media_type = MEDIA_TYPE;
    header.disk_size = disk_size;  // Little endian, but since struct, OK in LE machine

    // Set track offsets
    for (int i = 0; i < TRACKS; i++) {
        header.track_offsets[i] = HEADER_SIZE + i * track_data_size;
    }
    for (int i = TRACKS; i < MAX_TRACKS; i++) {
        header.track_offsets[i] = 0;
    }

    // Create output file
    FILE *fp = fopen(output, "wb");
    if (!fp) {
        fprintf(stderr, "Error creating %s\n", output);
        return 1;
    }

    // Write header
    fwrite(&header, 1, HEADER_SIZE, fp);

    // Write track data: sector headers and empty data
    uint8_t sector_data[SECTOR_SIZE] = {0};  // Empty sector
    for (int trk = 0; trk < TRACKS; trk++) {
        int cyl = trk / HEADS;
        int head = trk % HEADS;

        for (int sec = 0; sec < SECTORS_PER_TRACK; sec++) {
            // Sector header (16 bytes)
            uint8_t sect_header[16] = {0};
            sect_header[0] = cyl;         // C
            sect_header[1] = head;        // H
            sect_header[2] = sec + 1;     // R (1-based)
            sect_header[3] = SECTOR_N;    // N
            sect_header[4] = SECTORS_PER_TRACK & 0xFF;  // Num sectors low
            sect_header[5] = (SECTORS_PER_TRACK >> 8) & 0xFF;  // High
            sect_header[6] = DENSITY;     // Density
            sect_header[7] = 0x00;        // Deleted mark
            sect_header[8] = 0x00;        // Status
            // 9-13 reserved 0
            sect_header[14] = SECTOR_SIZE & 0xFF;  // Size low
            sect_header[15] = (SECTOR_SIZE >> 8) & 0xFF;  // High

            fwrite(sect_header, 1, 16, fp);
            fwrite(sector_data, 1, SECTOR_SIZE, fp);
        }
    }

    // Now overwrite with binaries
    write_binary_to_sector(fp, file1, sector1);
    write_binary_to_sector(fp, file2, sector2);

    fclose(fp);
    printf("Created %s successfully.\n", output);
    return 0;
}