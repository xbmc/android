/*!
 \file Key.h
 \brief
 */

#ifndef GUILIB_KEY
#define GUILIB_KEY

#pragma once

/*
 *      Copyright (C) 2005-2008 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "utils/StdString.h"

#include "guilib_defines.h"

class CKey;

/*!
  \ingroup actionkeys
  \brief class encapsulating information regarding a particular user action to be sent to windows and controls
  */
class CAction
{
public:
  CAction(int actionID, float amount1 = 1.0f, float amount2 = 0.0f, const CStdString &name = "", unsigned int holdTime = 0);
  CAction(int actionID, wchar_t unicode);
  CAction(int actionID, unsigned int state, float posX, float posY, float offsetX, float offsetY, const CStdString &name = "");
  CAction(int actionID, const CStdString &name, const CKey &key);

  /*! \brief Identifier of the action
   \return id of the action
   */
  int GetID() const { return m_id; };

  /*! \brief Is this an action from the mouse
   \return true if this is a mouse action, false otherwise
   */
  bool IsMouse() const { return (m_id >= ACTION_MOUSE_START && m_id <= ACTION_MOUSE_END); };

  bool IsGesture() const { return (m_id >= ACTION_GESTURE_NOTIFY && m_id <= ACTION_GESTURE_END); };

  /*! \brief Human-readable name of the action
   \return name of the action
   */
  const CStdString &GetName() const { return m_name; };

  /*! \brief Get an amount associated with this action
   \param zero-based index of amount to retrieve, defaults to 0
   \return an amount associated with this action
   */
  float GetAmount(unsigned int index = 0) const { return (index < max_amounts) ? m_amount[index] : 0; };

  /*! \brief Unicode value associated with this action
   \return unicode value associated with this action, for keyboard input.
   */
  wchar_t GetUnicode() const { return m_unicode; };

  /*! \brief Time in ms that the key has been held
   \return time that the key has been held down in ms.
   */
  unsigned int GetHoldTime() const { return m_holdTime; };

  /*! \brief Time since last repeat in ms
   \return time since last repeat in ms. Returns 0 if unknown.
   */
  float GetRepeat() const { return m_repeat; };

  /*! \brief Button code that triggered this action
   \return button code
   */
  unsigned int GetButtonCode() const { return m_buttonCode; };

private:
  int          m_id;
  CStdString   m_name;

  static const unsigned int max_amounts = 4; // Must be at least 4.
  float        m_amount[max_amounts];

  float        m_repeat;
  unsigned int m_holdTime;
  unsigned int m_buttonCode;
  wchar_t      m_unicode;
};

/*!
  \ingroup actionkeys, mouse
  \brief Simple class for mouse events
  */
class CMouseEvent
{
public:
  CMouseEvent(int actionID, int state = 0, float offsetX = 0, float offsetY = 0)
  {
    m_id = actionID;
    m_state = state;
    m_offsetX = offsetX;
    m_offsetY = offsetY;
  };

  int    m_id;
  int    m_state;
  float  m_offsetX;
  float  m_offsetY;
};

/*!
  \ingroup actionkeys
  \brief
  */
class CKey
{
public:
  CKey(void);
  CKey(uint32_t buttonCode, uint8_t leftTrigger = 0, uint8_t rightTrigger = 0, float leftThumbX = 0.0f, float leftThumbY = 0.0f, float rightThumbX = 0.0f, float rightThumbY = 0.0f, float repeat = 0.0f);
  CKey(uint32_t buttonCode, unsigned int held);
  CKey(uint8_t vkey, wchar_t unicode, char ascii, uint32_t modifiers, unsigned int held);
  CKey(const CKey& key);

  virtual ~CKey(void);
  const CKey& operator=(const CKey& key);
  uint8_t GetLeftTrigger() const;
  uint8_t GetRightTrigger() const;
  float GetLeftThumbX() const;
  float GetLeftThumbY() const;
  float GetRightThumbX() const;
  float GetRightThumbY() const;
  float GetRepeat() const;
  bool FromKeyboard() const;
  bool IsAnalogButton() const;
  bool IsIRRemote() const;
  void SetFromService(bool fromService);
  bool GetFromService() const { return m_fromService; }

  inline uint32_t GetButtonCode() const { return m_buttonCode; }
  inline uint8_t  GetVKey() const       { return m_vkey; }
  inline wchar_t  GetUnicode() const    { return m_unicode; }
  inline char     GetAscii() const      { return m_ascii; }
  inline uint32_t GetModifiers() const  { return m_modifiers; };
  inline unsigned int GetHeld() const   { return m_held; }

  enum Modifier {
    MODIFIER_CTRL  = 0x00010000,
    MODIFIER_SHIFT = 0x00020000,
    MODIFIER_ALT   = 0x00040000,
    MODIFIER_RALT  = 0x00080000,
    MODIFIER_SUPER = 0x00100000
  };

private:
  void Reset();

  uint32_t m_buttonCode;
  uint8_t  m_vkey;
  wchar_t  m_unicode;
  char     m_ascii;
  uint32_t m_modifiers;
  unsigned int m_held;

  uint8_t m_leftTrigger;
  uint8_t m_rightTrigger;
  float m_leftThumbX;
  float m_leftThumbY;
  float m_rightThumbX;
  float m_rightThumbY;
  float m_repeat; // time since last keypress
  bool m_fromService;
};
#endif

