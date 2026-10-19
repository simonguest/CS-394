using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class LlamaCppPlugin : ModuleRules
{
    public LlamaCppPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Projects"
            }
        );
        
        // Path to llama.cpp
        string LlamaCppPath = Path.Combine(ModuleDirectory, "../ThirdParty/llama.cpp");
        
        // Include directories
        PublicIncludePaths.Add(Path.Combine(LlamaCppPath, "include"));
        PublicIncludePaths.Add(Path.Combine(LlamaCppPath, "ggml-include"));
        
        // Link libraries
        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // Add ALL .a files in the directory
            string[] LibFiles = Directory.GetFiles(LlamaCppPath, "*.a");
            foreach (string LibFile in LibFiles)
            {
                PublicAdditionalLibraries.Add(LibFile);
                System.Console.WriteLine("Adding library: " + Path.GetFileName(LibFile));
            }
            
            // Add required Mac frameworks
            PublicFrameworks.Add("Metal");
            PublicFrameworks.Add("MetalKit");
            PublicFrameworks.Add("Foundation");
            PublicFrameworks.Add("Accelerate");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string[] LibFiles = Directory.GetFiles(LlamaCppPath, "*.a");
            foreach (string LibFile in LibFiles)
            {
                PublicAdditionalLibraries.Add(LibFile);
            }
        }
    }
}