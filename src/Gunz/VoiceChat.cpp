#include "stdafx.h"
#include "VoiceChat.h"
#include "RGMain.h"

#ifdef VOICECHAT

template <typename SampleType = VoiceChat::SampleFormat>
unsigned long GetSampleFormat()
{
	static_assert(false, "Unkwown type");
}

template <> unsigned long GetSampleFormat<char>()			{ return paInt8; }
template <> unsigned long GetSampleFormat<unsigned char>()	{ return paUInt8; }
template <> unsigned long GetSampleFormat<short>()			{ return paInt16; }
template <> unsigned long GetSampleFormat<int>()			{ return paInt32; }
template <> unsigned long GetSampleFormat<float>()			{ return paFloat32; }

VoiceChat* VoiceChat::Instance = nullptr;

VoiceChat::VoiceChat()
{
	if (Instance != nullptr)
		throw std::runtime_error("VoiceChat multiply created");

	Instance = this;

#ifdef WAVEIN
	thr = std::thread([this]()
	{
		while (true)
		{
			ThreadLoop();
		}
	});
#endif

	int error;

	CanPlay = true;

	error = Pa_Initialize();
	if (error != paNoError)
	{
		MLog("Pa_Initialize failed with error code %d: %s\n", error, Pa_GetErrorText(error));

		CanPlay = false;

#ifndef WAVEIN
		CanRecord = true;
#endif

		return;
	}

	[&] {
		CanRecord = true;

#ifdef WAVEIN
		WAVEFORMATEX Format;
		Format.wFormatTag = WAVE_FORMAT_PCM;
		Format.nChannels = 1;
		Format.nSamplesPerSec = SampleRate;
		Format.wBitsPerSample = 16;
		Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * Format.wBitsPerSample / 8;
		Format.nBlockAlign = Format.nChannels * Format.wBitsPerSample / 8;
		Format.cbSize = 0;

		auto ret = waveInOpen(&WaveIn, WAVE_MAPPER, &Format, (DWORD_PTR)GetThreadId(thr.native_handle()), 0, WAVE_FORMAT_DIRECT | CALLBACK_THREAD);

		if (ret != MMSYSERR_NOERROR)
		{
			char Reason[256];

			waveInGetErrorText(ret, Reason, sizeof(Reason));

			MLog("waveInOpen failed: %s\n", Reason);

			CanRecord = false;

			return;
		}
#else
		error = Pa_OpenDefaultStream(&InputStream, NumChannels, 0, GetSampleFormat(), SampleRate, FrameSize, &RecordCallbackWrapper, nullptr);

		if (error != paNoError)
		{
			MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));

			CanRecord = false;

			return;
		}
#endif

		pOpusEncoder = opus_encoder_create(SampleRate, NumChannels, OPUS_APPLICATION_VOIP, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_encoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanRecord = false;

			return;
		}
	}();

	[&] {
		CanDecode = true;

		pOpusDecoder = opus_decoder_create(SampleRate, 1, &error);

		if (error != OPUS_OK)
		{
			MLog("opus_decoder_create failed with error code %d: %s\n", error, opus_strerror(error));

			CanDecode = false;
		}
	}();
}

VoiceChat::~VoiceChat()
{
#ifdef WAVEIN
	waveInClose(WaveIn);
#else
	if(pOpusEncoder)
		opus_encoder_destroy(pOpusEncoder);
	if(pOpusDecoder)
		opus_decoder_destroy(pOpusDecoder);
#endif
}

