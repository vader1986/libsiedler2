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

#ifndef IALLOCATOR_H_INCLUDED
#define IALLOCATOR_H_INCLUDED

#include "enumTypes.h"

namespace libsiedler2
{
    class ArchivItem;

    /**
     *  @brief Abstract ArchivItem factory
     */
    class IAllocator{
    public:
        virtual ~IAllocator(){}
        virtual ArchivItem* create(BOBTYPES type, SOUNDTYPES subtype = SOUNDTYPE_NONE) const = 0;
        virtual ArchivItem* clone(const ArchivItem& item) const = 0;
    };

}

#endif //IALLOCATOR_H_INCLUDED
