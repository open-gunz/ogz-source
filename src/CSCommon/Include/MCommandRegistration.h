#pragma once

#include "MSharedCommandTable.h"
#include "MCommandManager.h"

namespace detail {
inline void AddConditions(MCommandParameterDesc* Param) {}

template <typename CurType, typename... RestType>
void AddConditions(MCommandParameterDesc* ParamDesc, CurType&& CurCondition, RestType&&... Rest) {
	ParamDesc->AddCondition(new CurType{ std::forward<CurType>(CurCondition) });
	AddConditions(ParamDesc, std::forward<RestType>(Rest)...);
}
}

// Introduces some helper functions into the scope.
//
// C(ID, Name, Description, Flags) -- Adds a new command description.
//
// P(Type, Description, Conditions...) -- Adds a new parameter description to the command
// description most recently added with C.
// Optionally, you can add some parameter conditions after the description.
//
// CA(Name, Text) -- Adds a command alias.
// 
// IsTypeAnyOf(SharedTypes...) -- Checks if the shared type is any of the arguments.

#define BEGIN_CMD_DESC(CommandManager, SharedType)\
	MCommandDesc* pCD4m = nullptr;\
	\
	auto C = [&](int ID, const char* Name, const char* Description, int Flags) {\
		pCD4m = new MCommandDesc{ ID, Name, Description, Flags };\
		CommandManager->AddCommandDesc(pCD4m);\
	};\
	\
	auto P = [&](MCommandParameterType Type, const char* Description, auto&&... Conditions) {\
		auto* ParamDesc = new MCommandParameterDesc{ Type, Description };\
		detail::AddConditions(ParamDesc, std::forward<decltype(Conditions)>(Conditions)...);\
		pCD4m->AddParamDesc(ParamDesc);\
	};\
	\
	auto CA = [&](const char* Name, const char* Text) {\
		CommandManager->AddAlias(Name, Text);\
	};\
	\
	auto IsTypeAnyOf = [&](auto&&... Vals) {\
		u32 Sum = 0;\
		for (auto&& Val : { Vals... })\
			Sum |= Val;\
		return (SharedType & Sum) != 0;\
	};\
	\
	using namespace MSharedCommandType
