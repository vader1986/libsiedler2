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
#include "ArchivItem_Map.h"
#include "ArchivItem_Map_Header.h"
#include "ArchivItem_Raw.h"
#include "libsiedler2.h"
#include "IAllocator.h"
#include "EndianStream.h"
#include <fstream>

/** @class libsiedler2::ArchivItem_Map
 *
 *  Klasse für eine Mapfile.
 */

libsiedler2::ArchivItem_Map::ArchivItem_Map() : ArchivItem(), ArchivInfo()
{
    setBobType(BOBTYPE_MAP);

    alloc(16);
}

libsiedler2::ArchivItem_Map::~ArchivItem_Map()
{
}

/**
 *  lädt die Mapdaten aus einer Datei.
 *
 *  @param[in] file Dateihandle der Datei
 *  @param[in] only_header Soll nur der Header gelesen werden?
 *
 *  @return liefert Null bei Erfolg, ungleich Null bei Fehler
 */
int libsiedler2::ArchivItem_Map::load(std::istream& file, bool only_header)
{
    if(!file)
        return 1;

    ArchivItem_Map_Header* header = dynamic_cast<ArchivItem_Map_Header*>(getAllocator().create(BOBTYPE_MAP_HEADER));

    if(header->load(file) != 0){
        delete header;
        return 2;
    }

    set(0, header);

    // nur der Header?
    if(only_header)
        return 0;

    struct BlockHeader{ //-V802
        uint16_t id; // Must be 0x2710
        uint32_t unknown; // Always 0
        uint16_t w, h;
        uint16_t multiplier; // Not sure, always 1
        uint32_t blockLength;
    };

    const unsigned short w = header->getWidth();
    const unsigned short h = header->getHeight();

    libendian::LittleEndianIStreamRef fs(file);
    for(unsigned int i = 0; i < 14; ++i)
    {
        BlockHeader bHeader;
        fs >> bHeader.id >> bHeader.unknown >> bHeader.w >> bHeader.h >> bHeader.multiplier >> bHeader.blockLength;
        // Header id must match
        if(bHeader.id != 0x2710 || bHeader.unknown != 0)
        {
            assert(false);
            return 3;
        }
        // Blocksize must match extents
        if(bHeader.blockLength != static_cast<unsigned>(w)*static_cast<unsigned>(h))
        {
            assert(false);
            return 4;
        }
        // Multiplier of 0 means unused block and implies no data
        if(bHeader.multiplier == 0)
        {
            assert(bHeader.blockLength == 0);
            continue;
        }
        // If there is data, size must match
        if(bHeader.w != w || bHeader.h != h)
        {
            assert(false);
            return 5;
        }

        if(i == 0 && header->hasExtraWord())
        {
            // Work around for map file bug: There are 2 extra bytes inbetween the header which would actually belong to the first block
            fs.setPositionRel(-2);
        }

        ArchivItem_Raw* layer = dynamic_cast<ArchivItem_Raw*>(getAllocator().create(BOBTYPE_RAW));
        if(layer->load(file, bHeader.blockLength) != 0){
            delete layer;
            return 6;
        }
        set(i + 1, layer);
    }

    extraInfo.clear();

    while(true)
    {
        ExtraAnimalInfo info;
        fs >> info.id;
        if(info.id == 0xFF)
            break;
        fs >> info.x >> info.y;
        extraInfo.push_back(info);
    }

    return 0;
}

/**
 *  schreibt die Mapdaten in eine Datei.
 *
 *  @param[in] file Dateihandle der Datei
 *
 *  @return liefert Null bei Erfolg, ungleich Null bei Fehler
 */
int libsiedler2::ArchivItem_Map::write(std::ostream& file) const
{
    if(!file)
        return 1;

    const ArchivItem_Map_Header* header = dynamic_cast<const ArchivItem_Map_Header*>(get(0));

    if(!header)
        return 2;
    if(header->write(file) != 0)
        return 3;

    libendian::LittleEndianOStreamRef fs(file);
    for(unsigned int i = 0; i < 14; ++i)
    {
        const ArchivItem_Raw* layer = dynamic_cast<const ArchivItem_Raw*>(get(i + 1));
        fs << uint16_t(0x2710) << uint32_t(0);
        if(layer)
        {
            fs << uint16_t(header->getWidth()) << uint16_t(header->getHeight()) << uint16_t(1);
            assert(layer->getData().size() == size_t(header->getWidth()) * size_t(header->getHeight()));
            if(layer->write(file, true) != 0)
                return 4 + i;
        } else
        {
            fs << uint16_t(0) << uint16_t(0) << uint16_t(0) << uint32_t(0);
        }
    }

    for(std::vector<ExtraAnimalInfo>::const_iterator it = extraInfo.begin(); it != extraInfo.end(); ++it)
    {
        fs << it->id << it->x << it->y;
    }
    fs << uint8_t(0xFF);

    return 0;
}
