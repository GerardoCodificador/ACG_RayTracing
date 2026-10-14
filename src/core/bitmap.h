#ifndef BITMAP_H
#define BITMAP_H

#include "vector3d.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
//#include <iostream>

/**
 * @brief The bmp24_file_header struct
 */
struct bmp24_file_header
{
    char      magic1;    // 'B'
    char      magic2;    // 'M'
    int32_t      size;      // 0
    short int reserved1; // 0
    short int reserved2; // 0
    int32_t      offbits;   // 14 + 40
                         // (info header size) + (fileheader size)

    /**
     * @brief bmp24_file_header
     */
    bmp24_file_header() : magic1('B'), magic2('M'), size(0),
                          reserved1(0), reserved2(0), offbits(54)
    { }

    /**
     * @brief toCharBlock
     * @return
     */
    char *toCharBlock()
    {
        // Allocate 14 bytes of memory
        char* block = (char*)malloc(14);
        block[0]  = magic1;
        block[1]  = magic2;
        memcpy((void*)&block[2],  &size, sizeof(size));
        memcpy((void*)&block[6],  &reserved1, sizeof(reserved1));
        memcpy((void*)&block[8],  &reserved2, sizeof(reserved2));
        memcpy((void*)&block[10], &offbits, sizeof(offbits));
        //long test = block[10];
        //std::cout << "Writing offbits value " << offbits << ": " << test << std::endl;

        return block;
    }
};

/**
 * @brief The bmp24_info_header struct
 */
struct bmp24_info_header
{
    int32_t	  size;             // 40 (size of the info header block in bytes)
    int32_t	  width;            // img.width
    int32_t	  height;           // img.height
    short int planes;           // 1
    short int bit_count;        // 24
    int32_t	  compression;      // 0
    int32_t      size_image;       // (img.width * 3 + extra_bytes) * img.height
    int32_t	  x_pels_per_meter; // 2952
    int32_t	  y_pels_per_meter; // 2952
    int32_t	  clr_used;         // 0
    int32_t	  clr_important;    // 0

    /**
     * @brief bmp24_info_header
     * @param width_
     * @param height_
     */
    bmp24_info_header(size_t width_, size_t height_) : size(40), planes(1), bit_count(24),
                                   compression(0), x_pels_per_meter(2952),
                                   y_pels_per_meter(2952), clr_used(0),
                                   clr_important(0)
    {
        width  = (int32_t) width_;
        height = (int32_t) height_;

        int extra_bytes = (4 - (width * 3) % 4) % 4;
        size_image = (width * 3 + extra_bytes) * height;
    }

    char *toCharBlock()
    {
        char *block = (char *)malloc(40);

        memcpy((void*)&block[0],  &size,   sizeof(int32_t));
        memcpy((void*)&block[4],  &width,  sizeof(int32_t));
        memcpy((void*)&block[8],  &height, sizeof(int32_t));
        memcpy((void*)&block[12], &planes, sizeof(short int));
        memcpy((void*)&block[14], &bit_count,   sizeof(short int));
        memcpy((void*)&block[16], &compression, sizeof(int32_t));
        memcpy((void*)&block[20], &size_image,  sizeof(int32_t));
        memcpy((void*)&block[24], &x_pels_per_meter, sizeof(int32_t));
        memcpy((void*)&block[28], &y_pels_per_meter, sizeof(int32_t));
        memcpy((void*)&block[32], &clr_used,         sizeof(int32_t));
        memcpy((void*)&block[36], &clr_important,    sizeof(int32_t));

        return block;
    }
};


class BitMap
{
public:
    BitMap();

    static int save(Vector3D** &data, const size_t &width, const size_t &height);
    static int read(Vector3D** &dataOut, size_t &width, size_t &height, std::string &fileName);
};

#endif // BITMAP_H
