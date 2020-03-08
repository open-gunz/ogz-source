#pragma once

namespace RealSpace2
{
class RVisualMesh;
}

class ZSkyBox 
{
public:
	ZSkyBox(std::unique_ptr<RealSpace2::RVisualMesh> VMesh);
	~ZSkyBox();

	void Render();

private:
	std::unique_ptr<RealSpace2::RVisualMesh> VMesh;
};
