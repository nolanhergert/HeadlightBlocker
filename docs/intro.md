**It’s not going to be perfect, and that’s ok**

**You're not trying to become CEO of a startup. You're trying to see whether you can make a weird little piece of technology that you currently don't know how to make.**

It's at least a million dollar idea / can help >1000 people in theory. I think that's good enough for me to continue with it and make videos documenting it.

Who am I making this video for? 
* Anyone at least mildly interested in the topic, at least for initial one. ELI5 / 10 level? Spark curiosity


# Checklist
* Tripod
* Glasses stuff, including two sunglasses and ... two light sources?
* Manual focus and exposure on pixel 5, lock the exposure.

# Thumbnail
Show an example covering of the sun, but with lit trees around it to differentiate it from an eclipse photo? "Yes, It's Possible!"

# Hook and preview:
It’s a near-universal experience, you’re driving along at nighttime and get blinded by a bright light up ahead, sometimes at the worst possible moment. Or your eyes are exhausted and you can’t get any relief from the discomforting light sources itching at your retina. Existing products <show a sun visor / sunglasses> help for a lot of situations, but still come up short in a number of ways. <show MLB player dropping the ball>

Is there a solution that fixes this? If not, is the problem technical, economic, political? All of the above?

I've been learning and tinkering around in this space for a while now, and it's been a fascinating dive into many topics that are surprisingly accessible to a general audience. While I can't promise I'll cover everything, for this video we're going to focus In this series Let's take a look at some of the interesting bits together!

# Existing Solutions
Very briefly, the main use case is when you have a bright light source in your field of view and you need to see details in darker areas as well.

## Sunglasses
You can put on sunglasses, but you'll run into a few problems:
* They darken the whole scene equally. Reducing the brightness of the bright areas directly reduces the visibility of the dark areas.
* Your eyes will adjust to the new relative levels and generally make the light painful again! 
  * You can actually try this out yourself! Take two pairs of sunglasses ... draw a graph of your brain's perceptions over time with various scenarios.


## Sun Visor / Cap
This is closer to an ideal solution. It can block the offending light source directly, but at the expense of visibility of parts above that area of your scene. It's also limited ...

## Hand
They work fairly well! Except when you are using them for other things! Also it's annoying to keep them aligned when moving and it's tiring to hold them up. I've heard of people using pieces of tape on their glasses or visors, which seems to suffer from a similar fate.

## Others
Piece of tape / attachment to glasses

There are other options like photochromic lenses that dim when UV light hits them, but they suffer from the same fundamental problems.

# The Ideal Solution
So let's take a break and think about what the ideal solution would look like.

We need something that stays hidden until it's needed, and then quickly dims just the offending light source(s) and moves as it moves around.

## Glare Experiment
Let's do an experiment and see what we can observe! <Maybe borrow Dad's camera so you can adjust aperture size?>
* Use a pen/pencil with a bright light source (phone LED or bright white phone screen in an adjustably- dimmable dark room for glare effects)
* Test with clear plastic with sharpie drawn, then a scratched portion too <simulating particular lcd mode not being as useful, but not sure that I'll touch on it later>

The closer it is to your face, the more large and blurry the blocker gets. But the required absolute size of the blocker stays the same.

A lot around the object is surprisingly visible still, even when it is large and blurry.

<strikethrough>The required size is the convolution of the size of the object and the size of your pupil.</strikethrough>

Note that the glare *around* the bright source doesn't go away if you have something closely mounted. Far away is better.

There is a tradeoff between the over-drawing and its sensitivity to eye motion. To do better would involve eye tracking, which greatly increases the complexity and to some extent, the power budget, of a head-worn device. 

<draw on whiteboard?>

But, all in all, it seems possible in theory to have a device that will do what we want!

## The Medium
I initially played around with mounting an cable driven ______ on a windshield. The math for motion and calibration is interesting. but I'm just not that good mechanically yet. In addition, it'll only work for one element, which isn't good enough, and doesn't change in size without a seemingly-complicated mechanism.

