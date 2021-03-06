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
#include "ArchivItem_Sound_Midi.h"
#include <fstream>
#include "EndianStream.h"
#include <cstring>

namespace libsiedler2{

/** @class baseArchivItem_Sound_Midi
 *
 *  Basisklasse für MIDI-Sounds.
 */

baseArchivItem_Sound_Midi::baseArchivItem_Sound_Midi() : baseArchivItem_Sound()
{
    setType(SOUNDTYPE_MIDI);

    tracks = 0;
}

baseArchivItem_Sound_Midi::baseArchivItem_Sound_Midi(const baseArchivItem_Sound_Midi& item) : baseArchivItem_Sound( item )
{
    tracks = item.tracks;
    tracklist = item.tracklist;
}

baseArchivItem_Sound_Midi::~baseArchivItem_Sound_Midi()
{
}

baseArchivItem_Sound_Midi& baseArchivItem_Sound_Midi::operator=(const baseArchivItem_Sound_Midi& item)
{
    if(this == &item)
        return *this;

    baseArchivItem_Sound::operator=(item);
    tracks = item.tracks;
    tracklist = item.tracklist;

    return *this;
}

int baseArchivItem_Sound_Midi::load(std::istream& file, unsigned int length)
{
    if(!file || length == 0)
        return 1;

    libendian::BigEndianIStreamRef fs(file);
    unsigned int item_length = length;
    long position = fs.getPosition();

    char header[4];
    unsigned int chunk;
    unsigned short type = 0;
    unsigned short ppqs = 96;

    // Header einlesen
    fs >> header;

    // ist es eine MIDI-File? (Header "MThd")
    if(strncmp(header, "MThd", 4) != 0)
        return 3;

    // Länge einlesen
    fs >> length;

    // Typ einlesen
    fs >> type;

    // Tracksanzahl einlesen
    fs >> tracks;

    // PPQS einlesen
    fs >> ppqs;

    if(tracks == 0 || tracks > 256)
        return 8;

    unsigned short track_nr = 0;
    while(track_nr < tracks)
    {
        // Chunk-Typ einlesen
        fs >> chunk;

        switch(chunk)
        {
            case 0x4D54726B: // "MTrk"
            {
                // Länge einlesen
                fs >> length;

                fs.setPositionRel(-8);
                length += 8;

                if(tracklist[track_nr].readMid(file, length) != 0)
                    return 11;

                ++track_nr;
            } break;
            default:
                return 12;
        }
    }

    // auf jeden Fall kompletten Datensatz überspringen
    fs.setPosition(position + item_length);
    return 0;
}

int baseArchivItem_Sound_Midi::write(std::ostream& file) const
{
    if(!file)
        return 1;

    libendian::BigEndianOStreamRef fs(file);
    libendian::LittleEndianOStreamRef fsLE(file);

    unsigned int length = 0;
    for(unsigned short i = 0; i < tracks; ++i)
        length += tracklist[i].getMidLength(false);

    // LST-Länge schreiben (little endian!)
    fsLE << (length + 14);

    // Header schreiben
    fs.write("MThd", 4);

    // Länge schreiben
    fs << length;

    // Typ schreiben
    fs << uint16_t(0);

    // Tracksanzahl schreiben
    fs << tracks;

    // PPQS schreiben
    fs << uint16_t(96);

    for(unsigned short i = 0; i < tracks; ++i)
    {
        fs.write(tracklist[i].getMid(false), tracklist[i].getMidLength(false));
    }

    return 0;
}

} // namespace libsiedler2
