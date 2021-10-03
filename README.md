# isolation-game
An open source FPS written in C++ for Unreal Engine

If you are attempting to compile the code on a Mac, make sure to add "PysicsCore" to your public dependancy modules in the YourProjectName.Build.cs file. Overriding PhysicalMaterials, as done in SWeaponBase for example, requires this to compile.