/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "audio/linux/AudioEngine-linux.h"

#include <utility>
#include <vector>

#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "platform/CCFileUtils.h"

using namespace cocos2d;
using namespace cocos2d::experimental;

AudioEngineImpl::AudioEngineImpl()
: _engineInitialized(false)
, _nextAudioID(0)
, _scheduler(nullptr)
{
}

AudioEngineImpl::~AudioEngineImpl()
{
    if (_scheduler)
    {
        _scheduler->unschedule(
            schedule_selector(AudioEngineImpl::update), this);
    }

    stopAll();

    if (_engineInitialized)
    {
        ma_engine_uninit(&_engine);
        _engineInitialized = false;
    }
}

bool AudioEngineImpl::init()
{
    const ma_result result = ma_engine_init(nullptr, &_engine);
    if (result != MA_SUCCESS)
    {
        cocos2d::log(
            "AudioEngine: miniaudio initialization failed: %s",
            ma_result_description(result));
        return false;
    }

    _engineInitialized = true;
    _scheduler = Director::getInstance()->getScheduler();
    _scheduler->schedule(
        schedule_selector(AudioEngineImpl::update), this, 0.05f, false);
    return true;
}

int AudioEngineImpl::play2d(
    const std::string& filePath, bool loop, float volume)
{
    if (!_engineInitialized)
    {
        return AudioEngine::INVALID_AUDIO_ID;
    }

    const std::string fullPath =
        FileUtils::getInstance()->fullPathForFilename(filePath);
    std::unique_ptr<ma_sound> sound(new (std::nothrow) ma_sound);
    if (!sound)
    {
        return AudioEngine::INVALID_AUDIO_ID;
    }

    const ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
    ma_result result = ma_sound_init_from_file(
        &_engine, fullPath.c_str(), flags, nullptr, nullptr, sound.get());
    if (result != MA_SUCCESS)
    {
        cocos2d::log(
            "AudioEngine: unable to load %s: %s",
            filePath.c_str(), ma_result_description(result));
        return AudioEngine::INVALID_AUDIO_ID;
    }

    ma_sound_set_looping(sound.get(), loop ? MA_TRUE : MA_FALSE);
    ma_sound_set_volume(sound.get(), volume);

    result = ma_sound_start(sound.get());
    if (result != MA_SUCCESS)
    {
        cocos2d::log(
            "AudioEngine: unable to play %s: %s",
            filePath.c_str(), ma_result_description(result));
        ma_sound_uninit(sound.get());
        return AudioEngine::INVALID_AUDIO_ID;
    }

    const int audioID = _nextAudioID++;
    SoundInfo info;
    info.sound = std::move(sound);
    info.path = filePath;
    _sounds.emplace(audioID, std::move(info));
    AudioEngine::_audioIDInfoMap[audioID].state =
        AudioEngine::AudioState::PLAYING;
    return audioID;
}

void AudioEngineImpl::setVolume(int audioID, float volume)
{
    const auto it = _sounds.find(audioID);
    if (it != _sounds.end())
    {
        ma_sound_set_volume(it->second.sound.get(), volume);
    }
}

void AudioEngineImpl::setLoop(int audioID, bool loop)
{
    const auto it = _sounds.find(audioID);
    if (it != _sounds.end())
    {
        ma_sound_set_looping(
            it->second.sound.get(), loop ? MA_TRUE : MA_FALSE);
    }
}

bool AudioEngineImpl::pause(int audioID)
{
    const auto it = _sounds.find(audioID);
    return it != _sounds.end() &&
           ma_sound_stop(it->second.sound.get()) == MA_SUCCESS;
}

bool AudioEngineImpl::resume(int audioID)
{
    const auto it = _sounds.find(audioID);
    return it != _sounds.end() &&
           ma_sound_start(it->second.sound.get()) == MA_SUCCESS;
}

bool AudioEngineImpl::stop(int audioID)
{
    const auto it = _sounds.find(audioID);
    if (it == _sounds.end())
    {
        return false;
    }

    ma_sound_stop(it->second.sound.get());
    ma_sound_uninit(it->second.sound.get());
    _sounds.erase(it);
    return true;
}