What if we used LCD screens? They check a lot of boxes, simple mechanically, low power, etc. They suffer from an "off-state" transmissivity of ~40% (so aren't ideal at nighttime), but there are ways to fix that too.

# LCDs
So...how far away should the LCD be? The "best" version is something decently far away from your eyes, as we found above. But, let's say you're driving in a car, now you need something large enough to cover the entire windshield! That gets expensive and unwieldy fast. *However, a number of people I talked to preferred to have an "appliance" like this.*

Although it has other constraints, it's much cheaper to have small lenses like sunglasses, and it's a more interesting problem for me personally for some reason. Feel free to choose differently!



I was able to source some various passive monochrome LCDs. Let's take a look at how they perform!

## LCD Experiment
<Draw a larger circle on the display?>

<Move the matrix LCD close and farther away from the video to demonstrate blurring effects?>

## Contrast / Multiplexing
Active vs. passive matrix

Multiplexing vs. single pixel contrast

## ITO trace refraction

## Pixel Size vs. ITO Gap Size
(more resolution = less contrast)

But farther away, the more annoying the gaps can become too with a bright light source!

## Pixel Shape



# Camera

How will the image pipeline work? It's not as complicated as you might think!

## Adjustable Camera Experiment?

## Thresholding

Cameras also have a set exposure that they use to image the world with. In our case though, we want to only see the light sources that are significantly brighter than the other objects in the scene. By significantly lowering the exposure, this automatically thresholds the image and saves on power!

## Mapping
Drawing the opaque region is similarly straightforward. Just invert the thresholded image!

Well, there are a *few* more details but we can skip them for now.

## Required resolution / FOV
The power drawn by the camera is proportional to the camera's resolution

Do we actually need a high resolution camera if our output resolution is limited? You would think not, but there are some edge cases...
* Detecting how bright the source is is ambiguous at low resolutions when the source is small
* Sometimes the pixel array has a large region in each pixel dedicated to sampling circuit...PAC9001LT. <probably need to run the video grab by them, just show them the final cut, with the GUI blurred out?>

## Optimizing power draw with different modes/sensors
### Detection
### Localization
### Tracking

# Wrap Up
So, a device certainly seems plausible! But, that leaves the open question, "why hasn't anyone *else* done it yet?" Well, as it turns out, a number of companies have tried, and they even had prototypes! <show them in the background> So, why did they stop pursuing it? Oh my, look at the time, I guess we'll have to leave that topic for another video! Feel free to read ahead on the Github page in the description, and leave any comments and questions you have below! I read them all! Bye! :)

# Microcontroller


# UX

## Compactness / Layout

## Calibration / Configuration
While some might move to BLE and it's pretty easy to do with a CH572, I don't think I need it here and it adds size/parts, certification complexity, etc. I'm probably wrong here Good thing it's my project! I have a camera and probably a touch sensor already. 

Touch on surface plus 


### UNKNOWN


 I wish it was that simple...
  * Expand the detected bright regions by roughly the size of your pupils. Since they are out of focus. Requires guessing the current dilation of your eyes...
  * Redraw the points on each LCD based based on an initial calibration procedure
Potential complications:
  * The user's eyes veer too far away from neutral (would require eye tracking to fix, much more expensive)
  * The glasses slip down due to sweat, need to be readjusted



You can play around with it yourself at a nearby sporting goods or department store with two different tints of sunglasses.
  * You have a few different options: No tint, some tint, and a dark tint. Add in your phone LED if you can't find bright nearby lights. 
  * I've found that for me, there is a tint level around 50% (the light tint levels) that my eyes adjust to for a bright source.



There are other modes of liquid crystals, one in particular scatters the light. We can simulate that using scratches and I found that I didn't like it as much. Was too bright.

Mention ST7065C?

## Uniform Appearance on "shade" mode

## Spreadsheet Trace Length

## Other Attempts
It turns out that, surprise surprise, I'm not the first person to have this idea! Let's take a look at what other attempts have been made in this area.

## DynEye
The first documented commercial example I could find was by Chris Mullin and his company, DynEye. He was all the rage 20 years ago and got a working prototype with a roughly 20x15 pixel screen and a custom low power camera chip even! However, he struggled to turn it into a company. One interesting thing I learned from our several conversations is that he was able to get a meeting with a large sports sunglasses brand to pitch them his prototype. They ended up declining, as they decided they would rather continue to sell $10 sunglasses for $300 than take the risk on an unproven and somewhat complicated product.

## Himax

Fun to talk with! Brief note on CES experience? Say hi to _________

## Bosch

## Chamelo?




Bosch did a thing...




Yes, the name for the GitHub sucks. Turns out it's kinda hard to come up with a good name! I'm open to ideas.


Current Design Philosophy
Simple and affordable product, complex design tradeoffs, like Zenni, IKEA, Harbor Freight


