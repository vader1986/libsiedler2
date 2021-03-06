// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "main.h" // IWYU pragma: keep
#include "ArchivItem_Bitmap.h"
#include "ArchivItem_Palette.h"
#include "prototypen.h"
#include "libsiedler2.h"
#include "IAllocator.h"
#include "EndianStream.h"
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem/path.hpp> // For UTF8 support
#include <vector>
#include <cmath>
#include <iostream>

/**
 *  liest eine Bitmapzeile
 */
template<class T_FStream>
static inline void LoadBMP_ReadLine(T_FStream& bmp,
                                    unsigned short y,
                                    unsigned int width,
                                    unsigned char bbp,
                                    libsiedler2::baseArchivItem_Bitmap& bitmap,
                                    std::vector<unsigned char>& buffer)
{
    bmp >> buffer;

    for(unsigned short x = 0; x < width; ++x)
    {
        switch(bbp)
        {
            case 1: // 256
            {
                bitmap.tex_setPixel(x, y, buffer[x * bbp], NULL);
            } break;
            case 3: // 24 bit
            {
                if(buffer[x * bbp + 2] == 0xFF && buffer[x * bbp + 1] == 0x00 && buffer[x * bbp + 0] == 0x8F) // transparenz? (color-key "rosa")
                    bitmap.tex_setPixel(x, y, 0, 0, 0, 0x00);
                else
                    bitmap.tex_setPixel(x, y, buffer[x * bbp + 2], buffer[x * bbp + 1], buffer[x * bbp + 0], 0xFF);
            } break;
        }
    }
    if(width * bbp % 4 > 0)
        bmp.ignore(4 - (width * bbp % 4));
}

/**
 *  lädt eine BMP-File in ein ArchivInfo.
 *
 *  @param[in]  file    Dateiname der BMP-File
 *  @param[out] items   ArchivInfo-Struktur, welche gefüllt wird
 *
 *  @return Null bei Erfolg, ein Wert ungleich Null bei Fehler
 *
 *  @todo RGB Bitmaps (Farben > 8Bit) ebenfalls einlesen.
 */
int libsiedler2::loader::LoadBMP(const std::string& file, ArchivItem*& image, ArchivItem** palette)
{
    struct BMHD
    {
        unsigned short header; // 2
        unsigned int size; // 6
        unsigned int reserved; // 10
        unsigned int offset; // 14
    } bmhd;

    struct BMIH
    {
        unsigned int length; // 4
        int width; // 8
        int height; // 12
        short planes; // 14
        short bbp; // 16
        unsigned int compression; // 20
        unsigned int size; // 24
        int xppm; // 28
        int yppm; // 32
        int clrused; // 36
        int clrimp; // 40
    } bmih ;

    image = NULL;
    if(palette)
        *palette = NULL;

    bool bottomup = false;

    if(file.empty())
        return 1;

    // Datei zum lesen öffnen
    boost::iostreams::mapped_file_source mmapFile;
    try{
        mmapFile.open(bfs::path(file));
    }catch(std::exception& e){
        std::cerr << "Could not open '" << file << "': " << e.what() << std::endl;
        return 2;
    }
    typedef boost::iostreams::stream<boost::iostreams::mapped_file_source> MMStream;
    MMStream mmapStream(mmapFile);
    libendian::EndianIStream<false, MMStream& > bmp(mmapStream);

    // hat das geklappt?
    if(!bmp)
        return 2;

    // Bitmap-Header einlesen
    bmp >> bmhd.header;
    bmp >> bmhd.size;
    bmp >> bmhd.reserved;
    bmp >> bmhd.offset;

    if(bmhd.header != 0x4D42)
        return 4;

    // Bitmap-Info-Header einlesen
    //bmp >> bmih;
    bmp >> bmih.length;
    bmp >> bmih.width;
    bmp >> bmih.height;
    bmp >> bmih.planes;
    bmp >> bmih.bbp;
    bmp >> bmih.compression;
    bmp >> bmih.size;
    bmp >> bmih.xppm;
    bmp >> bmih.yppm;
    bmp >> bmih.clrused;
    bmp >> bmih.clrimp;

    if(bmih.height > 0)
        bottomup = true;

    bmih.height = (bottomup ? bmih.height : -bmih.height);

    // nur eine Ebene
    if(bmih.planes != 1)
        return 6;

    boost::interprocess::unique_ptr< baseArchivItem_Bitmap, Deleter<baseArchivItem_Bitmap> > bitmap(dynamic_cast<baseArchivItem_Bitmap*>(getAllocator().create(BOBTYPE_BITMAP_RAW)));
    bitmap->setName(file);

    switch(bmih.bbp)
    {
        case 8: // nur 8 Bit
        {
            bitmap->setFormat(libsiedler2::FORMAT_PALETTED);
        } break;
        case 24: // oder 24 Bit
        {
            bitmap->setFormat(libsiedler2::FORMAT_RGBA);
        } break;
        default:
        {
            fprintf(stderr, "unknown bitmap depth: %d ", bmih.bbp);
            return 7;
        } break;
    }

    // keine Kompression
    if(bmih.compression != 0)
        return 8;

    // Einträge in der Farbtabelle
    if(bmih.clrused == 0)
        bmih.clrused = (int)pow(2.0, bmih.bbp);

    //items->alloc(2);

    if(bmih.bbp == 8)
    {
        if(palette)
            *palette = (ArchivItem_Palette*)getAllocator().create(BOBTYPE_PALETTE);
        //ArchivItem_Palette *palette = (ArchivItem_Palette*)getAllocator().create(BOBTYPE_PALETTE, NULL);
        //items->set(0, palette);

        // Farbpalette lesen
        unsigned char colors[256][4];
        bmp.read(colors[0], bmih.clrused * 4);

        // Farbpalette zuweisen
        if(palette)
        {
            ArchivItem_Palette* pal = dynamic_cast<ArchivItem_Palette*>(*palette);
            for(int i = 0; i < bmih.clrused; ++i)
                pal->set(i, Color(colors[i][2], colors[i][1], colors[i][0]));

            bitmap->setPalette(pal);
        }
    }

    // Bitmapdaten setzen
    bitmap->setWidth(bmih.width);
    bitmap->setHeight(bmih.height);
    bitmap->tex_alloc();

    unsigned char bbp = (bmih.bbp / 8);
    std::vector<unsigned char> buffer(bmih.width * bbp);

    // Bitmap Pixel einlesen
    if(bottomup)
    {
        // Bottom-Up, "von unten nach oben"
        for(int y = bmih.height - 1; y >= 0; --y)
            LoadBMP_ReadLine(bmp, y, bmih.width, bbp, *bitmap, buffer);
    }
    else
    {
        // Top-Down, "von oben nach unten"
        for(int y = 0; y < bmih.height; ++y)
            LoadBMP_ReadLine(bmp, y, bmih.width, bbp, *bitmap, buffer);
    }

    // Bitmap zuweisen
    image = bitmap.release();

    // alles ok
    return 0;
}

