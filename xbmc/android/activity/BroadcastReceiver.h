#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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
#include <jni.h>
#include "JNIManager.h"
#include "threads/Event.h"

class CBroadcastReceiver : public CAndroidJNIBase
{
friend class CAndroidJNIManager;
public:
  bool WaitForMedia(int timeout);
private:
  CBroadcastReceiver();
  static void ReceiveGenericIntent(JNIEnv *env, jobject thiz, jobject intent);
  static void ReceiveMediaMounted(JNIEnv *env, jobject thiz, jobject intent);
  CEvent m_mediaMounted;
};