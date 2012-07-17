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

#include "AndroidTouch.h"
#include "XBMCApp.h"
#include "Application.h"
#include "windowing/WinEvents.h"
#include "guilib/GUIWindowManager.h"

CAndroidTouch::CAndroidTouch()
     : m_touchGestureState(TouchGestureUnknown)
{
}

CAndroidTouch::~CAndroidTouch()
{
}

bool CAndroidTouch::onTouchEvent(AInputEvent* event)
{
  CXBMCApp::android_printf("%s", __PRETTY_FUNCTION__);
  if (event == NULL)
    return false;

  size_t numPointers = AMotionEvent_getPointerCount(event);
  if (numPointers <= 0)
    return false;

  int32_t eventAction = AMotionEvent_getAction(event);
  int8_t touchAction = eventAction & AMOTION_EVENT_ACTION_MASK;
  size_t touchPointer = eventAction >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

  float x = AMotionEvent_getX(event, touchPointer);
  float y = AMotionEvent_getY(event, touchPointer);
  float size = AMotionEvent_getTouchMinor(event, touchPointer);
  int64_t time = AMotionEvent_getEventTime(event);

  switch (touchAction)
  {
    case AMOTION_EVENT_ACTION_CANCEL:
      CXBMCApp::android_printf(" => touch cancel (%f, %f)", x, y);
      if (m_touchGestureState == TouchGesturePan ||
          m_touchGestureState >= TouchGestureMultiTouchStart)
      {
        // TODO: can we actually cancel these gestures?
      }

      m_touchGestureState = TouchGestureUnknown;
      for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
        m_touchPointers[pointer].reset();
      // TODO: Should we return true?
      break;

    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      CXBMCApp::android_printf(" => touch down (%f, %f, %d)", x, y, touchPointer);
      m_touchPointers[touchPointer].down.x = x;
      m_touchPointers[touchPointer].down.y = y;
      m_touchPointers[touchPointer].down.time = time;
      m_touchPointers[touchPointer].moving = false;
      m_touchPointers[touchPointer].size = size;

      // If this is the down event of the primary pointer
      // we start by assuming that it's a single touch
      if (touchAction == AMOTION_EVENT_ACTION_DOWN)
      {
        m_touchGestureState = TouchGestureSingleTouch;
        
        // Send a mouse motion event for getting the current guiitem selected
        XBMC_Touch(XBMC_MOUSEMOTION, 0, (uint16_t)x, (uint16_t)y);
      }
      // Otherwise it's the down event of another pointer
      else if (numPointers > 1)
      {
        // If we so far assumed single touch or still have the primary
        // pointer of a previous multi touch pressed down, we can update to multi touch
        if (m_touchGestureState == TouchGestureSingleTouch ||
            m_touchGestureState == TouchGestureMultiTouchDone)
        {
          m_touchGestureState = TouchGestureMultiTouchStart;
        }
        // Otherwise we should ignore this pointer
        else
        {
          CXBMCApp::android_printf(" => ignore this pointer");
          m_touchPointers[touchPointer].reset();
          break;
        }
      }
      return true;

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
      CXBMCApp::android_printf(" => touch up (%f, %f, %d)", x, y, touchPointer);
      if (!m_touchPointers[touchPointer].valid() ||
          m_touchGestureState == TouchGestureUnknown)
      {
        CXBMCApp::android_printf(" => abort touch up");
        break;
      }

      // Just a single tap with a pointer
      if (m_touchGestureState == TouchGestureSingleTouch)
      {
        CXBMCApp::android_printf(" => a single tap");
        XBMC_Touch(XBMC_MOUSEBUTTONDOWN, XBMC_BUTTON_LEFT,
                          (uint16_t)m_touchPointers[touchPointer].down.x,
                          (uint16_t)m_touchPointers[touchPointer].down.y);
        XBMC_Touch(XBMC_MOUSEBUTTONUP, XBMC_BUTTON_LEFT,
                          (uint16_t)m_touchPointers[touchPointer].down.x,
                          (uint16_t)m_touchPointers[touchPointer].down.y);
      }
      // A pan gesture started with a single pointer (ignoring any other pointers)
      else if (m_touchGestureState == TouchGesturePan)
      {
        CXBMCApp::android_printf(" => a pan gesture ending");
        float velocityX = 0.0f; // number of pixels per second
        float velocityY = 0.0f; // number of pixels per second
        int64_t timeDiff = time - m_touchPointers[touchPointer].down.time;
        if (timeDiff > 0)
        {
          velocityX = ((x - m_touchPointers[touchPointer].down.x) * 1000000000) / timeDiff;
          velocityY = ((y - m_touchPointers[touchPointer].down.y) * 1000000000) / timeDiff;
        }
        XBMC_TouchGesture(ACTION_GESTURE_END, velocityX, velocityY, x, y);
      }
      // A single tap with multiple pointers
      else if (m_touchGestureState == TouchGestureMultiTouchStart)
      {
        CXBMCApp::android_printf(" => a single tap with multiple pointers");
        // we send a right-click for this and always use the coordinates
        // of the primary pointer
        XBMC_Touch(XBMC_MOUSEBUTTONDOWN, XBMC_BUTTON_RIGHT,
                          (uint16_t)m_touchPointers[0].down.x,
                          (uint16_t)m_touchPointers[0].down.y);
        XBMC_Touch(XBMC_MOUSEBUTTONUP, XBMC_BUTTON_RIGHT,
                          (uint16_t)m_touchPointers[0].down.x,
                          (uint16_t)m_touchPointers[0].down.y);
      }
      else if (m_touchGestureState == TouchGestureMultiTouch)
      {
        CXBMCApp::android_printf(" => a multi touch gesture ending");
      }
      
      if (touchAction == AMOTION_EVENT_ACTION_UP)
      {
        // Send a mouse motion event pointing off the screen
        // to deselect the selected gui item
        XBMC_Touch(XBMC_MOUSEMOTION, 0, -1, -1);
      }

      // If we were in multi touch mode and lifted the only secondary pointer
      // we can go into the TouchGestureMultiTouchDone state which will allow
      // the user to go back into multi touch mode without lifting the primary finger
      if ((m_touchGestureState == TouchGestureMultiTouchStart || m_touchGestureState == TouchGestureMultiTouch) &&
          numPointers == 2 && touchAction == AMOTION_EVENT_ACTION_POINTER_UP)
      {
        m_touchGestureState = TouchGestureMultiTouchDone;
      }
      // Otherwise abort
      else
      {
        m_touchGestureState = TouchGestureUnknown;
      }
      
      m_touchPointers[touchPointer].reset();
      return true;

    case AMOTION_EVENT_ACTION_MOVE:
      CXBMCApp::android_printf(" => touch move (%f, %f)", x, y);
      if (!m_touchPointers[touchPointer].valid() ||
          m_touchGestureState == TouchGestureUnknown ||
          m_touchGestureState == TouchGestureMultiTouchDone)
      {
        CXBMCApp::android_printf(" => abort touch move");
        break;
      }
      
      if (m_touchGestureState == TouchGestureSingleTouch)
      {
        updateTouches(event);
        
        // Check if the touch has moved far enough to count as movement
        if (!m_touchPointers[touchPointer].moving)
        {
          CXBMCApp::android_printf(" => touch of size %f has not moved far enough => abort touch move", m_touchPointers[touchPointer].size);
          break;
        }
        
        CXBMCApp::android_printf(" => a pan gesture starts");
        XBMC_TouchGesture(ACTION_GESTURE_BEGIN, 
                                 m_touchPointers[touchPointer].down.x,
                                 m_touchPointers[touchPointer].down.y,
                                 0.0f, 0.0f);
        m_touchPointers[touchPointer].last.copy(m_touchPointers[touchPointer].down);
        m_touchGestureState = TouchGesturePan;
      }
      else if (m_touchGestureState == TouchGestureMultiTouchStart)
      {
        CXBMCApp::android_printf(" => switching from TouchGestureMultiTouchStart to TouchGestureMultiTouch");
        m_touchGestureState = TouchGestureMultiTouch;
        
        updateTouches(event);
        for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
          m_touchPointers[pointer].last.copy(m_touchPointers[pointer].current);
      }

      // Let's see if we have a pan gesture (i.e. the primary and only pointer moving)
      if (m_touchGestureState == TouchGesturePan)
      {
        CXBMCApp::android_printf(" => a pan gesture");
        XBMC_TouchGesture(ACTION_GESTURE_PAN, x, y,
                                 x - m_touchPointers[touchPointer].last.x,
                                 y - m_touchPointers[touchPointer].last.y);
        m_touchPointers[touchPointer].last.x = x;
        m_touchPointers[touchPointer].last.y = y;
      }
      else if (m_touchGestureState == TouchGestureMultiTouch)
      {
        CXBMCApp::android_printf(" => a multi touch gesture");
        CAndroidTouch::handleMultiTouchGesture(event);
      }
      else
        break;

      return true;

    default:
      CXBMCApp::android_printf(" => touch unknown (%f, %f)", x, y);
      break;
  }

  return false;
}