void VoiceChat::StartRecording()
{
	if (Recording || !CanRecord || ZGetGame()->IsReplay())
		return;

#ifdef WAVEIN
	for (int i = 0; i < 2; i++)
	{
		WAVEHDR &wavehdr = wavehdrs[i];
		wavehdr.lpData = (LPSTR)FrameBuffers[i].data();
		wavehdr.dwBufferLength = sizeof(FrameBuffers[i]);
		wavehdr.dwBytesRecorded = 0;
		wavehdr.dwUser = 0;
		wavehdr.dwFlags = 0;
		wavehdr.dwLoops = 0;
		//MLog("size: %d\n", wavehdr.dwBufferLength);

		auto result = waveInPrepareHeader(WaveIn, &wavehdr, sizeof(WAVEHDR));
		if (result)
		{
			MLog("waveInPrepareHeader failed: %d\n", result);
			return;
		}

		result = waveInAddBuffer(WaveIn, &wavehdr, sizeof(WAVEHDR));
		if (result)
		{
			MLog("waveInAddBuffer failed: %d\n", result);
			return;
		}
	};

	MLog("Added buffers\n");

	waveInStart(WaveIn);

	MLog("Called waveInStart\n");
#else
	if (!Pa_IsStreamActive(InputStream))
	{
		Pa_StopStream(InputStream);
		Pa_StartStream(InputStream);
	}
#endif

	Recording = true;
}

void VoiceChat::StopRecording()
{
	if (!Recording || !CanRecord || ZGetGame()->IsReplay())
		return;

#ifdef WAVEIN
	waveInReset(WaveIn);
	waveInStop(WaveIn);
#else
	Pa_StopStream(InputStream);
#endif

	Recording = false;
}

void VoiceChat::OnReceiveVoiceChat(ZCharacter *Char, const uint8_t *Buffer, int Length)
{
	if (!CanDecode)
		return;

	if (MutedPlayers.find(Char->GetUID()) != MutedPlayers.end())
		return;

	//MLog("OnReceiveVoiceChat size %d\n", Length);

	//short DecodedFrame[FrameSize];

	MicFrame mf;

	int ret = opus_decode(pOpusDecoder, Buffer, Length, mf.pcm, FrameSize, 0);

	if (ret < 0)
	{
		MLog("opus_decode failed with error code %d: %s\n", ret, opus_strerror(ret));
		return;
	}

	auto it = MicStreams.find(Char);

	if (it == MicStreams.end())
	{
		PaStream *Stream = nullptr;

		auto error = Pa_OpenDefaultStream(
			&Stream,
			0,
			1,
			paInt16,
			SampleRate,
			FrameSize,
			PlayCallback,
			Char);

		if (error != paNoError)
		{
			MLog("Pa_OpenStream failed with error code %d: %s\n", error, Pa_GetErrorText(error));
		}

		it = MicStreams.emplace(Char, MicStuff(Stream)).first;

		return;
	}

	if (!it->second.Stream) // Dead stream object that failed to create
		return;

	{
		std::lock_guard<std::mutex> lock(it->second.QueueMutex);

		it->second.Data.push(mf);
	}

	if (!Pa_IsStreamActive(it->second.Stream) && it->second.Data.size() > 2)
	{
		Pa_StopStream(it->second.Stream);
		Pa_StartStream(it->second.Stream);
		it->second.Streaming = true;
	}
}

void VoiceChat::OnDestroyCharacter(ZCharacter * Char)
{
	auto it = MicStreams.find(Char);

	if (it == MicStreams.end())
		return;

	MicStreams.erase(it);
}

bool VoiceChat::MutePlayer(const MUID & UID)
{
	auto it = MutedPlayers.find(UID);

	if (it == MutedPlayers.end())
	{
		MutedPlayers.insert(UID);
		return true;
	}

	MutedPlayers.erase(it);

	return false;
}

#ifdef WAVEIN
void VoiceChat::ThreadLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, MM_WIM_DATA, MM_WIM_DATA) == 1)
	{
		WAVEHDR &wavehdr = *LPWAVEHDR(msg.lParam);

		EncodedSizes[CurrentBufferIndex] = opus_encode(pOpusEncoder, (const opus_int16 *)FrameBuffers[CurrentBufferIndex].data(), FrameSize, &EncodedFrames[CurrentBufferIndex][0], sizeof(EncodedFrames[0]));

		if (EncodedSizes[CurrentBufferIndex] < 0)
		{
			MLog("opus_encode failed with error code %d: %s\n", EncodedSizes[CurrentBufferIndex], opus_strerror(EncodedSizes[CurrentBufferIndex]));
			return;
		}

		//MLog("Encoded data, size %d\n", EncodedSizes[CurrentBufferIndex]);

		auto lambda = [EncodedFrame = EncodedFrames[CurrentBufferIndex], Size = EncodedSizes[CurrentBufferIndex]]
		{
			ZPostVoiceChat(EncodedFrame.data(), Size);
		};

		GetRGMain().Invoke(lambda, 2);

		CurrentBufferIndex = (CurrentBufferIndex + 1) % NumBuffers;

		wavehdr.lpData = LPSTR(FrameBuffers[CurrentBufferIndex].data());

		waveInAddBuffer(WaveIn, &wavehdr, sizeof(WAVEHDR));
	}
}
#else

