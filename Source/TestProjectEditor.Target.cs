using UnrealBuildTool;
using System.Collections.Generic;

public class TestProjectEditorTarget : TargetRules
{
	public TestProjectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TestProject");
	}
}
