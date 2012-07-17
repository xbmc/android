#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
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
#include <android/input.h>
#include <math.h>

#define TOUCH_MAX_POINTERS  2
class CAndroidTouch
{

public:
  CAndroidTouch();
  ~CAndroidTouch();
  bool onTouchEvent(AInputEvent* event);

private:
  void updateTouches(AInputEvent *event, bool saveLast = true);
  void handleMultiTouchGesture(AInputEvent *event);
  class Touch {
    public:
      Touch() { reset(); }

      bool valid() const { return x >= 0.0f && y >= 0.0f && time >= 0; }
      void reset() { x = -1.0f; y = -1.0f; time = -1; }
      void copy(const Touch &other) { x = other.x; y = other.y; time = other.time; }

      float x;      // in pixels (With possible sub-pixels)
      float y;      // in pixels (With possible sub-pixels)
      int64_t time; // in nanoseconds
  };

  class Pointer {
    public:
      Pointer() { reset(); }

      bool valid() const { return down.valid(); }
      void reset() { down.reset(); last.reset(); moving = false; size = 0.0f; }

      Touch down;
      Touch last;
      Touch current;
      bool moving;
      float size;
  };

  Pointer m_touchPointers[TOUCH_MAX_POINTERS];

  typedef enum {
    TouchGestureUnknown = 0,
    // only primary pointer active but stationary so far
    TouchGestureSingleTouch,
    // primary pointer moving
    TouchGesturePan,
    // at least two pointers active but stationary so far
    TouchGestureMultiTouchStart,
    // at least two pointers active and moving
    TouchGestureMultiTouch,
    // all but primary pointer have been lifted
    TouchGestureMultiTouchDone
  } TouchGestureState;

  class CVector {
    public:
      CVector()
      : x(0.0f), y(0.0f)
      { }
      CVector(float xCoord, float yCoord)
      : x(xCoord), y(yCoord)
      { }
      CVector(const CAndroidTouch::Touch &touch)
      : x(touch.x), y(touch.y)
      { }

      const CVector operator+(const CVector &other) const { return CVector(x + other.x, y + other.y); }
      const CVector operator-(const CVector &other) const { return CVector(x - other.x, y - other.y); }

      float scalar(const CVector &other) { return x * other.x + y * other.y; }
      float length() { return sqrt(pow(x, 2) + pow(y, 2)); }

      float x;
      float y;
  };
  int  XBMC_TouchGestureCheck(float posX, float posY);
  void XBMC_TouchGesture(int32_t action, float posX, float posY, float offsetX, float offsetY);
  void XBMC_Touch(uint8_t type, uint8_t button, uint16_t x, uint16_t y);
  TouchGestureState m_touchGestureState;
};
