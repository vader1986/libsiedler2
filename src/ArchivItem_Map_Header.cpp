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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "ArchivItem_Map_Header.h"
#include "oem.h"
#include "types.h"
#include <fstream>
#include <EndianStream.h>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class libsiedler2::ArchivItem_Map_Header
 *
 *  Klasse für einen Mapheader.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p ArchivItem_Map_Header.
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem_Map_Header::ArchivItem_Map_Header(void)
    : ArchivItem(),
      width(0), height(0), gfxset(0), player(0)
{
    setBobType(BOBTYPE_MAP_HEADER);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Destruktor von @p ArchivItem_Map_Header.
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem_Map_Header::~ArchivItem_Map_Header(void)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  lädt den Mapheader aus einer Datei.
 *
 *  @param[in] file Dateihandle der Datei
 *
 *  @return liefert Null bei Erfolg, ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::ArchivItem_Map_Header::load(std::istream& file)
{
    if(!file)
        return 1;

    const char VALID_ID[10] = {'W', 'O', 'R', 'L', 'D', '_', 'V', '1', '.', '0'};
    char id[10];

    libendian::LittleEndianIStreamRef fs(file);
    // Signatur einlesen
    fs >> id;

    // und prüfen
    if(memcmp(id, VALID_ID, 10))
        return 3;

    // Name einlesen
    char name[24];
    fs >> name;
    OemToAnsi(name, name);
    this->name = name;

    // GFX-Set einlesen
    fs >> gfxset;

    // Spielerzahl einlesen
    fs >> player;

    // Autor einlesen
    char author[20];
    fs >> author;
    OemToAnsi(author, author);
    this->author = author;

    long old = fs.getPosition();
    fs.setPosition(2348);

    // Breite einlesen
    fs >> width;

    // Höhe einlesen
    fs >> height;

    fs.setPosition(old);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  schreibt den Mapheader in eine Datei.
 *
 *  @param[in] file Dateihandle der Datei
 *
 *  @return liefert Null bei Erfolg, ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::ArchivItem_Map_Header::write(std::ostream& file) const
{
    if(!file)
        return 1;

    return 256;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert den Namen der Map.
 *
 *  @author FloSoft
 */
const std::string& libsiedler2::ArchivItem_Map_Header::getName(void) const
{
    return name;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den Namen der Map.
 *
 *  @param[in] name Name der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setName(const std::string& name)
{
    this->name = name;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert die Breite der Map.
 *
 *  @author FloSoft
 */
unsigned short libsiedler2::ArchivItem_Map_Header::getWidth(void) const
{
    return width;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt die Breite der Map.
 *
 *  @param[in] width Breite der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setWidth(unsigned short width)
{
    this->width = width;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert die Höhe der Map.
 *
 *  @author FloSoft
 */
unsigned short libsiedler2::ArchivItem_Map_Header::getHeight(void) const
{
    return height;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt die Höhe der Map.
 *
 *  @param[in] height Höhe der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setHeight(unsigned short height)
{
    this->height = height;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert das GfxSet der Map.
 *
 *  @author FloSoft
 */
unsigned char libsiedler2::ArchivItem_Map_Header::getGfxSet(void) const
{
    return gfxset;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt das GfxSet der Map.
 *
 *  @param[in] gfxset GfxSet der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setGfxSet(unsigned char gfxset)
{
    this->gfxset = gfxset;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert die Spielerzahl der Map.
 *
 *  @author FloSoft
 */
unsigned char libsiedler2::ArchivItem_Map_Header::getPlayer(void) const
{
    return player;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt die Spielerzahl der Map.
 *
 *  @param[in] player Spielerzahl der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setPlayer(unsigned char player)
{
    this->player = player;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert den Autor der Map.
 *
 *  @author FloSoft
 */
const std::string& libsiedler2::ArchivItem_Map_Header::getAuthor(void) const
{
    return author;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den Autor der Map.
 *
 *  @param[in] author Autor der Map
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivItem_Map_Header::setAuthor(const std::string& author)
{
    this->author = author;
}