void AudioEngineImpl::stopAll()
{
    for (auto& entry : _sounds)
    {
        ma_sound_stop(entry.second.sound.get());
        ma_sound_uninit(entry.second.sound.get());
    }
    _sounds.clear();
}

float AudioEngineImpl::getDuration(int audioID)
{
    const auto it = _sounds.find(audioID);
    if (it == _sounds.end())
    {
        return AudioEngine::TIME_UNKNOWN;
    }

    float duration = 0.0f;
    return ma_sound_get_length_in_seconds(
               it->second.sound.get(), &duration) == MA_SUCCESS
               ? duration
               : AudioEngine::TIME_UNKNOWN;
}

float AudioEngineImpl::getCurrentTime(int audioID)
{
    const auto it = _sounds.find(audioID);
    if (it == _sounds.end())
    {
        return AudioEngine::TIME_UNKNOWN;
    }

    float currentTime = 0.0f;
    return ma_sound_get_cursor_in_seconds(
               it->second.sound.get(), &currentTime) == MA_SUCCESS
               ? currentTime
               : AudioEngine::TIME_UNKNOWN;
}

bool AudioEngineImpl::setCurrentTime(int audioID, float time)
{
    const auto it = _sounds.find(audioID);
    return it != _sounds.end() &&
           ma_sound_seek_to_second(
               it->second.sound.get(), time) == MA_SUCCESS;
}

void AudioEngineImpl::setFinishCallback(
    int audioID,
    const std::function<void(int, const std::string&)>& callback)
{
    const auto it = _sounds.find(audioID);
    if (it != _sounds.end())
    {
        it->second.finishCallback = callback;
    }
}

void AudioEngineImpl::uncache(const std::string& filePath)
{
    if (!_engineInitialized)
    {
        return;
    }

    const std::string fullPath =
        FileUtils::getInstance()->fullPathForFilename(filePath);
    ma_resource_manager_unregister_data(
        ma_engine_get_resource_manager(&_engine), fullPath.c_str());
    _preloadedFiles.erase(fullPath);
}

void AudioEngineImpl::uncacheAll()
{
    if (!_engineInitialized)
    {
        return;
    }

    ma_resource_manager* resourceManager =
        ma_engine_get_resource_manager(&_engine);
    for (const auto& filePath : _preloadedFiles)
    {
        ma_resource_manager_unregister_data(
            resourceManager, filePath.c_str());
    }
    _preloadedFiles.clear();
}

int AudioEngineImpl::preload(
    const std::string& filePath,
    std::function<void(bool isSuccess)> callback)
{
    if (!_engineInitialized)
    {
        if (callback)
        {
            callback(false);
        }
        return AudioEngine::INVALID_AUDIO_ID;
    }

    const std::string fullPath =
        FileUtils::getInstance()->fullPathForFilename(filePath);
    const ma_result result = ma_resource_manager_register_file(
        ma_engine_get_resource_manager(&_engine),
        fullPath.c_str(),
        MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE);
    const bool success =
        result == MA_SUCCESS || result == MA_ALREADY_EXISTS;

    if (success)
    {
        _preloadedFiles.insert(fullPath);
    }
    else
    {
        cocos2d::log(
            "AudioEngine: unable to preload %s: %s",
            filePath.c_str(), ma_result_description(result));
    }

    if (callback)
    {
        callback(success);
    }
    return success ? 0 : AudioEngine::INVALID_AUDIO_ID;
}

void AudioEngineImpl::update(float)
{
    std::vector<int> finishedSounds;
    for (const auto& entry : _sounds)
    {
        if (ma_sound_at_end(entry.second.sound.get()))
        {
            finishedSounds.push_back(entry.first);
        }
    }

    for (const int audioID : finishedSounds)
    {
        auto it = _sounds.find(audioID);
        if (it == _sounds.end())
        {
            continue;
        }

        const std::string filePath = it->second.path;
        const auto callback = it->second.finishCallback;
        ma_sound_uninit(it->second.sound.get());
        _sounds.erase(it);
        AudioEngine::remove(audioID);

        if (callback)
        {
            callback(audioID, filePath);
        }
    }
}