void CAndroidTouch::XBMC_Touch(uint8_t type, uint8_t button, uint16_t x, uint16_t y)
{
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  newEvent.type = type;
  newEvent.button.type = type;
  newEvent.button.button = button;
  newEvent.button.x = x;
  newEvent.button.y = y;

  CXBMCApp::android_printf("XBMC_Touch(%u, %u, %u, %u)", type, button, x, y);
  CWinEvents::MessagePush(&newEvent);
}

void CAndroidTouch::handleMultiTouchGesture(AInputEvent *event)
{
  CXBMCApp::android_printf("%s", __PRETTY_FUNCTION__);
  // don't overwrite the last position as we need
  // it for rotating
  updateTouches(event, false);

  Pointer& primaryPointer = m_touchPointers[0];
  Pointer& secondaryPointer = m_touchPointers[1];

  // calculate zoom/pinch
  CVector primary = primaryPointer.down;
  CVector secondary = secondaryPointer.down;
  CVector diagonal = primary - secondary;
  float baseDiffLength = diagonal.length();
  if (baseDiffLength != 0.0f)
  {
    CVector primaryNow = primaryPointer.current;
    CVector secondaryNow = secondaryPointer.current;
    CVector diagonalNow = primaryNow - secondaryNow;
    float curDiffLength = diagonalNow.length();

    float centerX = (primary.x + secondary.x) / 2;
    float centerY = (primary.y + secondary.y) / 2;

    float zoom = curDiffLength / baseDiffLength;

    XBMC_TouchGesture(ACTION_GESTURE_ZOOM, centerX, centerY, zoom, 0);
  }
}

