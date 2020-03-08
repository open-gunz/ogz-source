#include "Download.h"
#include "Log.h"
#include "MUtil.h"
#include "MFile.h"
#include "Hash.h"

#include "sodium.h"

// Include MWindows.h to undefine all the Windows macros that curl.h brought in.
#include "MWindows.h"
#include "curl/curl.h"

// MInetUtil.h must be included after curl.h, otherwise some Winsock stuff errors out.
#include "MInetUtil.h"

static int GetCurlResponseCode(CURL* curl)
{
	long ResponseCode;
	auto curl_ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ResponseCode);
	if (curl_ret != CURLE_OK)
	{
		Log.Error("curl_easy_getinfo returned error code %d, error message: %s, "
			"when trying to retrieve the response code\n",
			int(curl_ret), curl_easy_strerror(curl_ret));
		assert(false);
		return -1;
	}

	return int(ResponseCode);
}

enum class ProtocolType
{
	HTTP,
	HTTPS,
	FILE,
	Unknown,
};

struct DownloadInfoContext
{
	void* curl;
	const char* Range;
	ProtocolType Protocol;
};

static auto&& GetContext(void* Data)
{
	return *static_cast<DownloadInfoContext*>(Data);
}

bool DownloadInfo::IsRange()
{
	auto&& Ctx = GetContext(Data);

	switch (Ctx.Protocol)
	{
	case ProtocolType::FILE:
		// FILE always returns a range if you requested one.
		return Ctx.Range != nullptr;

	case ProtocolType::HTTP:
	case ProtocolType::HTTPS:
		// The server should return HTTP 206 Partial Content if it's a range.
		return GetCurlResponseCode(Ctx.curl) == 206;

	default:
		Log.Error("DownloadInfo::IsRange -- Got protocol %d, but not sure what to do with it\n",
			Ctx.Protocol);
		return false;
	}
}

optional<double> DownloadInfo::ContentLength()
{
	double ContentLength;
	auto curl_ret = curl_easy_getinfo(Data, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &ContentLength);
	if (curl_ret != CURLE_OK)
	{
		Log.Error("curl_easy_getinfo returned error code %d, error message: %s, "
			"when trying to retrieve the download content length\n",
			int(curl_ret), curl_easy_strerror(curl_ret));
		assert(false);
		return nullopt;
	}

	if (ContentLength == -1)
		return nullopt;

	return ContentLength;
}

static CURL* CreateCurl()
{
	auto res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (res != CURLE_OK)
		return nullptr;
	return curl_easy_init();
}

static void DestroyCurl(CURL* curl)
{
	if (curl)
		curl_easy_cleanup(curl);
	curl_global_cleanup();
}

DownloadManagerType CreateDownloadManager()
{
	return DownloadManagerType{ CreateCurl() };
}

void DownloadManagerDeleter::operator()(void* curl) const
{
	DestroyCurl(curl);
}

struct CurlWriteData
{
	CURL* curl;
	function_view<DownloadCallbackType> Callback;
	DownloadInfoContext Context;
};

extern "C" size_t CurlWriteFunction(void* buffer, size_t size, size_t nmemb, void* stream)
{
	assert(stream != nullptr);

	auto& Data = *static_cast<CurlWriteData*>(stream);

	const auto total_size = size * nmemb;

	DownloadInfo Info{ &Data.Context };
	auto CallbackRet = Data.Callback(static_cast<const u8*>(buffer), total_size, Info);

	return CallbackRet ? total_size : 0;
}

extern "C" int CurlProgressFunction(void *clientp,
	curl_off_t dltotal, curl_off_t dlnow,
	curl_off_t ultotal, curl_off_t ulnow)
{
	auto&& ProgressFunction = *static_cast<function_view<ProgressCallbackType>*>(clientp);
	ProgressFunction(size_t(dltotal), size_t(dlnow));
	return 0;
}