int VoiceChat::RecordCallbackWrapper(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	return GetInstance()->RecordCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags, userData);
}

int VoiceChat::RecordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	if (!Recording)
		return paComplete;

	std::array<unsigned char, FrameSize> EncodedFrame;
	auto Size = opus_encode(pOpusEncoder, (const opus_int16 *)inputBuffer, FrameSize, EncodedFrame.data(), sizeof(EncodedFrame));

	if (Size < 0)
	{
		MLog("opus_encode failed with error code %d: %s\n", Size, opus_strerror(Size));
		return paContinue;
	}

	//MLog("Encoded data, size %d\n", Size);

	auto lambda = [EncodedFrame, Size]
	{
		ZPostVoiceChat(EncodedFrame.data(), Size);
	};

	GetRGMain().Invoke(lambda);

	return paContinue;
}

#endif

int VoiceChat::PlayCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	auto it = GetInstance()->MicStreams.find((ZCharacter *)userData);

	if (it == GetInstance()->MicStreams.end())
		return paComplete;

	//MLog("Play! Size: %d\n", g_VoiceChat.MicStreams.size());

	{
		std::lock_guard<std::mutex> lock(it->second.QueueMutex);

		auto &Queue = it->second.Data;

		if (Queue.empty())
		{
			//MLog("Empty!\n");
			it->second.Streaming = false;
			return paComplete;
		}

		auto &p = Queue.front();

		memcpy(outputBuffer, p.pcm, sizeof(p.pcm));

		Queue.pop();
	}

	//MLog("Added more stuff! statusFlags: %d, time: %f, framesPerBuffer: %d\n", statusFlags, timeInfo->currentTime, framesPerBuffer);

	return paContinue;
}

void VoiceChat::OnCreateDevice()
{
	auto Success = SpeakerBitmap.Create("SpeakerIcon", RGetDevice(), "Interface/default/SpeakerIcon.png");
	if (!Success)
		MLog("Failed to create speaker icon texture\n");
}

void VoiceChat::OnDraw(MDrawContext* pDC)
{
	int i = 0;

	auto DrawStuff = [&](ZCharacter* Player)
	{
		v2 TopLeft{ float(RELWIDTH(1920 - 400)), float(RELHEIGHT(1080 / 2 + i * 100)) };
		v2 Extents{ float(RELWIDTH(300)), float(RELHEIGHT(50)) };

		auto color = Player->GetTeamID() == MMT_BLUE ? 0xC000A5C3 : 0xC0FF0000;

		pDC->SetColor(color);
		pDC->FillRectangle(TopLeft.x, TopLeft.y, Extents.x, Extents.y);

		v2 TextOffset{ float(RELWIDTH(50)), float(RELHEIGHT(10)) };

		auto v = TopLeft + TextOffset;

		pDC->SetColor(ARGB(255, 255, 255, 255));
		pDC->Text(v.x, v.y, Player->GetUserName());

		auto SpeakerIconOrigin = TopLeft + v2{ float(RELWIDTH(10)), float(RELHEIGHT(10)) };
		v2 SpeakerIconExtents{ float(RELWIDTH(30)), float(RELHEIGHT(30)) };

		pDC->SetBitmap(&SpeakerBitmap);
		pDC->Draw(SpeakerIconOrigin.x, SpeakerIconOrigin.y, SpeakerIconExtents.x, SpeakerIconExtents.y);

		i++;
	};

	if (Recording)
		DrawStuff(ZGetGame()->m_pMyCharacter);

	for (auto item : MicStreams)
	{
		if (!item.second.Streaming)
			continue;

		DrawStuff(item.first);
	}
}
#endif