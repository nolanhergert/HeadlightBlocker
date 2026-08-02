**It’s not going to be perfect, and that’s ok**

# Hook and preview:
It’s a universal experience, you’re driving along at nighttime and get blinded by headlights up ahead, sometimes at the worst possible moment. Or your eyes are exhausted and you can’t get any relief from the discomforting light sources itching at your retina. Existing products <show a sunvisor / sunglasses> help for a lot of situations, but still come up short in a number of ways.

*Even expensive sunglasses can't save you, as these MLB players are demonstrating.*

While I hope it'll eventually be helpful for a lot of people and I have done a bit of demand validation / customer discovery, that's a lot of expectations for a project that I want for myself anyways and enjoy learning about and tinkering with in my free time. So, for now let's put aside the business concerns and get hacking!

# Existing Solutions
Very briefly, the main use case is when you have a bright light source in your field of view and need to see details in darker areas as well.

## Sunglasses
You can put on sunglasses, but you'll run into a few problems:
* They darken the whole scene equally.
* Your eyes will adjust to the new relative levels and generally make the light painful again! You can play around with it yourself at a nearby sporting goods or department store with two different tints of sunglasses.
  * You have a few different options: No tint, some tint, and a dark tint. Add in your phone LED if you can't find bright nearby lights. 
  * I've found that for me, there is a tint level around 50% (the light tint levels) that my eyes adjust to for a bright source.


## Sun Visor / Cap
This is closer to an ideal solution. It can block the offending light source directly, but at the expense of visibility.

## Hand
They work fairly well! Except when you are using them for other things! Also it's annoying to keep them aligned when moving and it's tiring to hold them up. 

# The Ideal Solution
We need something that stays hidden until it's needed, and then quickly dims just the offending light source(s)

I initially played around with mounting an x/y gantry on a windshield. The math for motion and calibration is interesting. but I'm just not that good mechanically yet. In addition, it'll only work for one element, which isn't good enough.

What if we used LCD screens instead? They check a lot of boxes, simple mechanically, low power, etc. They suffer from an "off-state" transmissivity of ~40% (so aren't ideal at nighttime), but there are ways to change that too.

Additionally, where should we mount the LCDs? Although it's more complicated, it's cheaper to use them like sunglasses than to create a large version on a windshield.

# LCDs
## Contrast / Multiplexing
Active vs. passive matrix
ST7065C

## ITO trace refraction

## Pixel Shape

## Uniform Appearance on "shade" mode

## Spreadsheet


# Camera

## Required resolution / FOV

## Optimizing power draw with different modes

# Microcontroller


# UX

## Compactness / Layout

## Calibration / Configuration




## Other Attempts
It turns out that, surprise surprise, I'm not the first person to have this idea! Let's take a look at what other attempts have been made in this area.

## DynEye
The first documented commercial example I could find was by Chris Mullin and his company, DynEye. He was all the rage 20 years ago and got a working prototype with a roughly 20x15 pixel screen and a custom low power camera chip even! However, he struggled to turn it into a company. One interesting thing I learned from our several conversations is that he was able to get a meeting with Oakley to pitch them his prototype. They ended up declining, as they decided they would rather continue to sell $10 sunglasses for $300 than take the risk on an unproven and somewhat complicated product.

## Himax

Fun to talk with! Brief note on CES experience? Say hi to _________

## Bosch

## Chamelo?




Bosch did a thing...




Yes, the name for the GitHub sucks. Turns out it's kinda hard to come up with a good name! I'm open to ideas.


Current Design Philosophy
Simple and affordable product, complex design tradeoffs, like Zenni, IKEA, Harbor Freight