static bool SetProtocol(ProtocolType& Protocol, const char* URL)
{
	auto URLView = StringView{ URL };
	auto ColonIndex = URLView.find_first_of(':');
	if (ColonIndex == URLView.npos)
		return false;

	auto ProtocolString = URLView.substr(0, ColonIndex);

	if (ProtocolString == "http")
		Protocol = ProtocolType::HTTP;
	else if (ProtocolString == "https")
		Protocol = ProtocolType::HTTPS;
	else if (ProtocolString == "file")
		Protocol = ProtocolType::FILE;
	else
		return false;

	return true;
}

void FormatError(DownloadError* ErrorOutput, const char* Format, ...)
{
	constexpr auto Size = DownloadError::Size;
	char Buf[Size];
	auto Dest = ErrorOutput ? ErrorOutput->String : Buf;
	va_list va;
	va_start(va, Format);
	vsprintf_safe(Dest, Size, Format, va);
	va_end(va);
	Log.Error("%s\n", Dest);
}

bool DownloadFile(const DownloadManagerType& DownloadManager,
	const char* URL,
	int Port,
	function_view<DownloadCallbackType> Callback,
	function_view<ProgressCallbackType> ProgressCallback,
	const char* Range,
	DownloadError* ErrorOutput)
{
	using namespace std::literals;

	static_assert(DownloadError::Size >= CURL_ERROR_SIZE, "DownloadError::Size too small");
	

	const auto curl = DownloadManager.get();

	CurlWriteData WriteData{ curl, Callback };
	WriteData.Context.curl = curl;
	WriteData.Context.Range = Range;
	if (!SetProtocol(WriteData.Context.Protocol, URL))
	{
		FormatError(ErrorOutput, "Unrecognized protocol in URL \"%s\"", URL);
		return false;
	}

	if (!curl)
	{
		FormatError(ErrorOutput, "Curl is dead! Can't download files");
		return false;
	}

	// Log and return false if curl_easy_setopt returns an error.
#define curl_easy_setopt_v(...)\
	do {\
		const auto res = curl_easy_setopt(__VA_ARGS__);\
		if (res != CURLE_OK) {\
			Log.Error("curl_easy_setopt(" #__VA_ARGS__ ") returned %d\n", res);\
			assert(false);\
			return false;\
		}\
	} while (false);

	curl_easy_setopt_v(curl, CURLOPT_URL, URL);
	curl_easy_setopt_v(curl, CURLOPT_PORT, Port);
	curl_easy_setopt_v(curl, CURLOPT_WRITEFUNCTION, CurlWriteFunction);
	curl_easy_setopt_v(curl, CURLOPT_WRITEDATA, &WriteData);
	curl_easy_setopt_v(curl, CURLOPT_RANGE, Range);

	// Make curl return CURLE_HTTP_RETURNED_ERROR on HTTP response >= 400.
	curl_easy_setopt_v(curl, CURLOPT_FAILONERROR, 1l);

	curl_easy_setopt_v(curl, CURLOPT_ERRORBUFFER, ErrorOutput ? ErrorOutput->String : nullptr);

	if (ProgressCallback)
	{
		curl_easy_setopt_v(curl, CURLOPT_XFERINFOFUNCTION, CurlProgressFunction);
		curl_easy_setopt_v(curl, CURLOPT_XFERINFODATA, &ProgressCallback);
		curl_easy_setopt_v(curl, CURLOPT_NOPROGRESS, 0l);
	}
	else
	{
		curl_easy_setopt_v(curl, CURLOPT_XFERINFOFUNCTION, nullptr);
		curl_easy_setopt_v(curl, CURLOPT_XFERINFODATA, nullptr);
		curl_easy_setopt_v(curl, CURLOPT_NOPROGRESS, 1l);
	}

	const auto res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		if (ErrorOutput)
		{
			Log.Error("Curl error: %s\n", ErrorOutput->String);
		}
		else
		{
			if (res == CURLE_HTTP_RETURNED_ERROR)
			{
				const int ResponseCode = GetCurlResponseCode(curl);
				FormatError(ErrorOutput, "Received HTTP error code %d when trying to "
					"download from URL %s",
					ResponseCode, URL);
			}
			else
			{
				FormatError(ErrorOutput, "Curl error! Code = %d, message = \"%s\"",
					res, curl_easy_strerror(res));
			}
		}

		return false;
	}

	return true;
}