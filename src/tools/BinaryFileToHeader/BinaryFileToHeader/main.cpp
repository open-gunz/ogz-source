#include <string>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <fstream>
#include <cstdarg>

struct settings_t
{
	const char* source_file = "";
	const char* output_file = "";
	const char* object_name = "";
};

static bool process_commandline_option(settings_t& settings, const char* arg)
{
	if (arg[0] != '-')
	{
		settings.source_file = arg;
		return true;
	}

	auto get_option_argument = [&](const std::string& a) -> const char*
	{
		for (size_t i{}, end = a.length(); i < end; ++i)
			if (a[i] != arg[i])
				return nullptr;

		return arg + a.length();
	};

	std::unordered_map<std::string, const char*&> map{
		{ "-o", settings.output_file },
		{ "-name", settings.object_name },
	};


	for (auto& pair : map)
	{
		if (const char* p = get_option_argument(pair.first))
		{
			pair.second = p;
			return true;
		}
	}

	return false;
}

static std::string read_file(const char* filename)
{
	std::ifstream file{ filename, std::ios::binary | std::ios::ate };
	if (file.fail())
		return "";
	auto size = file.tellg();
	std::string contents;
	contents.resize(static_cast<size_t>(size));
	file.seekg(std::ios::beg);

	file.read(&contents[0], size);
	if (file.fail())
		return "";
	return contents;
}

static bool generate_header(const char* src, const char* dest, const char* name)
{
	auto src_file_contents = read_file(src);
	if (src_file_contents.empty())
		return false;

	if (src_file_contents.size() % sizeof(uint32_t) != 0)
		src_file_contents.append(src_file_contents.size() % sizeof(uint32_t), 0);

	std::string dest_file_contents;
	dest_file_contents.reserve(src_file_contents.size() * 5 + 512);

	auto format = [&](const char* format, ...)
	{
		char buf[512];
		va_list va;
		va_start(va, format);
		vsprintf_s(buf, format, va);
		va_end(va);
		dest_file_contents += buf;
	};

	format("#pragma once\n\nconst unsigned char %s[] = {", name);
	for (auto c : src_file_contents)
		format("0x%02X, ", static_cast<unsigned char>(c));
	format("};\n");

	std::ofstream dest_file{ dest };
	if (dest_file.fail())
		return false;

	dest_file.write(dest_file_contents.data(), dest_file_contents.size());
	return !dest_file.fail();
}

int main(int argc, char** argv)
{
	settings_t settings;
	const char* source_file = "";
	for (int i{}; i < argc; ++i)
		if (!process_commandline_option(settings, argv[i]))
			printf_s("Unrecognized command-line option %s\n", argv[i]);

	std::unordered_map<const char*, const char*> map{
		{ settings.source_file, "source file" },
		{ settings.output_file, "output file" },
		{ settings.object_name, "object name" },
	};

	for (auto& pair : map)
	{
		if (!strlen(pair.first))
		{
			printf_s("No %s, exiting\n", pair.second);
			return -1;
		}
	}

	auto generated_header = generate_header(settings.source_file, settings.output_file, settings.object_name);
	if (!generated_header)
	{
		printf_s("Failed to generate header\n");
		return -1;
	}

	return 0;
}