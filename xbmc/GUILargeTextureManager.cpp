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

#include "threads/SystemClock.h"
#include "GUILargeTextureManager.h"
#include "pictures/Picture.h"
#include "settings/GUISettings.h"
#include "FileItem.h"
#include "guilib/Texture.h"
#include "threads/SingleLock.h"
#include "utils/TimeUtils.h"
#include "utils/JobManager.h"
#include "guilib/GraphicContext.h"
#include "utils/log.h"
#include "TextureCache.h"

using namespace std;


CImageLoader::CImageLoader(const CStdString &path, float *width, float *height)
{
  m_path = path;
  m_texture = NULL;
  if (width)
    m_width = width;
  if (height)
    m_height = height;
}

CImageLoader::~CImageLoader()
{
  delete(m_texture);
}

bool CImageLoader::DoWork()
{
  CStdString texturePath = g_TextureManager.GetTexturePath(m_path);
  CStdString loadPath = CTextureCache::Get().CheckCachedImage(texturePath); 
  
  // If empty, then go on to validate and cache image as appropriate
  // If hit, continue down and load image
  if (loadPath.IsEmpty())
  {
    CFileItem file(m_path, false);

    // Validate file URL to see if it is an image
    if ((file.IsPicture() && !(file.IsZIP() || file.IsRAR() || file.IsCBR() || file.IsCBZ() )) 
       || file.GetMimeType().Left(6).Equals("image/")) // ignore non-pictures
    { 
      // Cache the image if necessary
      loadPath = CTextureCache::Get().CacheImageFile(texturePath);
      if (loadPath.IsEmpty())
        return false;
    }
    else
      return true;
  }

  m_texture = new CTexture();
  unsigned int start = XbmcThreads::SystemClockMillis();
  if (!m_texture->LoadFromFile(m_path, (unsigned int)*m_width, (unsigned int)*m_height, g_guiSettings.GetBool("pictures.useexifrotation")))
  {
    delete m_texture;
    m_texture = NULL;
  }
  else if (XbmcThreads::SystemClockMillis() - start > 100)
    CLog::Log(LOGDEBUG, "%s - took %u ms to load %s", __FUNCTION__, XbmcThreads::SystemClockMillis() - start, m_path.c_str());

  return true;
}

CGUILargeTextureManager::CLargeTexture::CLargeTexture(const CStdString &path)
{
  m_path = path;
  m_refCount = 1;
  m_timeToDelete = 0;
}

CGUILargeTextureManager::CLargeTexture::~CLargeTexture()
{
  assert(m_refCount == 0);
  m_texture.Free();
}

void CGUILargeTextureManager::CLargeTexture::AddRef()
{
  m_refCount++;
}

bool CGUILargeTextureManager::CLargeTexture::DecrRef(bool deleteImmediately)
{
  assert(m_refCount);
  m_refCount--;
  if (m_refCount == 0)
  {
    if (deleteImmediately)
      delete this;
    else
      m_timeToDelete = CTimeUtils::GetFrameTime() + TIME_TO_DELETE;
    return true;
  }
  return false;
}

bool CGUILargeTextureManager::CLargeTexture::DeleteIfRequired(bool deleteImmediately)
{
  if (m_refCount == 0 && (deleteImmediately || m_timeToDelete < CTimeUtils::GetFrameTime()))
  {
    delete this;
    return true;
  }
  return false;
}

void CGUILargeTextureManager::CLargeTexture::SetTexture(CBaseTexture* texture)
{
  assert(!m_texture.size());
  if (texture)
    m_texture.Set(texture, texture->GetWidth(), texture->GetHeight(), texture->GetOriginalImageWidth(), texture->GetOriginalImageHeight());
}

CGUILargeTextureManager::CGUILargeTextureManager()
{
}

CGUILargeTextureManager::~CGUILargeTextureManager()
{
}

void CGUILargeTextureManager::CleanupUnusedImages(bool immediately)
{
  CSingleLock lock(m_listSection);
  // check for items to remove from allocated list, and remove
  listIterator it = m_allocated.begin();
  while (it != m_allocated.end())
  {
    CLargeTexture *image = *it;
    if (image->DeleteIfRequired(immediately))
      it = m_allocated.erase(it);
    else
      ++it;
  }
}

