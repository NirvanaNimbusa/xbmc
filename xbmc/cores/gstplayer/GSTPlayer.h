#pragma once
/*
 *      Copyright (C) 2011 Team XBMC
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

#include "FileItem.h"
#include "cores/IPlayer.h"
#include "dialogs/GUIDialogBusy.h"
#include "threads/Thread.h"

typedef struct INT_GST_VARS INT_GST_VARS;

class CGSTPlayer : public IPlayer, public CThread
{
public:

  CGSTPlayer(IPlayerCallback &callback);
  virtual ~CGSTPlayer();
  
  virtual bool  Initialize(TiXmlElement* pConfig);
  virtual void  RegisterAudioCallback(IAudioCallback* pCallback) {}
  virtual void  UnRegisterAudioCallback()                        {}
  virtual bool  OpenFile(const CFileItem &file, const CPlayerOptions &options);
  virtual bool  QueueNextFile(const CFileItem &file)             {return false;}
  virtual void  OnNothingToQueueNotify()                         {}
  virtual bool  CloseFile();
  virtual bool  IsPlaying() const;
  virtual void  Pause();
  virtual bool  IsPaused() const;
  virtual bool  HasVideo() const;
  virtual bool  HasAudio() const;
  virtual void  ToggleFrameDrop();
  virtual bool  CanSeek();
  virtual void  Seek(bool bPlus = true, bool bLargeStep = false);
  virtual bool  SeekScene(bool bPlus = true);
  virtual void  SeekPercentage(float fPercent = 0.0f);
  virtual float GetPercentage();
  virtual float GetCachePercentage();

  virtual void  SetVolume(long nVolume);
  virtual void  SetDynamicRangeCompression(long drc)              {}
  virtual void  GetAudioInfo(CStdString &strAudioInfo);
  virtual void  GetVideoInfo(CStdString &strVideoInfo);
  virtual void  GetGeneralInfo(CStdString &strVideoInfo);
  virtual void  Update(bool bPauseDrawing);
  virtual void  GetVideoRect(CRect& SrcRect, CRect& DestRect);
  virtual void  SetVideoRect(const CRect &SrcRect, const CRect &DestRect);
  virtual void  GetVideoAspectRatio(float &fAR);
  virtual bool  CanRecord()                                       {return false;};
  virtual bool  IsRecording()                                     {return false;};
  virtual bool  Record(bool bOnOff)                               {return false;};
  virtual void  SetAVDelay(float fValue = 0.0f);
  virtual float GetAVDelay();

  virtual void  SetSubTitleDelay(float fValue = 0.0f);
  virtual float GetSubTitleDelay();
  virtual int   GetSubtitleCount();
  virtual int   GetSubtitle();
  virtual void  GetSubtitleName(int iStream, CStdString &strStreamName);
  virtual void  SetSubtitle(int iStream);
  virtual bool  GetSubtitleVisible();
  virtual void  SetSubtitleVisible(bool bVisible);
  virtual bool  GetSubtitleExtension(CStdString &strSubtitleExtension) { return false; }
  virtual int   AddSubtitle(const CStdString& strSubPath);

  virtual int   GetAudioStreamCount();
  virtual int   GetAudioStream();
  virtual void  GetAudioStreamName(int iStream, CStdString &strStreamName);
  virtual void  SetAudioStream(int iStream);
  virtual void  GetAudioStreamLanguage(int iStream, CStdString &strLanguage) {};

  virtual TextCacheStruct_t* GetTeletextCache()                   {return NULL;};
  virtual void  LoadPage(int p, int sp, unsigned char* buffer)    {};

  virtual int   GetChapterCount();
  virtual int   GetChapter();
  virtual void  GetChapterName(CStdString& strChapterName);
  virtual int   SeekChapter(int iChapter);

  virtual float GetActualFPS();
  virtual void  SeekTime(__int64 iTime = 0);
  virtual __int64 GetTime();
  virtual int   GetTotalTime();
  virtual void  ToFFRW(int iSpeed = 0);
  virtual int   GetAudioBitrate();
  virtual int   GetVideoBitrate();
  virtual int   GetSourceBitrate();
  virtual int   GetChannels();
  virtual int   GetBitsPerSample();
  virtual int   GetSampleRate();
  virtual CStdString GetAudioCodecName();
  virtual CStdString GetVideoCodecName();
  virtual int   GetPictureWidth();
  virtual int   GetPictureHeight();
  virtual bool  GetStreamDetails(CStreamDetails &details);
  // Skip to next track/item inside the current media (if supported).
  virtual bool  SkipNext()                                        {return false;}

  virtual bool  IsInMenu() const                                  {return false;};
  virtual bool  HasMenu()                                         {return false;};

  virtual void  DoAudioWork()                                     {};
  virtual bool  OnAction(const CAction &action)                   {return false;};

  virtual bool  GetCurrentSubtitle(CStdString& strSubtitle);
  //returns a state that is needed for resuming from a specific time
  virtual CStdString GetPlayerState();
  virtual bool  SetPlayerState(CStdString state);
  
  virtual CStdString GetPlayingTitle();
  
  virtual bool  IsCaching() const                                 {return false;};
  virtual int   GetCacheLevel() const;

  INT_GST_VARS* GetGSTVars(void)                                  {return m_gstvars;};
  void    ProbeStreams(void);
  void    GetLastFrame(void);

protected:
  virtual void  OnStartup();
  virtual void  OnExit();
  virtual void  Process();

  void          GSTShutdown(void);
  void          ProbeUDPStreams(void);
  bool          WaitForGSTPaused( int timeout_ms);
  bool          WaitForGSTPlaying(int timeout_ms);
  bool          WaitForWindowFullScreenVideo(int timeout_ms);

private:
  int                     m_speed;
  bool                    m_paused;
  bool                    m_StopPlaying;
  CEvent                  m_ready;
  CFileItem               m_item;
  CPlayerOptions          m_options;

  CCriticalSection        m_csection;

  int64_t                 m_elapsed_ms;
  int64_t                 m_duration_ms;
  int64_t                 m_avdelay_ms;
  int64_t                 m_subdelay_ms;
  int                     m_audio_index;
  int                     m_audio_count;
  CStdString              m_audio_info;
  uint32_t                m_audio_bits;
  uint32_t                m_audio_channels;
  uint32_t                m_audio_samplerate;
  int                     m_video_index;
  int                     m_video_count;
  CStdString              m_video_info;
  int                     m_subtitle_index;
  int                     m_subtitle_count;
  bool                    m_subtitle_show;

  int                     m_chapter_count;
  struct chapters
  {
    std::string name;
    int64_t     seekto_ms;
  }                       m_chapters[64];

  float                   m_video_fps;
  int                     m_video_width;
  int                     m_video_height;
  CRect                   m_dst_rect;
  int                     m_view_mode;

  INT_GST_VARS            *m_gstvars;
};
