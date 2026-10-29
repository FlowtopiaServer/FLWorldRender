#include "PlatformPrecomp.h"
#include "World/WorldScene.h"
#include "World/WorldRender.h"
#include "util/PNGWriter.h"

static void ShowUsage()
{
	LogMsg("FLWorldRender <world.rtwr> <out.png> [-res <client resources>] [-owner <name>] [-country <cc>] [-scale <0.5>] [-scene <x,y,w,h>]");
}

int main(int argc, char **argv)
{
	//LogMsg("FLWorldRender built %s %s", __DATE__, __TIME__);
	if (argc < 3)
	{
		ShowUsage();
		return 1;
	}
	WorldRenderOptions options;
	for (int i = 3; i + 1 < argc; i += 2)
	{
		string arg = argv[i];
		if (arg == "-res") options.resourcePath = argv[i + 1];
		else if (arg == "-owner") options.ownerName = argv[i + 1];
		else if (arg == "-country") options.ownerCountry = argv[i + 1];
		else if (arg == "-scale") options.scale = (float)atof(argv[i + 1]);
		else if (arg == "-scene")
		{
			if (sscanf(argv[i + 1], "%d,%d,%d,%d", &options.sceneX, &options.sceneY, &options.sceneW, &options.sceneH) != 4)
			{
				ShowUsage();
				return 1;
			}
		}
		else
		{
			ShowUsage();
			return 1;
		}
	}

	WorldScene world;
	if (!world.Load(argv[1])) return 1;
	//world.DumpToLog(); //too spammy for a 100x60 world

	WorldRender render;
	if (!render.Init(options))
	{
		LogError("Couldn't init the renderer, is %s right?", options.resourcePath.c_str());
		return 1;
	}

	SoftSurface out;
	if (!render.Render( world, out )) return 1;
	if (!SaveSurfaceAsPNG(out, argv[2]))
	{
		LogError("can t't write %s", argv[2]);
		return 1;
	}
	LogMsg("Rendered %s (%dx%d) to %s", world.m_name.c_str(), out.GetWidth(), out.GetHeight(), argv[2]);
	return 0;
}
