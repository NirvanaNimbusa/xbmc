#pragma once

/*
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "system.h"

namespace xbmc
{
/* Apparantly "linux" is a constant */
namespace linux_os
{
class IKeymap
{
public:

  virtual ~IKeymap() {};
  
  virtual uint32_t KeysymForKeycode(uint32_t code) const = 0;
  virtual void UpdateMask(uint32_t depressed,
                          uint32_t latched,
                          uint32_t locked,
                          uint32_t group) = 0;
  virtual uint32_t CurrentModifiers() const = 0;
  
  virtual uint32_t XBMCKeysymForKeycode(uint32_t code) const = 0;
  virtual uint32_t ActiveXBMCModifiers() const = 0;
};
}
}
