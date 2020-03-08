#pragma once

#include "MZFileSystem.h"

class MZFile
{
public:
	struct MaybeArrayDeleter {
		bool ShouldDelete{};
		MaybeArrayDeleter(bool ShouldDelete) : ShouldDelete{ ShouldDelete } {}

		void operator()(char* ptr) const {
			if (ptr && ShouldDelete)
				delete[] ptr;
		}
	};
	using DataPtr = std::unique_ptr<char[], MaybeArrayDeleter>;

	MZFile();
	MZFile(const MZFile&) = delete;
	MZFile& operator=(const MZFile&) = delete;
	MZFile(MZFile&&) = default;
	MZFile& operator=(MZFile&&) = default;
	~MZFile();

	bool Open(const char* szFileName, MZFileSystem* pZFS = nullptr);
	void Close();

	enum SeekPos { begin = 0x0, current = 0x1, end = 0x2 };
	bool Seek(i64 offset, SeekPos mode = current);
	i64 Tell() const;

	auto GetLength() const { return FileSize; }
	bool Read(void* pBuffer, int nMaxSize);
	template <typename T>
	bool Read(T& dest) {
		return Read(&dest, sizeof(dest));
	}

	DataPtr Release();

	static void SetReadMode(u32 mode) { ReadMode = mode; }
	static u32 GetReadMode() { return ReadMode; }
	static bool isMode(u32 mode) { return (ReadMode & mode) != 0; }

protected:
	bool IsArchive() const { return Desc != nullptr; }
	bool OpenArchive(const MZFileDesc& Desc, MZFileSystem& FS);
	bool LoadFile();
	void SetData(char* ptr, bool ShouldDelete) {
		Data = DataPtr{ ptr, MaybeArrayDeleter{ShouldDelete} }; }
	void SetData(nullptr_t) {
		SetData(nullptr, false); }

	CFilePtr fp;
	DataPtr Data{ (char*)nullptr, MaybeArrayDeleter{false} };
	const MZFileDesc* Desc{};

	i64 Pos{};
	size_t FileSize{};

	static u32 ReadMode;
};