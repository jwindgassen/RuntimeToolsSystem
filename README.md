# RuntimeToolsSystem

The **RuntimeToolsSystem** is a Plugin for Unreal, that allows you to use the **InteractiveToolsFramework (ITF)** at runtime.
The ITF is a Plugin in the Engine's Source Code, that is used to create the Tools in the Editor, i.e. Landscaping and 
Sculpting Tools.
This Plugin is designed to reuse the ITF at runtime, reimplementing the Architecture from the Editor to Spawn, Manage and Control the Tools.

The Plugin was originally developed by [gradientspace](https://www.gradientspace.com/), who also wrote a fantastic in-depth article, 
explaining how the ITF control-flow, lifecycle and classes work. Definitely check out
[the original article](https://www.gradientspace.com/tutorials/2021/01/19/the-interactive-tools-framework-in-ue426) and
[it's update for UE5](https://www.gradientspace.com/tutorials/2022/6/1/the-interactive-tools-framework-in-ue5)!

Hence, this repo started as a fork of [the original repo](https://github.com/gradientspace/UE5RuntimeToolsFrameworkDemo),
designed to make the system work with more general cases.
