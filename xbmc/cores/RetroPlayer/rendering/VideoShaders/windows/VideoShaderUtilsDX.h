#pragma once
/*
 *      Copyright (C) 2017 Team Kodi
 *      http://kodi.tv
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
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <map>
#include <algorithm>
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderUtils.h"
#include <directxmath.h>
#include <minwindef.h>

namespace KODI
{
namespace SHADER
{
  /* todo
  operator DirectX::XMFLOAT2(const float2& f) const
  {
    return DirectX::XMFLOAT2(static_cast<float>(f.x), static_cast<float>(f.y));
  }
  */

  struct CUSTOMVERTEX {
    FLOAT x, y, z;  // vertex positions
    FLOAT tu, tv;   // texture coordinates
  };
}
}