void CAndroidTouch::updateTouches(AInputEvent *event, bool saveLast /* = true */)
{
  for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
  {
    if (saveLast)
      m_touchPointers[pointer].last.copy(m_touchPointers[pointer].current);
    
    m_touchPointers[pointer].current.x = AMotionEvent_getX(event, pointer);
    m_touchPointers[pointer].current.y = AMotionEvent_getY(event, pointer);
    m_touchPointers[pointer].current.time = AMotionEvent_getEventTime(event);
    
    // Calculate whether the pointer has moved at all
    if (!m_touchPointers[pointer].moving)
    {
      CVector down = m_touchPointers[pointer].down;
      CVector current = m_touchPointers[pointer].current;
      CVector distance = down - current;
      
      if (distance.length() > m_touchPointers[pointer].size)
        m_touchPointers[pointer].moving = true;
    }
  }
}

void CAndroidTouch::XBMC_TouchGesture(int32_t action, float posX, float posY, float offsetX, float offsetY)
{
  CXBMCApp::android_printf("XBMC_TouchGesture(%d, %f, %f, %f, %f)", action, posX, posY, offsetX, offsetY);
  if (action == ACTION_GESTURE_BEGIN)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, 0, 0), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_PAN)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, offsetY), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_END)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, offsetY), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_ZOOM)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, 0), WINDOW_INVALID, false);
}

int CAndroidTouch::XBMC_TouchGestureCheck(float posX, float posY)
{
  CXBMCApp::android_printf("XBMC_TouchGestureCheck(%f, %f)", posX, posY);
  CGUIMessage message(GUI_MSG_GESTURE_NOTIFY, 0, 0, posX, posY);
  if (g_windowManager.SendMessage(message))
    return message.GetParam1();

  return EVENT_RESULT_UNHANDLED;
}