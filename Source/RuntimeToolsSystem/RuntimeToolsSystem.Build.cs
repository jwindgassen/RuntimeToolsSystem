using UnrealBuildTool;

public class RuntimeToolsSystem : ModuleRules {
	public RuntimeToolsSystem(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		Type = ModuleType.CPlusPlus;
		IWYUSupport = IWYUSupport.Full;

		PublicDependencyModuleNames.AddRange(new[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InteractiveToolsFramework",
			"MeshModelingTools",
			"MeshModelingToolsExp"
		});

		PrivateDependencyModuleNames.AddRange(new[] {
			"ModelingComponents",
			"SlateCore",
			"Slate",
			"InputCore",
			"RenderCore",
			"GeometryCore",
			"GeometryFramework",
			"MeshDescription",
			"StaticMeshDescription",
			"MeshConversion"
		});
	}
}