// if available, increment reference count, and return the image.
// else, add to the queue list if appropriate.
bool CGUILargeTextureManager::GetImage(const CStdString &path, CTextureArray &texture, bool firstRequest, float width, float height)
{
  CSingleLock lock(m_listSection);
  for (listIterator it = m_allocated.begin(); it != m_allocated.end(); ++it)
  {
    CLargeTexture *image = *it;
    if (image->GetPath() == path)
    {
      CTextureArray temp = image->GetTexture();
      if (temp.size() > 0)
      {
        // Our image may have been scaled to fit a control. If it's too small we'll need to create a new one.
        // If the current image is bigger than the required size, use it. If it's the full-size image, use it.
        // Else, return false signaling that we need a new GetImage.
        if (((int)temp.m_width >= (int)width && (int)temp.m_height >= (int)height) || 
           ((int)temp.m_width == (int)temp.m_originalWidth && (int)temp.m_height == (int)temp.m_originalHeight))
        {
          if (firstRequest)
            image->AddRef();
          texture = temp;
          return true;
        }
      }
    }
  }
  if (firstRequest)
  {
    QueueImage(path, width, height);
  }
  return true;
}

void CGUILargeTextureManager::ReleaseImage(const CStdString &path, bool immediately, float width, float height)
{
  CSingleLock lock(m_listSection);
  float foundWidth = 0, foundHeight = 0;
  for (listIterator it = m_allocated.begin(); it != m_allocated.end(); ++it)
  {
    CLargeTexture *image = *it;
    if (image->GetPath() == path)
    {
      CTextureArray texture;
      texture = image->GetTexture();
      if (texture.size() > 0 && (int)(texture.m_width) == (int)width && (int)(texture.m_height) == (int)height)
      {
        foundWidth = texture.m_width;
        foundHeight = texture.m_height;
        if (image->DecrRef(immediately) && immediately)
          m_allocated.erase(it);
        break;
      }
    }
  }
  for (queueIterator it = m_queued.begin(); it != m_queued.end(); ++it)
  {
    unsigned int id = it->first;
    CLargeTexture *image = it->second;
    if (image->GetPath() == path)
    {
      CTextureArray texture;
      texture = image->GetTexture();
      if ((int)width <= (int)foundWidth && (int)height <= (int)foundHeight)
      {
        // cancel this job
        image->DecrRef(true);
        CJobManager::GetInstance().CancelJob(id);
        m_queued.erase(it);
        break;
      }
    }
  }
}

// queue the image, and start the background loader if necessary
void CGUILargeTextureManager::QueueImage(const CStdString &path, float width, float height)
{
  CSingleLock lock(m_listSection);
  for (queueIterator it = m_queued.begin(); it != m_queued.end(); ++it)
  {
    CLargeTexture *image = it->second;
    if (image->GetPath() == path)
    {
      if (width > (int)*(image->GetWidth()) || height > (int)*(image->GetHeight()))
        image->SetSize(width, height);
      image->AddRef();
      return; // already queued
    }
  }
  // queue the item
  CLargeTexture *image = new CLargeTexture(path);
  image->SetSize(width, height);
  unsigned int jobID = CJobManager::GetInstance().AddJob(new CImageLoader(path, image->GetWidth(), image->GetHeight()), this, CJob::PRIORITY_NORMAL);
  m_queued.push_back(make_pair(jobID, image));
}

void CGUILargeTextureManager::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  // see if we still have this job id
  CSingleLock lock(m_listSection);
  for (queueIterator it = m_queued.begin(); it != m_queued.end(); ++it)
  {
    if (it->first == jobID)
    { // found our job
      CImageLoader *loader = (CImageLoader *)job;
      CLargeTexture *image = it->second;
      image->SetTexture(loader->m_texture);
      loader->m_texture = NULL; // we want to keep the texture, and jobs are auto-deleted.
      m_queued.erase(it);
      m_allocated.push_back(image);
      return;
    }
  }
}



