# Basic instructions
The two most important parameters to correctly setup your VDBs in Unreal are **Local step size** and **Density** 
(aka density multiplier). Once you get good results these two parameters, use the other parameters to tweak your 
rendering.

Depending on your volume, **Local step size** usually have values from 0.005 to 10. Smaller raymarching steps values 
mean better quality but slower performances. For better performance keep this value as high as possible. 
For improved rendering quality decrease value.

Every Digital Content Creator tool (eg. Houdini, Embergen, Blender etc.) have different volume options and 
different export settings. **This is why importing different VDBs from different software sometimes require very
different plugin display options**.

Depending on your volume and according to my tests, **Density** usually have values ranging from 1 to 1000.

I’ve taken the arguable decision to only use **local** distances/sizes, i.e. **rendering will be independent of your volume scaling**.

If your are struggling with your VDB rendering, reset your volume scaling to (1, 1, 1) then check the local size of 
your volume. It could be 1000 units (i.e 10m) or maybe only 10 units (i.e 10cm). If you are targetting *performance 
vs quality*, you should probably aim for 50 to 200 raymarching steps. If you are aiming for 
*quality over performance*, you could have 100 to 1000 raymarching steps. It really depends on your needs and the VDB itself.

To get a rough idea on the number of steps, use this equation:  
`nb raymarch steps = local volume size / local step size` 

# FAQ

#### Everything seems to work fine but I don't see my VDB on screen. What am I doing wrong ?
Press F to focus on your volume in the viewport. Then increase ***Density*** values (10, 100, 1000 etc.) and decrease ***(Local) Step size*** (1, 0.1, 0.01 etc.) until you see something. Those are the two most important settings to render your VDBs, as explained [here](HELPME.md#basic-instructions). After fixing those two parameters, you can start tweaking your rendering with the other available options.

#### Can my VDBs receive shadows ? Can my VDBs cast shadows on the environment ?

Sadly no. Plugins are not allowed to read or write shadows. I know this is a big limitation and I’ve asked Epic 
many times to open the renderer more but it is a long term project on their side with no ETA

#### My blackbody explosions do not light the environment (realtime and pathtrace). Can you do something about it ?

Sadly for the same reasons above I do not have access to Lumen or other related lighting information from a plugin 
so blackbody VDBs do not influence the environment

#### Why is my point light (or any other lights) not affecting the VDBs ?

Only the first directional light and the skylight are supported for now.

#### My VDB is shallow or very pixelated

First make sure the **Local step size** and the **Density** parameters are correctly setup (see 
[Basic Instructions](#basic-instructions)). You can also increase samples count (but I don’t 
recommend it for performance reasons, let Unreal's TAA care of it).

#### My VDBs are really slow

Although NanoVDB finally brings some good GPU performance and some decent realtime results as demonstrated by 
this plugin, let’s not forget that anything volumetric is still extremely expensive and computationally intense. 
We obviously can’t expect the same results as offline renders. 

That being said, most people are not aware how performant current graphics cards are and that it is now a reality to 
have real volumetrics in a game engine. Reasonably large volumes render really well and fast with this plugin, 
depending on your VDB parameters. Very large volumes like the ones we usually see in the VFX industry are probably 
too big and expensive to render properly in real-time or interactive time.

#### My volumes are too noisy. What can I do ?

Having realtime performance comes with a few sacrifices and approximations. Rendering with noise gives better 
results but requires a good denoiser. Unreal temporal anti-aliasing works well but VDBs do not write motion 
vectors yet making TAA irrelevant with VDBs sequences or camera motion. There are options to turn off noise completely 
but it will introduce other artefacts.

#### Can you do the same in Unity ?

No. I'm working on this plugin on my spare time only and don't have time (and motivation) to do this in Unity.

#### I saw your cloud demo with JangaFX and/or your clouds in your example level. How can I add more clouds ? 

1. Open Blueprint and bump the maximum number of clouds to N (in the default values). Save and apply. 
2. Open the reference Material then copy paste the cloud box nodes N times and make sure to connect results.  
3. Add clouds in the Actor blueprint *Details panel*.

#### I have sorting issues with transparent materials and/or hair strands. Is there any way to fix these ?

I'm well aware of transparency issues with translucent objects (including hair strands) and I don't have a solution for it as I'm limited by plugin capabilities.
Some people like using Unreal's Volumetric Clouds system to render VDBs (check the example levels in the Release packages) and it solves some of these issues when using the right options (tip: check "Apply Cloud Fogging" on your translucent materials), but you will loose my core VDB rendering features. Epic should release their own version of VDB rendering soon that there's no other choice than to wait for their release.

### I exported my VDB sequence at 24fps but it looks like the plugin is forcing 30fps. Can i fix that ?

For now the plugin forces all imports at 30fps. To get back to 24fps you will need to slow down the sequence play rate by a factor of 24/30 = 0.8.
* You can change the realtime in-game sequence framerate using the "Playback speed" option in the VDB details panel. 
* If you're using the sequencer, right click on the sequence and change the "Play Rate" option.

#### Can I only render VDBs using the Movie Render Queue (MRQ) ?

The plugin comes with two post-processing materials *MovieRenderQueue_VdbColor* and *MovieRenderQueue_VdbAlpha* that can be used as custom render passes with the Movie Render Queue (MRQ). 

For quick display checks / debug I recommend using the *r.Vdb.DebugMode* console variable (default value is 0) to render only VDBs radiance/rgb (*r.Vdb.DebugMode 1*) or VDBs alpha (*r.Vdb.DebugMode 2*).


#### What is the difference between VdbMaterial and VdbPrincipled ?

Unless you want to use the pathtracer, you should always use VdbMaterial Actors. Both actors have feature parity 
but VdbMaterial use Unreal materials, allowing custom user controlled rendering, better flexibility and other features.

Only VdbPrincipled are pathtracer compatible for now.


# Pro tips

#### Displacement
You can introduce displacement on your volume to increase details and/or introduce some artificial motion on a static vdb. See example in Gallery scene.

#### Padding
When introducing displacement your volume might over-extend its boundaries. Use *volume padding* to augment the volume size.

#### Rendering options
 The rendering model is greatly inspired by [Blender](https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/volume_principled.html) and [Arnold](https://docs.arnoldrenderer.com/display/A5AFMUG/Standard+Volume). If you want more information on all rendering and volume parameters, check their documentations.

#### Using command line for global improved rendering
When using Movie Render Queue (MRQ), use lighter VDBs parameters for faster rendering in the viewport and use console variables to improve offline rendering quality during MRQ renders.
* r.Vdb.Trilinear false/true
* r.Vdb.CinematicQuality 0/1/2. 
    * if 0 (default), no changes 
    * if 1, all step sizes are multiplied by 4 and samples per pixel by 2, 
    * if 2, all step sizes are multiplied by 10, samples by 4 and trilinear sampling is forced on all volumes.


