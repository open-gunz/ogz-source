#pragma once
#include "ZRule.h"
#include <unordered_map>
#include <vector>

struct Course
{
	std::string Name;
	rboundingbox Start;
	rboundingbox End;
};

class CourseManager
{
public:
	void Init();
	void Destroy() { pVB.reset(); }

	void SetCurrentMap(const char *szMap);

	bool IsInStartZone(const rvector &pos, int &CourseIndex) const;
	bool IsInEndZone(const rvector &pos, int CourseIndex) const;
	const std::string &GetCourseName(int CourseIndex) const;
	int GetNumCourses() const;
	rvector GetCourseStartPos(int CourseIndex) const;

	void Draw();

private:
	struct Line
	{
		rvector a, b;
	};

	std::unordered_map<std::string, std::vector<Course>> Courses;
	std::vector<Course> *pCurrentCourseSet;
	D3DPtr<IDirect3DVertexBuffer9> pVB;
	int PrimitiveCount = 0;
};

class ZRuleSkillmap : public ZRule
{
public:
	ZRuleSkillmap(ZMatch* pMatch);

	void OnFall();
	void Draw();
	virtual void OnUpdate(float fDelta) override;

	static CourseManager CourseMgr;

private:
	int CurrentCourseIndex = -1;
	float fStartTime = 0;
};